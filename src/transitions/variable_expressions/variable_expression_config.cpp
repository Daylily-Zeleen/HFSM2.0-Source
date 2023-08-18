/**************************************************************************/
/*  variable_expression_config.cpp                                        */
/**************************************************************************/
/*                         This file is part of:                          */
/*                   Hierarchical Finite State Machine                    */
/*            https://github.com/Daylily-Zeleen/HFSM2.0-Source            */
/**************************************************************************/
/* Copyright (c) 2023-present Daylily Zeleen.                             */
/*                                                                        */
/* Permission is hereby granted, free of charge, to any person obtaining  */
/* a copy of this software and associated documentation files (the        */
/* "Software"), to deal in the Software without restriction, including    */
/* without limitation the rights to use, copy, modify, merge, publish,    */
/* distribute, sublicense, and/or sell copies of the Software, and to     */
/* permit persons to whom the Software is furnished to do so, subject to  */
/* the following conditions:                                              */
/*                                                                        */
/* The above copyright notice and this permission notice shall be         */
/* included in all copies or substantial portions of the Software.        */
/*                                                                        */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. */
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY   */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,   */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE      */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                 */
/**************************************************************************/

#include "variable_expression_config.h"

#include "../../hfsm.h"
#include "../variable_expressions_transition.h"
#include <type_traits>

namespace Hfsm {

// VariableExpressionConfig
Ref<VariableConfig> VariableExpressionConfig::get_variable_config() const { return variable_config; }

Variant VariableExpressionConfig::get_value() const { return value; }

VariableExpressionConfig::Comparator VariableExpressionConfig::get_comparator() const { return comparator; }

VariableExpressionConfig::TriggerType VariableExpressionConfig::get_trigger_type() const { return trigger_type; }

bool VariableExpressionConfig::is_variable_as_value() const { return variable_as_value; }

VariableExpression *VariableExpressionConfig::create_variable_expression(HFSM *p_hfsm) {
	ERR_FAIL_COND_V_MSG(!variable_config.is_valid(), nullptr, "Create VariableExpression failed, \"variable_config\" is null.");
	// 获取变量类型
	auto v = p_hfsm->get_var(variable_config->get_variable_name());
	ERR_FAIL_COND_V_MSG(!v.is_valid(), nullptr, "Create VariableExpression failed, has not variable: " + variable_config->get_variable_name());
	//  触发器
	if (v->get_variable_type() == Variant::NIL) {
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
			return memnew(VariableComparationExpression(variable_config, comparator, value));
		} else {
			return memnew(ConstantComparationExpression(variable_config, comparator, value));
		}
	}
	ERR_FAIL_COND_V_MSG(!v.is_valid(), nullptr, "Create VariableExpression failed.");
}

bool VariableExpressionConfig::_set(const StringName &p_name, const Variant &p_property) {
	_TRY_SET_PROP(comparator);
	_TRY_SET_PROP(trigger_type);
	_TRY_SET_PROP(variable_as_value);
	_TRY_SET_PROP(value);
	return false;
}

bool VariableExpressionConfig::_get(const StringName &p_name, Variant &r_property) const {
	_TRY_GET_PROP(comparator);
	_TRY_GET_PROPB(variable_as_value);
	_TRY_GET_PROP(value);
	_TRY_GET_PROP(trigger_type);
	return false;
}

void VariableExpressionConfig::_get_property_list(List<PropertyInfo> *p_list) const {
	if (variable_config.is_valid()) {
		auto comparator_hint = "==,!=,>,>=,<,<=";
		switch (Variant::Type(variable_config->get_type())) {
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

		if (variable_config->get_type() != Variant::NIL) {
			_PUSH_PROP(INT, comparator, PROPERTY_HINT_ENUM, comparator_hint);
			_PUSH_PROP(BOOL, variable_as_value);
		}

		if (is_variable_as_value()) {
			_PUSH_PROP(OBJECT, value, PROPERTY_HINT_RESOURCE_TYPE, VariableConfig::get_class_static());
		} else {
			if (variable_config->get_type() != Variant::NIL) {
				p_list->push_back(PropertyInfo(variable_config->get_type(), TNAMEOF(value)));
			} else {
				_PUSH_PROP(INT, trigger_type, PROPERTY_HINT_ENUM, "Solo,Union,Normal");
			}
		}
	}
}

void VariableExpressionConfig::_bind_methods() {
	GDBIND_BEGIN(VariableExpressionConfig);

	// 变量资源
	GDADD_PROPERTY_RESOURCE(variable_config);

	// 操作符
	GDBIND_SETGET(comparator);
	GDBIND_SETGET_BOOL(variable_as_value);
	GDBIND_SETGET(value);
	GDBIND_SETGET(trigger_type);

	BIND_ENUM_CONSTANT(TRIGGER_TYPE_SOLO);
	BIND_ENUM_CONSTANT(TRIGGER_TYPE_UNION);
	BIND_ENUM_CONSTANT(TRIGGER_TYPE_NORMAL);
	BIND_ENUM_CONSTANT(TRIGGER_TYPE_MAX);

	// HACK

#ifdef GDEXTENSION_BUILD
#define BIND_COMPARATOR_ENUM_CONSTANT(m_constant) \
	godot::ClassDB::bind_integer_constant(get_class_static(), godot::_gde_constant_get_enum_name(Comparator::m_constant, #m_constant), #m_constant, Comparator::m_constant);
#else // GDEXTENSION_BUILD
#define BIND_COMPARATOR_ENUM_CONSTANT(m_constant) \
	::ClassDB::bind_integer_constant(get_class_static(), __constant_get_enum_name(Comparator::m_constant, #m_constant), #m_constant, Comparator::m_constant);
#endif // GDEXTENSION_BUILD

	// 枚举
	BIND_COMPARATOR_ENUM_CONSTANT(COMPARATOR_EQUAL);
	BIND_COMPARATOR_ENUM_CONSTANT(COMPARATOR_NOT_EQUAL);
	BIND_COMPARATOR_ENUM_CONSTANT(COMPARATOR_GREATER);
	BIND_COMPARATOR_ENUM_CONSTANT(COMPARATOR_GREATER_EQUAL);
	BIND_COMPARATOR_ENUM_CONSTANT(COMPARATOR_LESS);
	BIND_COMPARATOR_ENUM_CONSTANT(COMPARATOR_LESS_EQUAL);
}

void VariableExpressionConfig::set_variable_as_value(bool p_variable_as_value) {
	if (variable_as_value == p_variable_as_value) {
		return;
	}
	variable_as_value = p_variable_as_value;
	if (variable_as_value) {
		if (cast_to<VariableConfig>(value)) {
			value = Variant();
		}
	} else {
		if (!Variant::can_convert(value.get_type(), variable_config->get_type())) {
			value = variable_config->get_default_value();
		}
	}
	emit_changed();
	notify_property_list_changed();
}

void VariableExpressionConfig::set_value(const Variant &p_value) {
	if (!variable_config.is_valid()) {
		value = Variant();
		emit_changed();
	} else {
		if (variable_as_value && p_value.get_type() == Variant::OBJECT) {
			// 变量作为比较值
			if (auto v = cast_to<VariableConfig>(p_value)) {
				if (value != p_value &&
						Variant::can_convert_strict(
								Variant::Type(variable_config->get_type()),
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
						Variant::Type(variable_config->get_type()))) {
				if (value != p_value) {
					value = p_value;
					emit_changed();
				}
				return;
			}
		}
		// 异常情况用默认值
		if (value != variable_config->get_default_value()) {
			value = variable_config->get_default_value();
			emit_changed();
		}
	}
}

void VariableExpressionConfig::set_comparator(Comparator p_op) {
	if (variable_config.is_valid()) {
		if (variable_config->get_type() == Variant::STRING) {
			if (p_op == Comparator::COMPARATOR_EQUAL && p_op == Comparator::COMPARATOR_NOT_EQUAL) {
				if (p_op != comparator) {
					comparator = p_op;
					emit_changed();
					return;
				}
			}
		} else if (variable_config->get_type() == Variant::INT ||
				variable_config->get_type() == Variant::FLOAT) {
			if (p_op >= Comparator::COMPARATOR_EQUAL && p_op < 6) {
				if (p_op != comparator) {
					comparator = p_op;
					emit_changed();
					return;
				}
			}
		}
	}
	if (comparator != Comparator::COMPARATOR_EQUAL) {
		comparator = Comparator::COMPARATOR_EQUAL;
		emit_changed();
	}
}

void VariableExpressionConfig::set_variable_config(const Ref<VariableConfig> &p_variable_config) {
	if (variable_config != p_variable_config) {
		auto ptr = cast_to<Object>(this);
		if (variable_config.is_valid() && variable_config->IS_CONNECTED(s_changed, ptr, notify_property_list_changed)) {
			variable_config->DISCONNECT(s_changed, ptr, notify_property_list_changed);
		}
		variable_config = p_variable_config;
		if (variable_config.is_valid() && !variable_config->IS_CONNECTED(s_changed, ptr, notify_property_list_changed)) {
			variable_config->connect(s_changed, CALLABLE(ptr, notify_property_list_changed));
		}
		set_value(value);
		set_comparator(comparator);
		emit_changed();
		notify_property_list_changed();
	}
}

void VariableExpressionConfig::set_trigger_type(TriggerType p_trigger_type) {
	// ERR_FAIL_COND(!(p_trigger_type >= 0 && p_trigger_type < TRIGGER_TYPE_MAX));
	trigger_type = p_trigger_type;
	emit_changed();
}

} // namespace Hfsm
