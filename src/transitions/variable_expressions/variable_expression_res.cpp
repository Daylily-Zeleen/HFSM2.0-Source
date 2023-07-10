#include "variable_expression_res.h"

#include "../../hfsm.h"
#include "../variable_transition.h"

namespace Hfsm {

// VariableExpressionRes
Ref<HFSMVariableRes> VariableExpressionRes::get_variable_res() const { return variable_res; }

Variant VariableExpressionRes::get_value() const { return value; }

uint8_t VariableExpressionRes::get_comparator() const { return comparator; }

uint8_t VariableExpressionRes::get_trigger_type() const { return trigger_type; }

bool VariableExpressionRes::is_variable_as_value() const { return variable_as_value; }

VariableExpression *VariableExpressionRes::create_variable_expression(HFSM *p_hfsm) {
	ERR_FAIL_COND_V(!variable_res.is_valid(), nullptr);
	// 获取变量类型
	auto v = p_hfsm->get_var(variable_res->get_variable_name());
	ERR_FAIL_COND_V(!v.is_valid(), nullptr);
	//  触发器
	if (v->get_type() == Variant::NIL) {
		switch (trigger_type) {
			case TRIGGER_TYPE_NORMAL:
				return memnew(TriggerExpression(v));
			case TRIGGER_TYPE_UNION:
				return memnew(UnionTriggerExpression(v));
			case TRIGGER_TYPE_SOLO:
				return memnew(SoloTriggerExpression(v));
			default:
				break;
		}
	} else {
		if (variable_as_value) {
			return memnew(VariableComparationExpression(variable_res, comparator, value));
		} else {
			return memnew(ConstantComparationExpression(variable_res, comparator, value));
		}
	}
	return nullptr;
}

bool VariableExpressionRes::_set(const StringName &p_name, const Variant &p_property) {
	_TRY_SET_PROP(comparator);
	_TRY_SET_PROP(variable_as_value);
	_TRY_SET_PROP(value);
	_TRY_SET_PROP(trigger_type);
	return false;
}

bool VariableExpressionRes::_get(const StringName &p_name, Variant &r_property) const {
	_TRY_GET_PROP(comparator);
	_TRY_GET_PROPB(variable_as_value);
	_TRY_GET_PROP(value);
	_TRY_GET_PROP(trigger_type);
	return false;
}

void VariableExpressionRes::_get_property_list(List<PropertyInfo> *p_list) const {
	if (variable_res.is_valid()) {
		auto comparator_hint = "==,!=,>,>=,<,<=";
		switch (Variant::Type(variable_res->get_type())) {
			case Variant::BOOL:
			case Variant::STRING:
				comparator_hint = "==,!=";
				break;
			case Variant::INT:
			case Variant::FLOAT:
				comparator_hint = "==,!=,>,>=,<,<=";
				break;
			default:
				comparator_hint = "";
				break;
		}

		if (variable_res->get_type() != Variant::NIL) {
			_PUSH_PROP(INT, comparator, PROPERTY_HINT_ENUM, comparator_hint);
			_PUSH_PROP(BOOL, variable_as_value);
		}

		if (is_variable_as_value()) {
			_PUSH_PROP(OBJECT, value, PROPERTY_HINT_RESOURCE_TYPE, HFSMVariableRes::get_class_static());
		} else {
			if (variable_res->get_type() != Variant::NIL) {
				p_list->push_back(PropertyInfo(variable_res->get_type(), TNAMEOF(value)));
			} else {
				_PUSH_PROP(INT, trigger_type, PROPERTY_HINT_ENUM, "Solo,Union,Normal");
			}
		}
	}
}

void VariableExpressionRes::_bind_methods() {
	GDBIND_BEGIN(VariableExpressionRes);

	// 变量资源
	GDADD_PROPERTY_RESOURCE(variable_res);

	// 操作符
	GDBIND_SETGET(comparator);
	GDBIND_SETGET_BOOL(variable_as_value);
	GDBIND_SETGET(value);
	GDBIND_SETGET(trigger_type);
	// 枚举
	BIND_CONSTANT(TRANSITION_TYPE_SCRIPT);
	BIND_CONSTANT(TRANSITION_TYPE_VARIABLE);
	BIND_CONSTANT(TRANSITION_TYPE_EXPRESSION);
	BIND_CONSTANT(TRANSITION_TYPE_AUTO);

	BIND_CONSTANT(OP_EQUAL);
	BIND_CONSTANT(OP_NOT_EQUAL);
	BIND_CONSTANT(OP_GREATER);
	BIND_CONSTANT(OP_GREATER_EQUAL);
	BIND_CONSTANT(OP_LESS);
	BIND_CONSTANT(OP_LESS_EQUAL);
}

void VariableExpressionRes::set_variable_as_value(bool p_variable_as_value) {
	if (variable_as_value == p_variable_as_value) {
		return;
	}
	variable_as_value = p_variable_as_value;
	if (variable_as_value) {
		if (cast_to<HFSMVariableRes>(value)) {
			value = Variant();
		}
	} else {
		if (!Variant::can_convert(value.get_type(), variable_res->get_type())) {
			value = variable_res->get_default_value();
		}
	}
	emit_changed();
	notify_property_list_changed();
}

void VariableExpressionRes::set_value(const Variant &p_value) {
	if (!variable_res.is_valid()) {
		value = Variant();
		emit_changed();
	} else {
		if (variable_as_value && p_value.get_type() == Variant::OBJECT) {
			// 变量作为比较值
			if (auto v = cast_to<HFSMVariableRes>(p_value)) {
				if (value != p_value &&
						Variant::can_convert_strict(
								Variant::Type(variable_res->get_type()),
								Variant::Type(v->get_type()))) {
					value = p_value;
					emit_changed();
				} else {
					value = Variant();
					emit_changed();
				}
				return;
			}
		} else {
			// 常量作为比较值
			if (Variant::can_convert_strict(
						p_value.get_type(),
						Variant::Type(variable_res->get_type()))) {
				if (value != p_value) {
					value = p_value;
					emit_changed();
				}
				return;
			}
		}
		// 异常情况用默认值
		if (value != variable_res->get_default_value()) {
			value = variable_res->get_default_value();
			emit_changed();
		}
	}
}

void VariableExpressionRes::set_comparator(int64_t p_op) {
	if (variable_res.is_valid()) {
		if (variable_res->get_type() == Variant::STRING) {
			if (p_op == OP_EQUAL && p_op == OP_NOT_EQUAL) {
				if (p_op != comparator) {
					comparator = p_op;
					emit_changed();
					return;
				}
			}
		} else if (variable_res->get_type() == Variant::INT ||
				variable_res->get_type() == Variant::FLOAT) {
			if (p_op >= OP_EQUAL && p_op < 6) {
				if (p_op != comparator) {
					comparator = p_op;
					emit_changed();
					return;
				}
			}
		}
	}
	if (comparator != OP_EQUAL) {
		comparator = OP_EQUAL;
		emit_changed();
	}
}

void VariableExpressionRes::set_variable_res(const Ref<HFSMVariableRes> &p_variable_res) {
	if (variable_res != p_variable_res) {
		auto ptr = cast_to<Object>(this);
		if (variable_res.is_valid() && variable_res->IS_CONNECTED(s_changed, ptr, notify_property_list_changed)) {
			variable_res->DISCONNECT(s_changed, ptr, notify_property_list_changed);
		}
		variable_res = p_variable_res;
		if (variable_res.is_valid() && !variable_res->IS_CONNECTED(s_changed, ptr, notify_property_list_changed)) {
			variable_res->connect(s_changed, CALLABLE(ptr, notify_property_list_changed));
		}
		set_value(value);
		set_comparator(comparator);
		emit_changed();
		notify_property_list_changed();
	}
}

void VariableExpressionRes::set_trigger_type(int64_t p_trigger_type) {
	ERR_FAIL_COND(!(p_trigger_type >= 0 && p_trigger_type < TRIGGER_TYPE_MAX));
	trigger_type = uint8_t(p_trigger_type);
	emit_changed();
}

} // namespace Hfsm
