#include "hfsm_variable_res.hpp"
#include "fsm_res.hpp"
#include "hfsm_variable.hpp"

#include "fsm_res.hpp"
#include <godot_cpp/variant/utility_functions.hpp>

namespace Hfsm {
#pragma region HFSMVariableRes
bool HFSMVariableRes::_set(const StringName &p_name, const Variant &p_property) {
	if (p_name == StringName("default_value")) {
		if (Variant::can_convert(p_property.get_type(), _type)) {
			_default_val = p_property;
		} else {
			_default_val = Variant();
		}
		return true;
	} else if (p_name == StringName("comment")) {
		set_comment(p_property);
		return true;
	}
	return false;
}
bool HFSMVariableRes::_get(const StringName &p_name, Variant &r_property) const {
	if (p_name == StringName("default_value")) {
		if (Variant::can_convert(_default_val.get_type(), _type)) {
			r_property = _default_val;
		} else {
			r_property = Variant();
		}
		return true;
	} else if (p_name == StringName("comment")) {
		r_property = get_comment();
		return true;
	}
	return false;
}
void HFSMVariableRes::_get_property_list(List<PropertyInfo> *p_list) const {
	if (_type != Variant::NIL) {
		p_list->push_back(PropertyInfo(_type, "default_value"));
	}
	p_list->push_back(PropertyInfo(Variant::STRING, "comment"));
}

void HFSMVariableRes::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_variable_name"), &HFSMVariableRes::get_variable_name);
	ClassDB::bind_method(D_METHOD("set_variable_name", "name"), &HFSMVariableRes::set_variable_name);
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "variable_name"), "set_variable_name", "get_variables_name");

	ClassDB::bind_method(D_METHOD("get_type"), &HFSMVariableRes::get_type);
	ClassDB::bind_method(D_METHOD("set_type", "type"), &HFSMVariableRes::set_type);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "type", PROPERTY_HINT_ENUM, "Trigger,Bool,Int,Float,String"), "set_type", "get_type");

	ClassDB::bind_method(D_METHOD("get_comment"), &HFSMVariableRes::get_comment);
	ClassDB::bind_method(D_METHOD("set_comment", "comment"), &HFSMVariableRes::set_comment);

	ClassDB::bind_method(D_METHOD("get_default_value"), &HFSMVariableRes::get_default_val);
	ClassDB::bind_method(D_METHOD("set_default_value", "default_val"), &HFSMVariableRes::set_default_val);
	// ADD_PROPERTY(PropertyInfo(Variant::NIL, "default_value"),
	//              "set_default_value", "get_default_value");

	// 编辑器专用方法
	ClassDB::bind_method(D_METHOD("is_deleted"), &HFSMVariableRes::is_deleted);
	ClassDB::bind_method(D_METHOD("set_deleted", "delete"), &HFSMVariableRes::set_deleted);
	ClassDB::bind_method(D_METHOD("delete"), &HFSMVariableRes::delete_self);
}

HFSMVariableRes::HFSMVariableRes() = default;

void HFSMVariableRes::set_variable_name(const StringName &name) {
	// UtilityFunctions::print("set: ", name);
	if (!_fsm_res.is_valid()) {
		return;
	}
	bool unique = true;
	_name = StringName(name);
	do {
		unique = true;
		auto vrl = (static_cast<FsmRes *>(_fsm_res.ptr()))->_variable_res_list;
		for (auto i = 0; i < vrl.size(); i++) {
			Ref<HFSMVariableRes> v = vrl[i];
			if (v.is_valid() && v.ptr() != this && v->get("name") == _name) {
				_name = StringName(String("@") + String(_name));
				unique = false;
				break;
			}
		}
	} while (!unique);
}
StringName HFSMVariableRes::get_variable_name() { return _name; }

void HFSMVariableRes::set_type(int32_t t) {
	switch (t) {
		case Variant::NIL:
		case Variant::BOOL:
		case Variant::INT:
		case Variant::FLOAT:
		case Variant::STRING:
			if (_type != t) {
				_type = Variant::Type(t);
				if (_type == Variant::STRING && _default_val.get_type() != Variant::STRING) {
					_default_val = "";
				}
				notify_property_list_changed();
			}
			break;
		default:
			ERR_FAIL();
			break;
	}
}
int32_t HFSMVariableRes::get_type() const { return _type; }

void HFSMVariableRes::set_comment(const String &comment) {
	_comment = comment;
	notify_property_list_changed();
}
String HFSMVariableRes::get_comment() const { return _comment; }

void HFSMVariableRes::set_deleted(bool d) {
	static const StringName sn = "deleted";
	_deleted = d;
	if (_deleted) {
		emit_signal(sn, Ref<HFSMVariableRes>(this));
	}
	notify_property_list_changed();
}

void HFSMVariableRes::set_default_val(const Variant &default_val) {
	if (_default_val != default_val) {
		if (Variant::can_convert(default_val.get_type(), _default_val.get_type())) {
			_default_val = default_val;
			notify_property_list_changed();
		}
	}
	notify_property_list_changed();
}

Variant HFSMVariableRes::get_default_val() {
	if (_default_val.get_type() == Variant::NIL) {
		switch (_type) {
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
	return _default_val;
}
bool HFSMVariableRes::is_deleted() { return _deleted; }
void HFSMVariableRes::delete_self() { set_deleted(true); }

Ref<RefCounted> HFSMVariableRes::create_variable() {
	Ref<HFSMVariable> ret;
	ret.instantiate();
	ret->_name = _name;
	ret->_type = _type;
	ret->set_value(_default_val);
	return ret;
}

Ref<HFSMVariableRes> HFSMVariableRes::create_new(const Ref<FsmRes> &fsm_res) {
	Ref<HFSMVariableRes> ret;
	ret.instantiate();
	ret->_fsm_res = fsm_res;
	ret->set_name(ret->get_variable_name());
	return ret;
}

void HFSMVariableRes::set_fsm_res(const Ref<FsmRes> &fsm_res) {
	_fsm_res = fsm_res;
	set_variable_name(_name);
}

Ref<FsmRes> HFSMVariableRes::get_fsm_res() const { return _fsm_res; }

#pragma endregion

} // namespace Hfsm
