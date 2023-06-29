#include "hfsm_variable_res.hpp"
#include "fsm_res.hpp"
#include "hfsm_variable.hpp"

#include "fsm_res.hpp"
#include <godot_cpp/variant/utility_functions.hpp>

namespace Hfsm {
#pragma region HFSMVariableRes
bool HFSMVariableRes::_set(const StringName &p_name, const Variant &p_property) {
	if (p_name == TNAMEOF(default_value)) {
		if (Variant::can_convert(p_property.get_type(), type)) {
			default_value = p_property;
		} else {
			default_value = Variant();
		}
		return true;
	}
	_TRY_SET_PROP(comment);
	return false;
}
bool HFSMVariableRes::_get(const StringName &p_name, Variant &r_property) const {
	if (p_name == TNAMEOF(default_value)) {
		if (Variant::can_convert(default_value.get_type(), type)) {
			r_property = default_value;
		} else {
			r_property = Variant();
		}
		return true;
	}
	_TRY_GET_PROP(comment);
	return false;
}
void HFSMVariableRes::_get_property_list(List<PropertyInfo> *p_list) const {
	if (type != Variant::NIL) {
		p_list->push_back(PropertyInfo(type, TNAMEOF(default_value)));
	}
	_PUSH_PROP(STRING, comment);
}

void HFSMVariableRes::_bind_methods() {
	GDBIND_BEGIN(HFSMVariableRes);
	GDADD_PROPERTY(STRING, variable_name);
	GDADD_PROPERTY(INT, type, PROPERTY_HINT_ENUM, "Trigger,Bool,Int,Float,String");

	GDBIND_SETGET(comment);
	GDBIND_SETGET(default_value);
	// ADD_PROPERTY(PropertyInfo(Variant::NIL, "default_value"),
	//              "set_default_value", "get_default_value");

	// 编辑器专用方法
	GDBIND_SETGET_BOOL(deleted);
	// GDBIND_METHOD(delete_self);
	// ClassDB::bind_method(D_METHOD("delete"), &HFSMVariableRes::delete_self);
}

void HFSMVariableRes::set_variable_name(const StringName &p_name) {
	if (!fsm_res.is_valid()) {
		return;
	}
	bool unique = true;
	variable_name = StringName(p_name);
	do {
		unique = true;
		auto vrl = fsm_res->get_variable_res_list();
		for (auto i = 0; i < vrl.size(); i++) {
			Ref<HFSMVariableRes> v = vrl[i];
			if (v.is_valid() && v.ptr() != this && v->get_variable_name() == variable_name) {
				variable_name = StringName(String("@") + String(variable_name));
				unique = false;
				break;
			}
		}
	} while (!unique);
}
StringName HFSMVariableRes::get_variable_name() { return variable_name; }

void HFSMVariableRes::set_type(Variant::Type p_t) {
	switch (p_t) {
		case Variant::NIL:
		case Variant::BOOL:
		case Variant::INT:
		case Variant::FLOAT:
		case Variant::STRING:
			if (type != p_t) {
				type = Variant::Type(p_t);
				if (type == Variant::STRING && default_value.get_type() != Variant::STRING) {
					default_value = "";
				}
				emit_changed();
			}
			break;
		default:
			ERR_FAIL();
			break;
	}
}
Variant::Type HFSMVariableRes::get_type() const { return type; }

void HFSMVariableRes::set_comment(const String &p_comment) {
	comment = p_comment;
	emit_changed();
}
String HFSMVariableRes::get_comment() const { return comment; }

void HFSMVariableRes::set_deleted(bool p_d) {
	static const StringName sn = "deleted";
	deleted = p_d;
	if (deleted) {
		emit_signal(sn, Ref<HFSMVariableRes>(this));
	}
	emit_changed();
}

void HFSMVariableRes::set_default_value(const Variant &p_default_val) {
	if (default_value != p_default_val) {
		if (Variant::can_convert(p_default_val.get_type(), default_value.get_type())) {
			default_value = p_default_val;
			emit_changed();
		}
	}
}

Variant HFSMVariableRes::get_default_value() {
	if (default_value.get_type() == Variant::NIL) {
		switch (type) {
			case Variant::NIL:
				return nullptr; // TODO:: 原 false? 为啥
			case Variant::BOOL:
				return false;
			case Variant::INT:
				return 0;
			case Variant::FLOAT:
				return 0.0f;
			case Variant::STRING:
				return "";
			default:
				ERR_FAIL_V_MSG(Variant(), "Illegal variable type");
		}
	}
	return default_value;
}
bool HFSMVariableRes::is_deleted() { return deleted; }
void HFSMVariableRes::delete_self() { set_deleted(true); }

Ref<RefCounted> HFSMVariableRes::create_variable() {
	Ref<HFSMVariable> ret;
	ret.instantiate();
	ret->variable_name = variable_name;
	ret->type = type;
	ret->set_value(default_value);
	return ret;
}

Ref<HFSMVariableRes> HFSMVariableRes::create_new(const Ref<FsmRes> &p_fsm_res) {
	Ref<HFSMVariableRes> ret;
	ret.instantiate();
	ret->fsm_res = p_fsm_res;
	ret->set_name(ret->get_variable_name());
	return ret;
}

void HFSMVariableRes::set_fsm_res(const Ref<FsmRes> &p_fsm_res) {
	fsm_res = p_fsm_res;
	set_variable_name(variable_name);
}

Ref<FsmRes> HFSMVariableRes::get_fsm_res() const { return fsm_res; }

#pragma endregion

} // namespace Hfsm
