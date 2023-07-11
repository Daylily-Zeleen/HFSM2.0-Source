#include "transition_config.h"
#include "state_config.h"
#include "transitions/auto_transition.h"
#include "transitions/expression_transition.h"
#include "transitions/transition.h"
#include "transitions/variable_expressions/trigger_expression.h"
#include "transitions/variable_expressions/variable_expression.h"
#include "transitions/variable_expressions/variable_expression_config.h"
#include "transitions/variable_transition.h"

#ifdef GDEXTENSION_BUILD

#include <godot_cpp/classes/gd_script.hpp>
#include <godot_cpp/classes/resource_saver.hpp>
#include <godot_cpp/classes/translation_server.hpp>

#ifdef MODULE_MONO_ENABLED
#include <godot_cpp/classes/csharp_script.h>
#endif // MODULE_MONO_ENABLED

#else // GDEXTENSION_BUILD
#include <core/string/translation.h>
#include <modules/gdscript/gdscript.h>

#ifdef MODULE_MONO_ENABLED
#include <modules/mono/csharp_script.h>
#endif // MODULE_MONO_ENABLED

#endif //GDEXTENSION_BUILD

namespace Hfsm {

#pragma region TransitionConfig

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
		IF_FULL_VERSION(
				case TRANSITION_TYPE_SCRIPT
				: {
					_PUSH_PROP_RESOURCE(transition_script);
				} break;)
		case TRANSITION_TYPE_VARIABLE_EXPRESSIONS: {
			_PUSH_PROP(BOOL, and_mode);
			_PUSH_PROP_TYPED_ARRAY(variable_expression_config_list, VariableExpressionConfig);
		} break;
		case TRANSITION_TYPE_EXPRESSION: {
			_PUSH_PROP(STRING, expression_text, PROPERTY_HINT_MULTILINE_TEXT);
			_PUSH_PROP(STRING, expression_comment, PROPERTY_HINT_MULTILINE_TEXT);
		} break;
		case TRANSITION_TYPE_AUTO: {
			_PUSH_PROP(INT, auto_mode, PROPERTY_HINT_ENUM, "Animation Finish, Delay Timer,Fsm Exit,Manual,Update Times,Physics Update Times");
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

void TransitionConfig::_bind_methods() {
	GDBIND_BEGIN(TransitionConfig);
	// 通用
	GDADD_PROPERTY_RESOURCE(from_state_config, PROPERTY_USAGE_STORAGE);
	GDADD_PROPERTY_RESOURCE(to_state_config, PROPERTY_USAGE_STORAGE);
	GDADD_PROPERTY(INT, type, PROPERTY_HINT_ENUM, "Script,Variable,Expression,Auto");

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
	// ADD_PROPERTY(PropertyInfo(Variant::ARRAY,
	// "variable_expression_config_list"),
	//              "get_variable_expression_config_list",
	//              "set_variable_expression_config_list");

	//  脚本
	GDBIND_SETGET(transition_script);

	// ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "transition_script",
	// PROPERTY_HINT_RESOURCE_TYPE, "Script"),
	//              "set_transition_script", "get_transition_script");

	// ClassDB::bind_method(D_METHOD("get_valid_and_texts"),
	//                      &TransitionConfig::get_valid_and_texts);
	// 枚举
	BIND_CONSTANT(TRANSITION_TYPE_SCRIPT);
	BIND_CONSTANT(TRANSITION_TYPE_VARIABLE_EXPRESSIONS);
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

		bool type_valid = false;

		auto base = transition_script->get_instance_base_type();
		if (base == StringName(Transition::get_class_static())) {
			type_valid = true;
		}
		IF_GDM(else {
			type_valid = ClassDB::is_parent_class(base, Transition::get_class_static());
		})

#ifdef TOOLS_ENABLED
		if (transition_script->get_source_code().is_empty()) {
			if (Engine::get_singleton()->is_editor_hint()) {
				if (GDScript *s = Object::cast_to<GDScript>(transition_script.ptr())) {
					s->set_source_code(R"XXX(extends Transition

## <summary>
## Will be called every time when the HFSM update( or physics update)
## Your must to overried this method to determine whether transit to the to state or not.
## </summary>
## <returns> Can transit or not.</returns>
func _can_transit() -> bool:
	# Your check logic.
	return false;


## <summary>
## Will be called every time when the HFSM entry the from state.
## </summary>
func _refresh() -> void:
	pass

)XXX");
					type_valid = true;
				}
#ifdef MODULE_MONO_ENABLED
				else if (auto csharp = cast_to<CSharpScript>(transition_script.ptr())) {
					s->set_source_code(R"XXX(public partial class MyTransition: Godot.Transition
{
	// <summary>
	// Will be called every time when the HFSM update( or physics update)
	// Your must to overried this method to determine whether transit to the to state or not.
	// </summary>
	// <returns> Can transit or not.</returns>
	private bool _can_transit()
	{
		return false;
	}
	
	// <summary>
	// Will be called every time when the HFSM entry the from state.
	// </summary>
	private void _refresh()
	{
	}
}

)XXX");
					type_valid = true;
				}
#endif // MODULE_MONO_ENABLED
			}

			IF_GDM({
				auto lang = transition_script->get_language();
				auto templates = lang->get_built_in_templates("Object");
				if (templates.size() > 0) {
					auto s = lang->make_template(templates[0].content, "MyTransition", Transition::get_class_static());
					if (s->is_valid()) {
						transition_script->set_source_code(s->get_source_code());
						type_valid = true;
					}
				}
			})

			if (type_valid && !transition_script->get_path().is_empty()) {
				IF_GDE(ResourceSaver::get_singleton()->save(transition_script);)
				IF_GDM(ResourceSaver::save(transition_script);)
				transition_script->reload();
			}
		}
#endif // TOOLS_ENABLED

		if (!type_valid) {
			ED_MSG("HFSM: The Script \"%s\" set to Transition is not extended from \"%s\".", transition_script->get_path(), Transition::get_class_static());
		}

		script_valid = type_valid;
	}
	emit_changed();
}
Ref<Script> TransitionConfig::get_transition_script() const { return transition_script; }
bool TransitionConfig::is_script_valid() const { return script_valid; }

#endif

TransitionBase *TransitionConfig::create_transition(HFSM *p_hfsm, Ref<StateConfig> &r_from_state_config, Ref<StateConfig> &r_to_state_config) {
	TransitionBase *ret;
	switch (type) {
		IF_FULL_VERSION(
				case TRANSITION_TYPE_SCRIPT
				: {
					auto t = memnew(Transition);
					t->set_script(transition_script);
					t->hfsm = p_hfsm;
					ret = static_cast<TransitionBase *>(t);
				} break;)
		case TRANSITION_TYPE_VARIABLE_EXPRESSIONS: {
			auto vt = memnew(VariableTransition);
			vt->and_mode = is_and_mode();
			for (size_t i = 0; i < variable_expression_config_list.size(); i++) {
				Ref<VariableExpressionConfig> variable_expression_config = variable_expression_config_list[i];
				auto ve = variable_expression_config->create_variable_expression(p_hfsm);
				ERR_FAIL_COND_V(!ve, nullptr);
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
		case TRANSITION_TYPE_EXPRESSION: {
			auto et = memnew(ExpressionTransition);
			et->hfsm = p_hfsm;
			et->set_expression_text(expression_text);
			ret = et;
		} break;
		case TRANSITION_TYPE_AUTO: {
			auto at = memnew(AutoTransition);
			at->mode = auto_mode;
			at->delay_msec = auto_delay_msec;
			at->times = auto_times;
			ret = at;
		} break;
		default: {
			if (!Engine::get_singleton()->is_editor_hint()) {
				CRASH_NOW_MSG("Illegal transition type.");
			}
		} break;
	}
	r_from_state_config = get_from_state_config();
	r_to_state_config = get_to_state_config();
	return ret;
}

#pragma endregion

}; // namespace Hfsm