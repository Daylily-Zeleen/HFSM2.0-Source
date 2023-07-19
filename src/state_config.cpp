#include "state_config.h"
#include "fsm.h"
#include "fsm_config.h"
#include "hfsm.h"

#ifdef GDEXTENSION_BUILD
#include <godot_cpp/classes/animation_player.hpp>
#include <godot_cpp/classes/gd_script.hpp>
#include <godot_cpp/classes/resource_saver.hpp>

#ifdef MODULE_MONO_ENABLED
#include <godot_cpp/classes/csharp_script.hpp>
#endif // MODULE_MONO_ENABLED

#ifdef TOOLS_ENABLED
#include <godot_cpp/classes/engine.hpp>
#endif // TOOLS_ENABLED

#else // GDEXTENSION_BUILD

#include <modules/gdscript/gdscript.h>
#include <scene/animation/animation_player.h>

#ifdef MODULE_MONO_ENABLED
#include <modules/mono/csharp_script.h>
#endif // MODULE_MONO_ENABLED

#ifdef TOOLS_ENABLED
#include <core/config/engine.h>
#endif // TOOLS_ENABLED

#endif // GDEXTENSION_BUILD

namespace Hfsm {

#define GD_TEMPLATE "extends State\n\n\n"                                       \
					"func _initialize() -> void:\n\tpass\n\n\n"                 \
					"func _entry() -> void:\n\tpass\n\n\n"                      \
					"func _update(delta: float) -> void:\n\tpass\n\n\n"         \
					"func _physics_update(delta: float) -> void:\n\tpass\n\n\n" \
					"func _exit() -> void:\n\tpass\n"

#define CSHARP_TEMPLATE                              \
	"(public partial MyState: Godot.State\n"         \
	"{\n"                                            \
	"	private void _initialize()\n"                  \
	"	{\n"                                           \
	"		// Called after setup internal.\n"            \
	"	}\n\n"                                         \
	"	private void _entry()\n"                       \
	"	{\n"                                           \
	"		// Called when entered this state.\n"         \
	"	}\n\n"                                         \
	"	private void _update(float p_delta)\n"         \
	"	{\n"                                           \
	"		// Called when update this state.\n"          \
	"	}\n\n"                                         \
	"	private void _physics_update(float p_delta)\n" \
	"	{\n"                                           \
	"		// Called when physics update this state.\n"  \
	"	}\n\n"                                         \
	"	private void _exit()\n"                        \
	"	{\n"                                           \
	"		// Called when exit this state.\n"            \
	"	}\n"                                           \
	"}\n\n"

#pragma region StateConfig

PackedStringArray (*StateConfig::get_animation_list)() = nullptr;

bool StateConfig::_set(const StringName &p_name, const Variant &p_property) {
	_TRY_SET_PROP(animation_name);
	IF_FULL_VERSION({
		_TRY_SET_PROP(animation_blend_time);
		_TRY_SET_PROP(animation_speed);
		_TRY_SET_PROP(animation_reverse);
	})
	return false;
}

bool StateConfig::_get(const StringName &p_name, Variant &r_property) const {
	_TRY_GET_PROP(animation_name);
	IF_FULL_VERSION({
		_TRY_GET_PROP(animation_blend_time);
		_TRY_GET_PROP(animation_speed);
		_TRY_GET_PROP(animation_reverse);
	})
	return false;
}
void StateConfig::_get_property_list(List<PropertyInfo> *p_list) const {
	String animations;
	IF_TOOLS(
			if (get_animation_list) {
				PackedStringArray anim_list = get_animation_list();
				for (auto &&anim : anim_list) {
					if (!animations.is_empty()) {
						animations += ",";
					}
					animations += anim;
				}
			})
	_PUSH_PROP(STRING_NAME, animation_name, PROPERTY_HINT_ENUM_SUGGESTION, animations);

	IF_FULL_VERSION({
		_PUSH_PROP(FLOAT, animation_blend_time);
		_PUSH_PROP(FLOAT, animation_speed);
		_PUSH_PROP(BOOL, animation_reverse);
	})
}

void StateConfig::_bind_methods() {
	GDBIND_BEGIN(StateConfig);
	GDADD_PROPERTY_RESOURCE(state_script);

	// Not allow change state name in inspector.
	GDADD_PROPERTY(STRING, state_name, PROPERTY_HINT_NONE, "", PROPERTY_USAGE_STORAGE);
	GDADD_PROPERTY_RESOURCE(fsm_config, PROPERTY_USAGE_STORAGE);
	GDADD_PROPERTY(INT, type, PROPERTY_HINT_ENUM, "Normal,Entry,Exit", PROPERTY_USAGE_STORAGE);
	GDADD_PROPERTY_BOOL(nested, PROPERTY_HINT_NONE, "", PROPERTY_USAGE_STORAGE);

	IF_TOOLS(GDADD_PROPERTY(VECTOR2, editor_offset, PROPERTY_HINT_NONE, "", PROPERTY_USAGE_STORAGE);)

	ADD_GROUP("Animation", "animation_");

#ifdef TOOLS_ENABLED
	GDBIND_METHOD(_set_state_node);
	GDBIND_METHOD(_get_state_node);
#endif
}

void StateConfig::set_state_name(const StringName &p_name) {
	state_name = p_name;
	// 在 StateNode 中检查重复
	emit_changed();
}
StringName StateConfig::get_state_name() const { return state_name; }

void StateConfig::set_type(State::StateType p_state_type) {
	type = p_state_type;
	emit_changed();
}
State::StateType StateConfig::get_type() const { return type; }
void StateConfig::set_state_script(const Ref<Script> &p_script) {
	auto cb = TCALLABLE(set_state_script);
	if (state_script.is_valid() && state_script->is_connected(s_changed, cb)) {
		state_script->disconnect(s_changed, cb);
	}

	state_script = p_script;
	if (state_script.is_null()) {
		script_valid = true;
	} else {
		if (state_script.is_valid() && !state_script->is_connected(s_changed, cb)) {
			state_script->connect(s_changed, TCALLABLE_BIND(set_state_script, state_script));
		}

		bool type_valid = false;

		auto script_base_type = state_script->get_instance_base_type();
		if (script_base_type == StringName(State::get_class_static())) {
			type_valid = true;
		}
		IF_GDM(else {
			type_valid = ClassDB::is_parent_class(script_base_type, State::get_class_static());
		})

#ifdef TOOLS_ENABLED
		if (state_script->get_source_code().is_empty()) {
			if (Engine::get_singleton()->is_editor_hint()) {
				if (auto gds = cast_to<GDScript>(state_script.ptr())) {
					gds->set_source_code(GD_TEMPLATE);
					type_valid = true;
				}
#ifdef MODULE_MONO_ENABLED
				else if (auto csharp = cast_to<CSharpScript>(state_script.ptr())) {
					csharp->set_source_code(CSHARP_TEMPLATE);
					type_valid = true;
				}
#endif // MODULE_MONO_ENABLED
				IF_GDM(
						else {
							auto templates = state_script->get_language()->get_built_in_templates("Object");
							if (templates.size() > 0) {
								auto s = state_script->get_language()->make_template(templates[0].content, "MyState", State::get_class_static());
								if (s->is_valid()) {
									state_script->set_source_code(s->get_source_code());
									type_valid = true;
								}
							}
						})
				if (type_valid && !state_script->get_path().is_empty()) {
					IF_GDE(ResourceSaver::get_singleton()->save(state_script);)
					IF_GDM(ResourceSaver::save(state_script);)

					state_script->reload();
				}
			}
		}
#endif // TOOLS_ENABLED

		if (!type_valid) {
			ED_MSG("HFSM: The Script \"%s\" set to State is not extended from \"%s\".", state_script->get_path(), State::get_class_static());
		}

		script_valid = type_valid;
	}

	call_deferred(SNAME("emit_changed"));
}
Ref<Script> StateConfig::get_state_script() const { return state_script; }
bool StateConfig::is_script_valid() const { return script_valid; }

void StateConfig::set_nested(bool p_nested) {
	nested = p_nested;
	emit_changed();
}
bool StateConfig::is_nested() const { return nested; }

void StateConfig::set_fsm_config(const Ref<FSMConfig> &p_fsm_config) {
	fsm_config = p_fsm_config;
	emit_changed();
}
Ref<FSMConfig> StateConfig::get_fsm_config() const { return fsm_config; }

#ifdef TOOLS_ENABLED
void StateConfig::set_editor_offset(Vector2 p_offset) {
	editor_offset = p_offset;
	emit_changed();
}
Vector2 StateConfig::get_editor_offset() const { return editor_offset; }
#endif // TOOLS_ENABLED

StringName StateConfig::get_animation_name() const { return animation_name; }
void StateConfig::set_animation_name(const StringName &p_anim_name) { animation_name = p_anim_name; }

#ifdef FULL_VERSION
double StateConfig::get_animation_blend_time() const { return animation_blend_time; }
void StateConfig::set_animation_blend_time(double p_blend_time) { animation_blend_time = p_blend_time; }
double StateConfig::get_animation_speed() const { return animation_speed; }
void StateConfig::set_animation_speed(double p_speed) { animation_speed = p_speed; }
bool StateConfig::get_animation_reverse() const { return animation_reverse; }
void StateConfig::set_animation_reverse(bool p_reverse) { animation_reverse = p_reverse; }
#endif

//
Ref<State> StateConfig::create_state(HFSM *p_hfsm, FSM *p_fsm) {
	// 内嵌状态机
	FSM *sub_fsm = nullptr;
	if (fsm_config.is_valid()) {
		sub_fsm = get_fsm_config()->create_fsm(p_hfsm);
	}

	// 脚本处理
	Ref<Script> script_to_set = state_script;
	if (script_to_set.is_valid()) {
		if (script_to_set->get_instance_base_type() == State::get_class_static()) {
			if (script_to_set.is_valid()) {
				auto base_type = script_to_set->get_instance_base_type();

				if (base_type != StringName(State::get_class_static())) {
					IF_GDM(if (!ClassDB::is_parent_class(base_type, State::get_class_static())) {)
					script_to_set.unref();
					IF_GDM( })
				}
			}
		} else {
			String path_text;
			for (auto i = 0; i < p_fsm->get_path().size(); i++) {
				path_text = path_text + Ref<State>(p_fsm->get_path()[i])->get_name();
				if (i != i < p_fsm->get_path().size() - 1) {
					path_text = path_text + String("/");
				}
			}

			ERR_PRINT(path_text + String(": Script is not extends from 'State'."));
		}
	}

	auto ret = memnew(State(state_name, p_hfsm, type, p_fsm->get_path(), script_to_set, sub_fsm, p_fsm->get_fsm_update_queue()));

	ret->set_animation_name(animation_name);
	IF_FULL_VERSION({
		ret->set_animation_speed(animation_speed);
		ret->set_animation_blend_time(animation_blend_time);
		ret->set_animation_reverse(animation_reverse);
	})

	// Call initialized (finally allow to access HFSM).
	ret->initialize_state();

	return ret;
}

#pragma endregion
}; // namespace Hfsm