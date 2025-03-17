/**************************************************************************/
/*  transition_config.cpp                                                 */
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

#include "transition_config.h"
#include "transitions/auto_transition.h"
#include "transitions/expression_transition.h"
#include "transitions/transition.h"
#include "transitions/variable_expressions/trigger_expression.h"
#include "transitions/variable_expressions/variable_expression.h"
#include "transitions/variable_expressions_transition.h"

#ifdef GDEXTENSION_BUILD

#include <godot_cpp/classes/gd_script.hpp>
#include <godot_cpp/classes/resource_saver.hpp>
#include <godot_cpp/classes/translation_server.hpp>

#ifdef MODULE_MONO_ENABLED
#include <godot_cpp/classes/c_sharp_script.hpp>
#endif // MODULE_MONO_ENABLED

#else // GDEXTENSION_BUILD
#include <core/string/translation.h>
#include <modules/gdscript/gdscript.h>

#ifdef MODULE_MONO_ENABLED
#include <modules/mono/csharp_script.h>
#endif // MODULE_MONO_ENABLED

#endif //GDEXTENSION_BUILD

namespace HFSM2 {

#pragma region TransitionConfig

#define GD_TEMPLATE                                                                              \
	"extends Transition\n\n\n"                                                                   \
	"# Will be called every time when the HFSM update( or physics update)\n"                     \
	"# Your must to overried this method to determine whether transit to the to state or not.\n" \
	"# <returns> Can transit or not.</returns>\n"                                                \
	"func _can_transit() -> bool:\n"                                                             \
	"	# Your check logic.\n"                                                                     \
	"	return false;\n\n\n"                                                                       \
	"# Will be called every time when the HFSM enter the from state.\n"                          \
	"func _refresh() -> void:\n"                                                                 \
	"	pass\n"

#ifdef MODULE_MONO_ENABLED
#define CSHARP_TEMPLATE                                                                                \
	"// Because GDExtension has not ability to generate binding for C#, extends form RefCounted here." \
	"public partial class MyTransition: Godot.RefCounted\n"                                            \
	"{\n"                                                                                              \
	"	// <summary>\n"                                                                                  \
	"	// Will be called every time when the HFSM update( or physics update)\n"                         \
	"	// Your must to overried this method to determine whether transit to the to state or not.\n"     \
	"	// </summary>\n"                                                                                 \
	"	// <returns> Can transit or not.</returns>\n"                                                    \
	"	private bool _can_transit()\n"                                                                   \
	"	{\n"                                                                                             \
	"		return false;\n"                                                                                \
	"	}\n\n"                                                                                           \
	"	// <summary>\n"                                                                                  \
	"	// Will be called every time when the HFSM enter the from state.\n"                              \
	"	// </summary>\n"                                                                                 \
	"	private void _refresh()\n"                                                                       \
	"	{\n"                                                                                             \
	"	}\n"                                                                                             \
	"}"
#endif // MODULE_MONO_ENABLED

bool TransitionConfig::_set(const StringName &p_name, const Variant &p_property) {
	_TRY_SET_PROP(variable_expression_config_list);
	_TRY_SET_PROP(expression_text);
	_TRY_SET_PROP(expression_comment);
	_TRY_SET_PROP(auto_mode);
	_TRY_SET_PROP(auto_delay_msec);
	_TRY_SET_PROP(and_mode);

	IF_FULL_VERSION(_TRY_SET_PROP(transition_script);)

	if (p_name == StringName("update_times") || p_name == StringName("physics_update_times")) {
		set_auto_times(p_property);
		return true;
	} else if (p_name == StringName("from") || p_name == StringName("to")) {
		return true;
	}
	return false;
}

bool TransitionConfig::_get(const StringName &p_name, Variant &r_property) const {
	if (p_name == StringName("from")) {
		if (from_state_config.is_valid()) {
			r_property = from_state_config->get_state_name();
		} else {
			r_property = "";
		}
		return true;
	} else if (p_name == StringName("to")) {
		if (to_state_config.is_valid()) {
			r_property = to_state_config->get_state_name();
		} else {
			r_property = "";
		}
		return true;

	} else if (p_name == StringName("update_times") || p_name == StringName("physics_update_times")) {
		r_property = get_auto_times();
		return true;
	}

	IF_FULL_VERSION(_TRY_GET_PROP(transition_script);)

	_TRY_GET_PROP(variable_expression_config_list);
	_TRY_GET_PROP(expression_text);
	_TRY_GET_PROP(expression_comment);
	_TRY_GET_PROP(auto_mode);
	_TRY_GET_PROP(auto_delay_msec);
	_TRY_GET_PROPB(and_mode);
	return false;
}

void TransitionConfig::_get_property_list(List<PropertyInfo> *p_list) const {
	p_list->push_back(PropertyInfo(Variant::STRING, "from", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY));
	p_list->push_back(PropertyInfo(Variant::STRING, "to", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY));

	switch (type) {
		case TRANSITION_TYPE_EXPRESSION: {
			_PUSH_PROP(STRING, expression_text, PROPERTY_HINT_MULTILINE_TEXT);
			_PUSH_PROP(STRING, expression_comment, PROPERTY_HINT_MULTILINE_TEXT);
		} break;
		case TRANSITION_TYPE_AUTO: {
			_PUSH_PROP(INT, auto_mode, PROPERTY_HINT_ENUM, "Delay Timer,Animation Finish,FSM Exit,Manual,Update Times,Physics Update Times");
			switch (auto_mode) {
				case AUTO_TRANSIT_MODE_DELAY_TIMER: {
					_PUSH_PROP(INT, auto_delay_msec, PROPERTY_HINT_RANGE, "0,2147483647,1,or_greater");
				} break;
				case AUTO_TRANSIT_MODE_UPDATE_TIMES: {
					p_list->push_back(PropertyInfo(Variant::INT, "update_times", PROPERTY_HINT_RANGE, "0,2147483647,1,or_greater"));
				} break;
				case AUTO_TRANSIT_MODE_PHYSICS_UPDATE_TIMES: {
					p_list->push_back(PropertyInfo(Variant::INT, "physics_update_times", PROPERTY_HINT_RANGE, "0,2147483647,1,or_greater"));
				} break;
				default: // AUTO_TRANSIT_MODE_MANUAL,  AUTO_TRANSIT_MODE_FSM_EXIT
					break;
			}
		} break;
		case TRANSITION_TYPE_VARIABLE_EXPRESSIONS: {
			_PUSH_PROP(BOOL, and_mode);
			_PUSH_PROP_TYPED_ARRAY(variable_expression_config_list, VariableExpressionConfig);
		} break;
#ifdef FULL_VERSION
		case TRANSITION_TYPE_SCRIPT: {
			_PUSH_PROP_RESOURCE(transition_script);
		} break;
#endif // FULL_VERSION

		default:
			break;
	}
}

void TransitionConfig::_bind_methods() {
	GDBIND_BEGIN(TransitionConfig);
	// 通用
	GDADD_PROPERTY_RESOURCE(from_state_config, PROPERTY_USAGE_STORAGE);
	GDADD_PROPERTY_RESOURCE(to_state_config, PROPERTY_USAGE_STORAGE);
	String type_hint_string = "Auto,Expression,Variable Compare Expressions";
	IF_FULL_VERSION(type_hint_string += String(",Script"));
	GDADD_PROPERTY(INT, type, PROPERTY_HINT_ENUM, type_hint_string);

	// Auto
	GDBIND_SETGET(auto_mode);
	GDBIND_SETGET(auto_delay_msec);
	GDBIND_SETGET(auto_times);
	// 表达式
	GDBIND_SETGET(expression_text);
	GDBIND_SETGET(expression_comment);
	// 变量表达式
	GDBIND_SETGET_BOOL(and_mode);
	GDBIND_SETGET(variable_expression_config_list);
	//  脚本
	IF_FULL_VERSION(GDBIND_SETGET(transition_script);)

	// 枚举
	BIND_ENUM_CONSTANT(TRANSITION_TYPE_AUTO);
	BIND_ENUM_CONSTANT(TRANSITION_TYPE_EXPRESSION);
	BIND_ENUM_CONSTANT(TRANSITION_TYPE_VARIABLE_EXPRESSIONS);
	IF_FULL_VERSION(BIND_ENUM_CONSTANT(TRANSITION_TYPE_SCRIPT);)
	BIND_ENUM_CONSTANT(TRANSITION_TYPE_MAX);

	BIND_ENUM_CONSTANT(AUTO_TRANSIT_MODE_DELAY_TIMER);
	BIND_ENUM_CONSTANT(AUTO_TRANSIT_MODE_ANIMATION_FINISH);
	BIND_ENUM_CONSTANT(AUTO_TRANSIT_MODE_FSM_EXIT);
	BIND_ENUM_CONSTANT(AUTO_TRANSIT_MODE_MANUAL);
	BIND_ENUM_CONSTANT(AUTO_TRANSIT_MODE_UPDATE_TIMES);
	BIND_ENUM_CONSTANT(AUTO_TRANSIT_MODE_PHYSICS_UPDATE_TIMES);
	BIND_ENUM_CONSTANT(AUTO_TRANSIT_MODE_MAX);
}

void TransitionConfig::set_from_state_config(const Ref<StateConfig> &p_from_state_config) {
	from_state_config = p_from_state_config;
	emit_changed();
}
Ref<StateConfig> TransitionConfig::get_from_state_config() const { return from_state_config; }

void TransitionConfig::set_to_state_config(const Ref<StateConfig> &p_to_state_config) {
	to_state_config = p_to_state_config;
	emit_changed();
}
Ref<StateConfig> TransitionConfig::get_to_state_config() const { return to_state_config; }
void TransitionConfig::set_type(TransitionType p_type) {
	type = p_type;
	emit_changed();
	notify_property_list_changed();
}
TransitionConfig::TransitionType TransitionConfig::get_type() const { return type; }

// Auto
void TransitionConfig::set_auto_mode(AuotoTtransitMode p_auto_mode) {
	ERR_FAIL_COND(p_auto_mode < 0 || p_auto_mode >= AUTO_TRANSIT_MODE_MAX);
	auto_mode = p_auto_mode;
	emit_changed();
	notify_property_list_changed();
}
TransitionConfig::AuotoTtransitMode TransitionConfig::get_auto_mode() const { return auto_mode; }
void TransitionConfig::set_auto_delay_msec(uint64_t p_delay_time) {
	auto_delay_msec = p_delay_time;
	emit_changed();
}
uint64_t TransitionConfig::get_auto_delay_msec() const { return auto_delay_msec; }
void TransitionConfig::set_auto_times(uint64_t p_times) {
	auto_times = p_times;
	emit_changed();
}
int64_t TransitionConfig::get_auto_times() const { return auto_times; }

// 表达式
void TransitionConfig::set_expression_text(const String &p_expression_text) {
	expression_text = p_expression_text;
	emit_changed();
}
String TransitionConfig::get_expression_text() const { return expression_text; }
void TransitionConfig::set_expression_comment(const String &p_expression_comment) {
	expression_comment = p_expression_comment;
	emit_changed();
}
String TransitionConfig::get_expression_comment() const { return expression_comment; }

// 变量表达式
void TransitionConfig::set_and_mode(bool p_and_mode) {
	and_mode = p_and_mode;
	emit_changed();
}
bool TransitionConfig::is_and_mode() const { return and_mode; }

void TransitionConfig::set_variable_expression_config_list(const Array &p_variable_expression_config_list) {
	auto cb = Callable(this, SNAME("emit_changed"));
	for (auto i = 0; i < variable_expression_config_list.size(); ++i) {
		Ref<VariableExpressionConfig> vec = variable_expression_config_list[i];
		if (vec.is_valid() && vec->is_connected(s_changed, cb)) {
			vec->disconnect(s_changed, cb);
		}
	}

	variable_expression_config_list = decltype(variable_expression_config_list)(p_variable_expression_config_list);

	for (auto i = 0; i < variable_expression_config_list.size(); ++i) {
		Ref<VariableExpressionConfig> vec = variable_expression_config_list[i];
		if (vec.is_valid() && !vec->is_connected(s_changed, cb)) {
			vec->connect(s_changed, cb);
		}
	}

	emit_changed();
}

TypedArray<VariableExpressionConfig> TransitionConfig::get_variable_expression_config_list() const {
	return variable_expression_config_list;
}

#ifdef FULL_VERSION
// 脚本
void TransitionConfig::set_transition_script(const Ref<Script> &p_transition_script) {
	auto cb = TCALLABLE(set_transition_script);
	if (transition_script.is_valid() && transition_script->is_connected(s_changed, cb)) {
		transition_script->disconnect(s_changed, cb);
	}

	transition_script = p_transition_script;
	if (transition_script.is_null()) {
		script_valid = true;
	} else {
		if (transition_script.is_valid() && !transition_script->is_connected(s_changed, cb)) {
			transition_script->connect(s_changed, TCALLABLE_BIND(set_transition_script, transition_script));
		}

		IF_TOOLS({
			IF_MONO(Utils::set_template_if_source_code_is_empty(transition_script, GD_TEMPLATE, CSHARP_TEMPLATE);)
			IF_NOT_MONO(Utils::set_template_if_source_code_is_empty(state_script, GD_TEMPLATE);)
		})

		script_valid = Utils::is_script_instance_type_valid(transition_script, Transition::get_class_static(), [] {static const LocalVector<StringName> methods =  { "_refresh", "can_transit" };return methods; });

		if (!script_valid) {
			ED_MSG("HFSM: The Script \"%s\" set to Transition is invalid (not extended from \"%s\" (GDScript) or have illegal methods).", transition_script->get_path(), Transition::get_class_static());
		}
	}

	call_deferred(SNAME("emit_changed"));
}
Ref<Script> TransitionConfig::get_transition_script() const { return transition_script; }
bool TransitionConfig::is_script_valid() const { return script_valid; }

#endif // FULL_VERSION

TransitionBase *TransitionConfig::create_transition(HFSM *p_hfsm) {
	TransitionBase *ret = nullptr;
	switch (type) {
		case TRANSITION_TYPE_AUTO: {
			ret = memnew(AutoTransition(auto_mode, auto_delay_msec, auto_times));
		} break;
		case TRANSITION_TYPE_EXPRESSION: {
			ret = memnew(ExpressionTransition(p_hfsm, expression_text));
		} break;
		case TRANSITION_TYPE_VARIABLE_EXPRESSIONS: {
			auto vt = memnew(VariableExpressionsTransition);
			vt->and_mode = is_and_mode();
			for (size_t i = 0; i < variable_expression_config_list.size(); i++) {
				Ref<VariableExpressionConfig> variable_expression_config = variable_expression_config_list[i];
				auto ve = variable_expression_config->create_variable_expression(p_hfsm);
				ERR_FAIL_COND_V_MSG(!ve, memnew(AutoTransition(AUTO_TRANSIT_MODE_MANUAL, auto_delay_msec, auto_times)), "Create Variable Expression Transition failed, will create a manual Transition to replace it.");
				switch (ve->get_expression_type()) {
					case VariableExpression::ExpressionType::EXPRESSION_TYPE_NORMAL:
						vt->normal_expressions.append(ve);
						break;
					case VariableExpression::ExpressionType::EXPRESSION_TYPE_UNION_TRIGGER:
						vt->union_triggers.append(static_cast<UnionTriggerExpression *>(ve));
						break;
					case VariableExpression::ExpressionType::EXPRESSION_TYPE_SOLO_TRIGGER:
						vt->solo_triggers.append(static_cast<SoloTriggerExpression *>(ve));
						break;
					default:
						break;
				}
			}
			ret = vt;
		} break;
#ifdef FULL_VERSION
		case TRANSITION_TYPE_SCRIPT: {
			ret = memnew(Transition(p_hfsm));
			if (!script_valid) {
				WARN_PRINT(vformat("\"%s\" is not a valid script for Transition, will create a Transition without script.", transition_script->get_path()));
			} else {
				static_cast<Transition *>(ret)->set_script(transition_script);
			}
		} break;
#endif // FULL_VERSION
		default: {
			if (!Engine::get_singleton()->is_editor_hint()) {
				CRASH_NOW_MSG("Illegal transition type: " + itos(type));
			}
		} break;
	}
	return ret;
}

#pragma endregion

#if TOOLS_ENABLED
Array TransitionConfig::debug_serialize(const Ref<FSMConfig> &p_fsm_config, const Ref<FSMConfig> &p_root_config) const {
	int from_idx = p_fsm_config->get_state_config_list().find(from_state_config);
	int to_idx = p_fsm_config->get_state_config_list().find(to_state_config);

	Array variable_expression_configs;
	for (int64_t i = 0; i < variable_expression_config_list.size(); ++i) {
		Ref<VariableExpressionConfig> vec = variable_expression_config_list[i];
		variable_expression_configs.push_back(vec->debug_serialize(p_root_config));
	}

#ifdef FULL_VERSION
	// Script
	Dictionary script_info;
	if (transition_script.is_valid()) {
		bool real_file;
		IF_GDM(real_file = FileAccess::exists(transition_script->get_path());)
		IF_GDE(real_file = FileAccess::file_exists(transition_script->get_path());)
		script_info["path"] = real_file ? transition_script->get_path() : "";
		script_info["type"] = transition_script->get_class();
		script_info["source_code"] = real_file ? "" : transition_script->get_source_code();
	}
#endif // FULL_VERSION

	return make_arr<Array>(
			from_idx, to_idx, type,
			auto_mode, auto_delay_msec, auto_times,
			expression_text, expression_comment,
			and_mode, variable_expression_configs
#ifdef FULL_VERSION
			,
			script_info, script_valid,
#endif // FULL_VERSION
	);
}

Ref<TransitionConfig> TransitionConfig::debug_deserialize(const Array &p_data, const Ref<FSMConfig> &p_fsm_config, const Ref<FSMConfig> &p_root_config) {
	Ref<TransitionConfig> ret;
	ret.instantiate();

	int from_idx = p_data[0];
	int to_idx = p_data[1];
	if (from_idx >= 0) {
		ret->from_state_config = p_fsm_config->get_state_config_list()[from_idx];
	}
	if (to_idx >= 0) {
		ret->to_state_config = p_fsm_config->get_state_config_list()[to_idx];
	}

	ret->type = TransitionType(p_data[2].operator int());
	ret->auto_mode = AuotoTtransitMode(p_data[3].operator int());
	ret->auto_delay_msec = p_data[4];
	ret->auto_times = p_data[5];

	ret->expression_text = p_data[6];
	ret->expression_comment = p_data[7];

	ret->and_mode = p_data[8];

	Array raw_variable_expression_configs = p_data[9];
	Array variable_expression_configs;
	for (int64_t i = 0; i < raw_variable_expression_configs.size(); ++i) {
		auto vec = VariableExpressionConfig::debug_deserialize(raw_variable_expression_configs[i], p_root_config);
		variable_expression_configs.push_back(vec);
	}
	ret->set_variable_expression_config_list(variable_expression_configs);

#ifdef FULL_VERSION
	Dictionary script_info = p_data[10];
	if (!script_info.is_empty()) {
		String path = script_info["path"];
		Ref<Script> script;
		if (path.is_empty()) {
			script = ClassDB::instantiate(script_info["type"]);
			script->set_source_code(script_info["source_code"]);
		} else {
			IF_GDE(script = ResourceLoader::get_singleton()->load(path);)
			IF_GDM(script = ResourceLoader::load(path);)
		}

		if (script.is_valid()) {
			ret->transition_script = script;
		}
	}
	ret->script_valid = p_data[11];
#endif // FULL_VERSION

	return ret;
}
#endif // TOOLS_ENABLED
}; // namespace HFSM2
