#include "transition_res.hpp"
#include "core/transitions/auto_transition.hpp"
#include "state_res.hpp"
#include "transitions/auto_transition.hpp"
#include "transitions/expression_transition.hpp"
#include "transitions/transition.hpp"
#include "transitions/variable_expressions/trigger_expression.hpp"
#include "transitions/variable_expressions/variable_expression.hpp"
#include "transitions/variable_expressions/variable_expression_res.hpp"
#include "transitions/variable_transition.hpp"
#include <hfsm_global.hpp>


#include <godot_cpp/classes/gd_script.hpp>
#include <godot_cpp/classes/translation_server.hpp>

namespace Hfsm {

#pragma region TransitionRes

bool TransitionRes::_set(const StringName &p_name, const Variant &p_property) {
	_TRY_SET_PROP(type);
	_TRY_SET_PROP(variable_expression_res_list);
	_TRY_SET_PROP(expression_text);
	_TRY_SET_PROP(expression_comment);
	_TRY_SET_PROP(auto_mode);
	_TRY_SET_PROP(auto_delay_msec);
#ifdef FULL_VERSION
	_TRY_SET_PROP(transition_script);
#endif

	if (p_name == StringName("update_times") || p_name == StringName("physics_update_times")) {
		set_auto_times(p_property);
		return true;
	} else if (p_name == StringName("from") || p_name == StringName("to")) {
		return true;
	}
	if (p_name == StringName("and_mode")) {
		set_variable_and_mode(p_property);
		return true;
	}
	return false;
}

bool TransitionRes::_get(const StringName &p_name, Variant &r_property) const {
	if (p_name == StringName("from")) {
		if (from_state_res.is_valid()) {
			r_property = from_state_res->get_state_name();
		} else {
			r_property = "";
		}
		return true;
	} else if (p_name == StringName("to")) {
		if (to_state_res.is_valid()) {
			r_property = to_state_res->get_state_name();
		} else {
			r_property = "";
		}
		return true;
	} else if (p_name == StringName("and_mode")) {
		r_property = is_variable_and_mode();
		return true;
	} else if (p_name == StringName("update_times") || p_name == StringName("physics_update_times")) {
		r_property = get_auto_times();
		return true;
	}

#ifdef FULL_VERSION
	_TRY_GET_PROP(transition_script);
#endif // FULL_VERSION
	_TRY_GET_PROP(type);
	_TRY_GET_PROP(variable_expression_res_list);
	_TRY_GET_PROP(expression_text);
	_TRY_GET_PROP(expression_comment);
	_TRY_GET_PROP(auto_mode);
	_TRY_GET_PROP(auto_delay_msec);
	return false;
}

void TransitionRes::_get_property_list(List<PropertyInfo> *p_list) const {
	p_list->push_back(PropertyInfo(Variant::STRING, "from", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY));
	p_list->push_back(PropertyInfo(Variant::STRING, "to", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_READ_ONLY));
	_PUSH_PROP(INT, type, PROPERTY_HINT_ENUM, "Script,HFSMVariable,Expression,Auto");
	switch (type) {
#ifdef FULL_VERSION
		case TRANSITION_TYPE_SCRIPT: {
			_PUSH_PROP_RESOURCE(transition_script, PROPERTY_USAGE_STORAGE, Script::get_class_static());
		} break;
#endif // FULL_VERSION
		case TRANSITION_TYPE_VARIABLE: {
			p_list->push_back(PropertyInfo(Variant::BOOL, "and_mode"));

			_PUSH_PROP_TYPED_ARRAY(variable_expression_res_list, VariableExpressionRes);
		} break;
		case TRANSITION_TYPE_EXPRESSION: {
			_PUSH_PROP(STRING, expression_text, PROPERTY_HINT_MULTILINE_TEXT);
			_PUSH_PROP(STRING, expression_comment, PROPERTY_HINT_MULTILINE_TEXT);
		} break;
		case TRANSITION_TYPE_AUTO: {
			_PUSH_PROP(INT, auto_mode, PROPERTY_HINT_ENUM, "Delay Timer,Fsm Exit,Manual,Update Times,Physics Update Times");
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
		default:
			break;
	}
}

void TransitionRes::_bind_methods() {
	GDBIND_BEGIN(TransitionRes);
	// 通用
	GDADD_PROPERTY_RESOURCE(from_state_res, PROPERTY_USAGE_STORAGE);
	GDADD_PROPERTY_RESOURCE(to_state_res, PROPERTY_USAGE_STORAGE);
	GDBIND_SETGET(type);
	// ADD_PROPERTY(PropertyInfo(Variant::INT, "type", PROPERTY_HINT_ENUM,
	//                           "Script,HFSMVariable,Expression,Auto"),
	//              "set_type", "get_type");

	// Auto
	GDBIND_SETGET(auto_mode);
	GDBIND_SETGET(auto_delay_msec);
	GDBIND_SETGET(auto_times);
	// 表达式
	GDBIND_SETGET(expression_text);
	GDBIND_SETGET(expression_comment);
	// 变量表达式
	GDBIND_SETGET_BOOL(variable_and_mode);
	GDBIND_SETGET(variable_expression_res_list);
	// ADD_PROPERTY(PropertyInfo(Variant::ARRAY,
	// "variable_expression_res_list"),
	//              "get_variable_expression_res_list",
	//              "set_variable_expression_res_list");

	//  脚本
	GDBIND_SETGET(transition_script);
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

void TransitionRes::set_from_state_res(const Ref<StateRes> &p_from_state_res) {
	from_state_res = p_from_state_res;
	emit_changed();
}
Ref<StateRes> TransitionRes::get_from_state_res() { return from_state_res; }

void TransitionRes::set_to_state_res(const Ref<StateRes> &p_to_state_res) {
	to_state_res = p_to_state_res;
	emit_changed();
}
Ref<StateRes> TransitionRes::get_to_state_res() { return to_state_res; }
void TransitionRes::set_type(int64_t p_type) {
	type = p_type;
	emit_changed();
}
int64_t TransitionRes::get_type() const { return type; }

// Auto
void TransitionRes::set_auto_mode(int64_t p_auto_mode) {
	ERR_FAIL_COND(p_auto_mode < 0 || p_auto_mode >= AUTO_TRANSIT_MODE_MAX);
	auto_mode = p_auto_mode;
	emit_changed();
}
int64_t TransitionRes::get_auto_mode() const { return auto_mode; }
void TransitionRes::set_auto_delay_msec(uint64_t p_delay_time) {
	auto_delay_msec = p_delay_time;
	emit_changed();
}
uint64_t TransitionRes::get_auto_delay_msec() const { return auto_delay_msec; }
void TransitionRes::set_auto_times(uint64_t p_times) {
	auto_times = p_times;
	emit_changed();
}
int64_t TransitionRes::get_auto_times() const { return auto_times; }

// 表达式
void TransitionRes::set_expression_text(const String &p_expression_text) {
	expression_text = p_expression_text;
	emit_changed();
}
String TransitionRes::get_expression_text() const { return expression_text; }
void TransitionRes::set_expression_comment(const String &p_expression_comment) {
	expression_comment = p_expression_comment;
	emit_changed();
}
String TransitionRes::get_expression_comment() const { return expression_comment; }

// 变量表达式
void TransitionRes::set_variable_and_mode(bool p_and_mode) {
	variable_and_mode = p_and_mode;
	emit_changed();
}
bool TransitionRes::is_variable_and_mode() const { return variable_and_mode; }

// void TransitionRes::__on_variable_expression_res_changed(const Ref<VariableExpressionRes> &p_ver) {
// 	ERR_FAIL_COND(p_ver.is_null());
// 	if (_variable_expression_res_list.has(p_ver)) {
// 		emit_changed();
// 	} else {
// 		if (p_ver->TIS_CONNECTED("changed", __on_variable_expression_res_changed)) {
// 			p_ver->TDISCONNECT("changed", __on_variable_expression_res_changed);
// 		}
// 	}
// }

void TransitionRes::set_variable_expression_res_list(const Array &p_variable_expression_res_list) {
	// StringName sn = "changed";
	// auto cb = TCALLABLE(emit_changed);

	// for (auto i = 0; i < _variable_expression_res_list.size(); ++i) {
	// 	Ref<VariableExpressionRes> ver = _variable_expression_res_list[i];
	// 	if (ver.is_valid() && ver->is_connected(sn, cb)) {
	// 		ver->disconnect(sn, cb);
	// 	}
	// }

	variable_expression_res_list = decltype(variable_expression_res_list)(p_variable_expression_res_list);
	// for (size_t i = 0; i < _variable_expression_res_list.size(); i++) {
	// 	Ref<VariableExpressionRes> ver = _variable_expression_res_list[i];
	// 	if (!ver.is_valid()) {
	// 		ver.instantiate();
	// 		_variable_expression_res_list[i] = ver;
	// 	}
	// 	if (!ver->is_connected(sn, cb)) {
	// 		ver->connect(sn, cb);
	// 	}
	// }

	emit_changed();
}
TypedArray<VariableExpressionRes> TransitionRes::get_variable_expression_res_list() const { return variable_expression_res_list; }

#ifdef FULL_VERSION
// 脚本
void TransitionRes::set_transition_script(const Ref<Script> &p_transition_script) {
	transition_script = p_transition_script;
	if (transition_script.is_valid() && Engine::get_singleton()->is_editor_hint()) {
		if (GDScript *s = Object::cast_to<GDScript>(transition_script.ptr())) {
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
}
Ref<Script> TransitionRes::get_transition_script() const { return transition_script; }
#endif

TransitionBase *TransitionRes::create_transition(HFSM *p_hfsm, Ref<StateRes> &r_from_state_res, Ref<StateRes> &r_to_state_res) {
	TransitionBase *r;
	switch (type) {
#ifdef FULL_VERSION
		case TRANSITION_TYPE_SCRIPT: {
			auto t = memnew(Transition);
			t->set_script(transition_script);
			t->hfsm = p_hfsm;
			r = static_cast<TransitionBase *>(t);
		} break;
#endif
		case TRANSITION_TYPE_VARIABLE: {
			auto vt = memnew(VariableTransition);
			vt->and_mode = is_variable_and_mode();
			for (size_t i = 0; i < variable_expression_res_list.size(); i++) {
				Ref<VariableExpressionRes> variable_expression_res = variable_expression_res_list[i];
				auto ve = variable_expression_res->create_variable_expression(p_hfsm);
				ERR_FAIL_COND_V(!ve, nullptr);
				switch (ve->get_expression_type()) {
					case VariableExpression::ExpressionType::NORMAL:
						vt->normal_expressions.append(ve);
						break;
					case VariableExpression::ExpressionType::UNION_TRIGGER:
						vt->union_triggers.append(static_cast<UnionTriggerExpression *>(ve));
						break;
					case VariableExpression::ExpressionType::SOLO_TRIGGER:
						vt->solo_triggers.append(static_cast<SoloTriggerExpression *>(ve));
						break;
					default:
						break;
				}
			}
			r = vt;
		} break;
		case TRANSITION_TYPE_EXPRESSION: {
			auto et = memnew(ExpressionTransition);
			et->hfsm = p_hfsm;
			et->set_expression_text(expression_text);
			r = et;
		} break;
		case TRANSITION_TYPE_AUTO: {
			auto at = memnew(AutoTransition);
			at->mode = auto_mode;
			at->delay_msec = auto_delay_msec;
			at->times = auto_times;
			r = at;
		} break;
		default: {
			if (!Engine::get_singleton()->is_editor_hint()) {
				CRASH_NOW_MSG("Illegal transition type.");
			}
		} break;
	}
	r_from_state_res = get_from_state_res();
	r_to_state_res = get_to_state_res();
	return r;
}

#pragma endregion

}; // namespace Hfsm