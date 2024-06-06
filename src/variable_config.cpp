/**************************************************************************/
/*  variable_config.cpp                                                   */
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

#include "variable_config.h"

#include "fsm_config.h"

#include "variable.h"

namespace HFSM2 {
#pragma region VariableConfig
bool VariableConfig::_set(const StringName &p_name, const Variant &p_property) {
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
bool VariableConfig::_get(const StringName &p_name, Variant &r_property) const {
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
void VariableConfig::_get_property_list(List<PropertyInfo> *p_list) const {
	if (type != Variant::NIL) {
		p_list->push_back(PropertyInfo(type, TNAMEOF(default_value)));
	}
	_PUSH_PROP(STRING, comment);
}

void VariableConfig::_bind_methods() {
	GDBIND_BEGIN(VariableConfig);
	GDADD_PROPERTY(STRING, variable_name);
	GDADD_PROPERTY(INT, type, PROPERTY_HINT_ENUM, "Trigger,Bool,Int,Float,String");

	GDBIND_SETGET(comment);
	GDBIND_SETGET(default_value);
	// ADD_PROPERTY(PropertyInfo(Variant::NIL, "default_value"),
	//              "set_default_value", "get_default_value");
}

void VariableConfig::set_variable_name(const StringName &p_name) {
	auto fsm_config = get_fsm_config();
	if (fsm_config.is_null()) {
		return;
	}

	// Only check if FSMConfig valid.
	variable_name = StringName(p_name);
	if (fsm_config.is_valid()) {
		bool unique = true;
		do {
			unique = true;
			auto vrl = get_fsm_config()->get_variable_config_list();
			for (auto i = 0; i < vrl.size(); i++) {
				Ref<VariableConfig> v = vrl[i];
				if (v.is_valid() && v.ptr() != this && v->get_variable_name() == variable_name) {
					variable_name = StringName(String("@") + String(variable_name));
					unique = false;
					break;
				}
			}
		} while (!unique);
	}

	set_name(String(variable_name) + ": " + get_type_text());
	emit_changed();
}

StringName VariableConfig::get_variable_name() { return variable_name; }

void VariableConfig::set_type(Variant::Type p_t) {
	switch (p_t) {
		case Variant::NIL:
		case Variant::BOOL:
		case Variant::INT:
		case Variant::FLOAT:
		case Variant::STRING: {
			if (type != p_t) {
				type = Variant::Type(p_t);
				if (type == Variant::STRING && default_value.get_type() != Variant::STRING) {
					default_value = "";
				}
				set_name(String(variable_name) + ": " + get_type_text());
				emit_changed();
			}
		} break;
		default:
			set_name(String(variable_name) + ": " + get_type_text());
			ERR_FAIL();
			break;
	}
}
Variant::Type VariableConfig::get_type() const { return type; }

void VariableConfig::set_comment(const String &p_comment) {
	comment = p_comment;
	emit_changed();
}
String VariableConfig::get_comment() const { return comment; }

void VariableConfig::set_default_value(const Variant &p_default_val) {
	if (default_value != p_default_val) {
		if (Variant::can_convert(p_default_val.get_type(), default_value.get_type())) {
			default_value = p_default_val;
			emit_changed();
		}
	}
}

Variant VariableConfig::get_default_value() const {
	if (default_value.get_type() == Variant::NIL) {
		switch (type) {
			case Variant::BOOL:
				return false;
			case Variant::INT:
				return 0;
			case Variant::FLOAT:
				return 0.0f;
			case Variant::STRING:
				return "";
			case Variant::NIL: {
				IF_DEBUG(WARN_PRINT(vformat("[%s:%d]", __FILE__, __LINE__) + vformat("Trigger type is using boolean, but \"get_default_value()\" should not be called."));)
				return false;
			}
			default:
				ERR_FAIL_V_MSG({}, "Illegal variable type");
		}
	}
	return default_value;
}

Ref<Variable> VariableConfig::create_variable() {
	return memnew(Variable(variable_name, type, default_value));
}

Ref<VariableConfig> VariableConfig::create_new(const Ref<FSMConfig> &p_fsm_config) {
	Ref<VariableConfig> ret;
	ret.instantiate();
	ret->fsm_config = p_fsm_config;
	ret->set_variable_name(ret->get_variable_name());
	return ret;
}

String VariableConfig::get_type_text() const {
	switch (get_type()) {
		case Variant::NIL:
			return "Trigger";
		case Variant::BOOL:
			return "Bool";
		case Variant::INT:
			return "Int";
		case Variant::FLOAT:
			return "Float";
		case Variant::STRING:
			return "String";
		default:
			ERR_FAIL_V_MSG("Unknowm", "Invald type: " + itos(get_type()));
	}
}

void VariableConfig::set_fsm_config(const Ref<FSMConfig> &p_fsm_config) {
	if (p_fsm_config.is_valid()) {
		fsm_config = weakref(p_fsm_config);
		set_variable_name(variable_name);
	} else {
		fsm_config.unref();
	}
}

Ref<FSMConfig> VariableConfig::get_fsm_config() const { return get_ref<Ref<FSMConfig>>(fsm_config); }

#pragma endregion

#if TOOLS_ENABLED
Array VariableConfig::debug_serialize() const {
	// fsm_config 不需要，将在反序列化时被设置
	return make_arr<Array>(variable_name, type, default_value, comment);
}

Ref<VariableConfig> VariableConfig::debug_deserialize(const Array &p_data) {
	Ref<VariableConfig> ret;
	ret.instantiate();
	ret->variable_name = p_data[0];
	ret->type = Variant::Type(p_data[1].operator int());
	ret->default_value = p_data[2];
	ret->comment = p_data[3];
	return ret;
}

#endif // TOOLS_ENABLED

} // namespace HFSM2
