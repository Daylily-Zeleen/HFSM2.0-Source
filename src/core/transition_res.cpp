#include "transition_res.hpp"
#include "../hfsm_global.hpp"
#include "core/transitions/auto_transition.hpp"
#include "state_res.hpp"
#include "transitions/auto_transition.hpp"
#include "transitions/expression_transition.hpp"
#include "transitions/transition.hpp"
#include "transitions/variable_expressions/trigger_expression.hpp"
#include "transitions/variable_expressions/variable_expression.hpp"
#include "transitions/variable_expressions/variable_expression_res.hpp"
#include "transitions/variable_transition.hpp"

#include <godot_cpp/classes/gd_script.hpp>
#include <godot_cpp/classes/translation_server.hpp>

namespace Hfsm {

#pragma region TransitionRes

bool TransitionRes::_set(const StringName &p_name, const Variant &p_property) {
	if (p_name == StringName("type")) {
		set_type(p_property);
		return true;
	} else if (p_name == StringName("and_mode")) {
		set_and_mode(p_property);
		return true;
	} else if (p_name == StringName("variable_expression_res_list")) {
		set_variable_expression_res_list(p_property);
		return true;
	} else if (p_name == StringName("expression_text")) {
		set_expression_text(p_property);
		return true;
	} else if (p_name == StringName("expression_comment")) {
		set_expression_comment(p_property);
		return true;
	} else if (p_name == StringName("auto_mode")) {
		set_auto_mode(p_property);
		return true;
	} else if (p_name == StringName("delay_msec")) {
		set_auto_delay_msec(p_property);
		return true;
	} else if (p_name == StringName("update_times") || p_name == StringName("physics_update_times")) {
		set_auto_times(p_property);
		return true;
	} else if (p_name == StringName("from") || p_name == StringName("to")) {
		return true;
	}
#ifdef FULL_VERSION
	else if (p_name == StringName("transition_script")) {
		set_transition_script(p_property);
		return true;
	}
#endif
	return false;
}
bool TransitionRes::_get(const StringName &p_name, Variant &r_property) const {
	if (p_name == StringName("from")) {
		if (_from_res.is_valid()) {
			r_property = StringName(_from_res->get("name"));
		} else
			r_property = "";
		return true;
	} else if (p_name == StringName("to")) {
		if (_to_res.is_valid()) {
			r_property = StringName(_to_res->get("name"));
		} else
			r_property = "";
		return true;
	} else if (p_name == StringName("type")) {
		r_property = get_type();
		return true;
	} else if (p_name == StringName("transition_script")) {
		r_property = get_transition_script();
		return true;
	} else if (p_name == StringName("and_mode")) {
		r_property = is_and_mode();
		return true;
	} else if (p_name == StringName("variable_expression_res_list")) {
		r_property = get_variable_expression_res_list();
		return true;
	} else if (p_name == StringName("expression_text")) {
		r_property = get_expression_text();
		return true;
	} else if (p_name == StringName("expression_comment")) {
		r_property = get_expression_comment();
		return true;
	} else if (p_name == StringName("auto_mode")) {
		r_property = get_auto_mode();
		return true;
	} else if (p_name == StringName("delay_msec")) {
		r_property = get_auto_delay_msec();
		return true;
	} else if (p_name == StringName("update_times") || p_name == StringName("physics_update_times")) {
		r_property = get_auto_times();
		return true;
	}
	return false;
}
void TransitionRes::_get_property_list(List<PropertyInfo> *p_list) const {
	p_list->push_back(PropertyInfo(Variant::STRING, "from", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_EDITOR));
	p_list->push_back(PropertyInfo(Variant::STRING, "to", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_EDITOR));
	p_list->push_back(PropertyInfo(Variant::INT, "type", PROPERTY_HINT_ENUM, "Script,HFSMVariable,Expression,Auto"));
	switch (_type) {
		case TRANSITION_TYPE_SCRIPT: {
			p_list->push_back(PropertyInfo(Variant::OBJECT, "transition_script", PROPERTY_HINT_RESOURCE_TYPE, "Script", PROPERTY_USAGE_STORAGE, "Script"));
		} break;
		case TRANSITION_TYPE_VARIABLE: {
			p_list->push_back(PropertyInfo(Variant::BOOL, "and_mode"));

			auto typed_VariableExpressionRes_array_hint_string = String("{0}/{1}:VariableExpressionRes").format(Array::make(Variant::OBJECT, PROPERTY_HINT_RESOURCE_TYPE));

			p_list->push_back(PropertyInfo(Variant::ARRAY, "variable_expression_res_list", PROPERTY_HINT_TYPE_STRING, typed_VariableExpressionRes_array_hint_string));
		} break;
		case TRANSITION_TYPE_EXPRESSION: {
			p_list->push_back(PropertyInfo(Variant::STRING, "expression_text", PROPERTY_HINT_MULTILINE_TEXT));
			p_list->push_back(PropertyInfo(Variant::STRING, "expression_comment", PROPERTY_HINT_MULTILINE_TEXT));
		} break;
		case TRANSITION_TYPE_AUTO: {
			auto p = PropertyInfo(Variant::INT, "auto_mode", PROPERTY_HINT_ENUM, "Delay Timer,Fsm Exit,Manual,Update Times,Physics Update Times");
			p_list->push_back(p);
			switch (_auto_mode) {
				case AUTO_TRANSIT_MODE_DELAY_TIMER: {
					p_list->push_back(PropertyInfo(Variant::INT, "delay_msec", PROPERTY_HINT_RANGE, "0,2147483647,1,or_greater"));
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
		default:
			break;
	}
}

void TransitionRes::_bind_methods() {
	// 通用
	ClassDB::bind_method(D_METHOD("get_from_state_res"), &TransitionRes::get_from_state_res);
	ClassDB::bind_method(D_METHOD("set_from_state_res", "from_state_res"), &TransitionRes::set_from_state_res);
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "from_state_res", PROPERTY_HINT_RESOURCE_TYPE, String("StateRes"), PROPERTY_USAGE_STORAGE, StringName("StateRes")), "set_from_state_res",
			"get_from_state_res");

	ClassDB::bind_method(D_METHOD("get_to_state_res"), &TransitionRes::get_to_state_res);
	ClassDB::bind_method(D_METHOD("set_to_state_res", "to_state_res"), &TransitionRes::set_to_state_res);
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "to_state_res", PROPERTY_HINT_RESOURCE_TYPE, String("StateRes"), PROPERTY_USAGE_STORAGE, StringName("StateRes")), "set_to_state_res",
			"get_to_state_res");

	ClassDB::bind_method(D_METHOD("get_type"), &TransitionRes::get_type);
	ClassDB::bind_method(D_METHOD("set_type", "type"), &TransitionRes::set_type);
	// ADD_PROPERTY(PropertyInfo(Variant::INT, "type", PROPERTY_HINT_ENUM,
	//                           "Script,HFSMVariable,Expression,Auto"),
	//              "set_type", "get_type");

	// Auto
	ClassDB::bind_method(D_METHOD("get_auto_mode"), &TransitionRes::get_auto_mode);
	ClassDB::bind_method(D_METHOD("set_auto_mode", "auto_mode"), &TransitionRes::set_auto_mode);

	ClassDB::bind_method(D_METHOD("get_auto_delay_msec"), &TransitionRes::get_auto_delay_msec);
	ClassDB::bind_method(D_METHOD("set_auto_delay_msec", "delay_time"), &TransitionRes::set_auto_delay_msec);

	ClassDB::bind_method(D_METHOD("get_auto_times"), &TransitionRes::get_auto_times);
	ClassDB::bind_method(D_METHOD("set_auto_times", "auto_times"), &TransitionRes::set_auto_times);
	// 表达式
	ClassDB::bind_method(D_METHOD("get_expression_text"), &TransitionRes::get_expression_text);
	ClassDB::bind_method(D_METHOD("set_expression_text", "expression_text"), &TransitionRes::set_expression_text);

	ClassDB::bind_method(D_METHOD("get_expression_comment"), &TransitionRes::get_expression_comment);
	ClassDB::bind_method(D_METHOD("set_expression_comment", "expression_comment"), &TransitionRes::set_expression_comment);

	// 变量表达式
	ClassDB::bind_method(D_METHOD("is_and_mode"), &TransitionRes::is_and_mode);
	ClassDB::bind_method(D_METHOD("set_and_mode", "and_mode"), &TransitionRes::set_and_mode);

	ClassDB::bind_method(D_METHOD("get_variable_expression_res_list"), &TransitionRes::get_variable_expression_res_list);
	ClassDB::bind_method(D_METHOD("set_variable_expression_res_list", "variable_expression_res_list"), &TransitionRes::set_variable_expression_res_list);

	// ADD_PROPERTY(PropertyInfo(Variant::ARRAY,
	// "variable_expression_res_list"),
	//              "get_variable_expression_res_list",
	//              "set_variable_expression_res_list");

	//  脚本
	ClassDB::bind_method(D_METHOD("get_transition_script"), &TransitionRes::get_transition_script);
	ClassDB::bind_method(D_METHOD("set_transition_script", "transition_script"), &TransitionRes::set_transition_script);
	// ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "transition_script",
	// PROPERTY_HINT_RESOURCE_TYPE, "Script"),
	//              "set_transition_script", "get_transition_script");

	// ClassDB::bind_method(D_METHOD("get_valid_and_texts"),
	//                      &TransitionRes::get_valid_and_texts);
	// 枚举
	BIND_CONSTANT(TRANSITION_TYPE_SCRIPT);
	BIND_CONSTANT(TRANSITION_TYPE_VARIABLE);
	BIND_CONSTANT(TRANSITION_TYPE_EXPRESSION);
	BIND_CONSTANT(TRANSITION_TYPE_AUTO);

	BIND_CONSTANT(AUTO_TRANSIT_MODE_ANIMATION_FINISH);
	BIND_CONSTANT(AUTO_TRANSIT_MODE_DELAY_TIMER);
	BIND_CONSTANT(AUTO_TRANSIT_MODE_FSM_EXIT);
	BIND_CONSTANT(AUTO_TRANSIT_MODE_MANUAL);
	BIND_CONSTANT(AUTO_TRANSIT_MODE_UPDATE_TIMES);
	BIND_CONSTANT(AUTO_TRANSIT_MODE_PHYSICS_UPDATE_TIMES);
	// BIND_CONSTANT(AUTO_TRANSIT_MODE_MAX);
}

TransitionRes::TransitionRes() = default;
TransitionRes::~TransitionRes() = default;

void TransitionRes::set_from_state_res(Ref<StateRes> from_state_res) {
	_from_res = from_state_res;
	notify_property_list_changed();
}
Ref<StateRes> TransitionRes::get_from_state_res() { return _from_res; }

void TransitionRes::set_to_state_res(Ref<StateRes> to_state_res) {
	_to_res = to_state_res;
	notify_property_list_changed();
}
Ref<StateRes> TransitionRes::get_to_state_res() { return _to_res; }
void TransitionRes::set_type(int64_t type) {
	_type = type;
	emit_changed();
	notify_property_list_changed();
}
int64_t TransitionRes::get_type() const { return _type; }

// Auto
void TransitionRes::set_auto_mode(int64_t auto_mode) {
	ERR_FAIL_COND(auto_mode < 0 || auto_mode >= AUTO_TRANSIT_MODE_MAX);
	_auto_mode = auto_mode;
	emit_changed();
	notify_property_list_changed();
}
int64_t TransitionRes::get_auto_mode() const { return _auto_mode; }
void TransitionRes::set_auto_delay_msec(uint64_t delay_time) {
	_auto_delay_msec = delay_time;
	emit_changed();
	// notify_property_list_changed();
}
uint64_t TransitionRes::get_auto_delay_msec() const { return _auto_delay_msec; }
void TransitionRes::set_auto_times(uint64_t times) {
	_auto_times = times;
	emit_changed();
	// notify_property_list_changed();
}
int64_t TransitionRes::get_auto_times() const { return _auto_times; }

// 表达式
void TransitionRes::set_expression_text(const String &expression_text) {
	_expression_text = expression_text;
	emit_changed();
	notify_property_list_changed();
}
String TransitionRes::get_expression_text() const { return _expression_text; }
void TransitionRes::set_expression_comment(const String &expression_comment) {
	_expression_comment = expression_comment;
	emit_changed();
	notify_property_list_changed();
}
String TransitionRes::get_expression_comment() const { return _expression_comment; }

// 变量表达式
void TransitionRes::set_and_mode(bool and_mode) {
	_variable_and_mode = and_mode;
	emit_changed();
	notify_property_list_changed();
}
bool TransitionRes::is_and_mode() const { return _variable_and_mode; }

void TransitionRes::set_variable_expression_res_list(Array variable_expression_res_list) {
	auto emit_changed_callable = Callable(this, "emit_changed");

	auto incoming_connections = get_incoming_connections();
	for (size_t i = 0; i < incoming_connections.size(); i++) {
		Dictionary signal_conn = incoming_connections[i];
		Signal signal = signal_conn["signal"];
		if (signal.is_connected(emit_changed_callable)) {
			signal.disconnect(emit_changed_callable);
		}
		// Callable callable = signal_conn["callable"];
		// if (!signal.is_null() && signal.get_name() == String("changed")) {
		//     Ref<VariableExpressionRes> tmp = signal.get_object();
		//     if ( tmp.is_valid() && callable == emit_changed_callable) {
		//         signal.disconnect(callable);
		//     }
		// }
	}

	_variable_expression_res_list = TypedArray<VariableExpressionRes>(variable_expression_res_list);
	for (size_t i = 0; i < _variable_expression_res_list.size(); i++) {
		Ref<VariableExpressionRes> ver = _variable_expression_res_list[i];
		if (!ver.is_valid()) {
			ver.instantiate();
			_variable_expression_res_list[i] = ver;
		}
		if (!ver->is_connected(StringName("changed"), emit_changed_callable)) {
			ver->connect(StringName("changed"), emit_changed_callable);
		}
	}

	emit_changed();
	notify_property_list_changed();
}
TypedArray<VariableExpressionRes> TransitionRes::get_variable_expression_res_list() const { return _variable_expression_res_list; }

#ifdef FULL_VERSION
// 脚本
void TransitionRes::set_transition_script(Ref<Script> transition_script) {
	_transition_script = transition_script;
	if (_transition_script.is_valid() && Engine::get_singleton()->is_editor_hint()) {
		GDScript *s = Object::cast_to<GDScript>(_transition_script.ptr());
		if (s != nullptr) {
			if (s->get_source_code().is_empty()) {
				s->set_source_code(R"EOF(extends Transition

## <summary>
## Will be called every time when the HFSM update( or physics update)
## Your must to overried this method to determine whether transit to the to state or not.
## </summary>
## <returns> Can transit or not.</returns>
func _can_transit() -> bool:
	# Your check logic.
	# for example:
	# return agents['player'].alive as bool
	return false;


## <summary>
## Will be called every time when the HFSM entry the from state.
## </summary>
func _refresh() -> void:
	pass

)EOF");
			}
		}
	}
	emit_changed();
	notify_property_list_changed();
}
Ref<Script> TransitionRes::get_transition_script() const { return _transition_script; }
#endif

TransitionBase *TransitionRes::create_transition(HFSM *hfsm, Ref<StateRes> &from_state_res, Ref<StateRes> &to_state_res) {
	TransitionBase *r;
	switch (_type) {
#ifdef FULL_VERSION
		case TRANSITION_TYPE_SCRIPT: {
			auto t = memnew(Transition);
			t->set_script(_transition_script);
			t->_hfsm = hfsm;
			r = static_cast<TransitionBase *>(t);
		} break;
#endif
		case TRANSITION_TYPE_VARIABLE: {
			auto vt = memnew(VariableTransition);
			vt->_and_mode = is_and_mode();
			for (size_t i = 0; i < _variable_expression_res_list.size(); i++) {
				Ref<VariableExpressionRes> variable_expression_res = _variable_expression_res_list[i];
				auto ve = variable_expression_res->create_variable_expression(hfsm);
				ERR_FAIL_COND_V(!ve, nullptr);
				switch (ve->get_expression_type()) {
					case VariableExpression::ExpressionType::NORMAL:
						vt->_normal_expressions.append(ve);
						break;
					case VariableExpression::ExpressionType::UNION_TRIGGER:
						vt->_union_triggers.append(static_cast<UnionTriggerExpression *>(ve));
						break;
					case VariableExpression::ExpressionType::SOLO_TRIGGER:
						vt->_solo_triggers.append(static_cast<SoloTriggerExpression *>(ve));
						break;
					default:
						break;
				}
			}
			r = vt;
		} break;
		case TRANSITION_TYPE_EXPRESSION: {
			auto et = memnew(ExpressionTransition);
			et->_hfsm = hfsm;
			et->set_expression_text(_expression_text);
			r = et;
		} break;
		case TRANSITION_TYPE_AUTO: {
			auto at = memnew(AutoTransition);
			at->_mode = _auto_mode;
			at->_delay_msec = _auto_delay_msec;
			at->_times = _auto_times;
			r = at;
		} break;
		default: {
			if (!Engine::get_singleton()->is_editor_hint())
				CRASH_NOW_MSG("Illegal transition type.");
		} break;
	}
	from_state_res = get_from_state_res();
	to_state_res = get_to_state_res();
	return r;
}

#pragma endregion

}; // namespace Hfsm