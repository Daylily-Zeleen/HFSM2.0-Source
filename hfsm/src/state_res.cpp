#include "state_res.h"
#include "fsm.h"
#include "fsm_res.h"
#include "hfsm.h"

#ifdef TOOLS_ENABLED
#include "../editor/hfsm_editor.h"
#include "../editor/hfsm_editor_plugin.h"
#endif // TOOLS_ENABLED

#ifdef GDEXTENSION_BUILD
#include <godot_cpp/classes/animation_player.hpp>
#ifdef MODULE_MONO_ENABLED
#include <godot_cpp/classes/csharp_script.hpp>
#endif // MODULE_MONO_ENABLED
#include <godot_cpp/classes/gdscript.hpp>

#else
#include <modules/gdscript/gdscript.h>
#include <modules/mono/csharp_script.h>
#include <scene/animation/animation_player.h>

#endif // GDEXTENSION_BUILD

namespace Hfsm {

#pragma region StateRes

bool StateRes::_set(const StringName &p_name, const Variant &p_property) {
	_TRY_SET_PROP(animation_name);
	IF_FULL_VERSION({
		_TRY_SET_PROP(animation_blend_time);
		_TRY_SET_PROP(animation_speed);
		_TRY_SET_PROP(animation_reverse);
	})
	return false;
}

bool StateRes::_get(const StringName &p_name, Variant &r_property) const {
	_TRY_GET_PROP(animation_name);
	IF_FULL_VERSION({
		_TRY_GET_PROP(animation_blend_time);
		_TRY_GET_PROP(animation_speed);
		_TRY_GET_PROP(animation_reverse);
	})
	return false;
}
void StateRes::_get_property_list(List<PropertyInfo> *p_list) const {
	String animations;
	IF_TOOLS(
			if (HfsmEditorPlugin::get_singleton()) {
				if (HfsmEditorPlugin::get_singleton()->get_hfsm_editor()) {
					if (HfsmEditorPlugin::get_singleton()->get_hfsm_editor()->get_editing_hfsm()) {
						if (HfsmEditorPlugin::get_singleton()->get_hfsm_editor()->get_editing_hfsm()->get_animation_player()) {
							PackedStringArray anim_list = HfsmEditorPlugin::get_singleton()->get_hfsm_editor()->get_editing_hfsm()->get_animation_player()->call("get_animation_list");
							for (auto &&anim : anim_list) {
								animations += "," + anim;
							}
						}
					}
				}
			})
	_PUSH_PROP(STRING_NAME, animation_name, PROPERTY_HINT_ENUM_SUGGESTION, animations);

	IF_FULL_VERSION({
		_PUSH_PROP(FLOAT, animation_blend_time);
		_PUSH_PROP(FLOAT, animation_speed);
		_PUSH_PROP(BOOL, animation_reverse);
	})
}

void StateRes::_bind_methods() {
	GDBIND_BEGIN(StateRes);
	GDADD_PROPERTY_RESOURCE(state_script);

	// Not allow change state name in inspector.
	GDADD_PROPERTY(STRING, state_name, PROPERTY_HINT_NONE, "", PROPERTY_USAGE_STORAGE);
	GDADD_PROPERTY_RESOURCE(fsm_res, PROPERTY_USAGE_STORAGE);
	GDADD_PROPERTY(INT, type, PROPERTY_HINT_ENUM, "Normal,Entry,Exit", PROPERTY_USAGE_STORAGE);
	GDADD_PROPERTY_BOOL(nested, PROPERTY_HINT_NONE, "", PROPERTY_USAGE_STORAGE);
	GDADD_PROPERTY(VECTOR2, editor_offset, PROPERTY_HINT_NONE, "", PROPERTY_USAGE_STORAGE);
	GDADD_PROPERTY(VECTOR2, size_in_editor, PROPERTY_HINT_NONE, "", PROPERTY_USAGE_STORAGE);

	ADD_GROUP("Animation", "animation_");

#ifdef TOOLS_ENABLED
	GDBIND_SETGET(state_node);
#endif
}

void StateRes::set_state_name(const StringName &p_name) {
	state_name = p_name;
	// 在 StateNode 中检查重复
	emit_changed();
}
StringName StateRes::get_state_name() const { return state_name; }

void StateRes::set_type(State::StateType p_state_type) {
	type = p_state_type;
	emit_changed();
}
State::StateType StateRes::get_type() const { return type; }
void StateRes::set_state_script(const Ref<Script> &p_script) {
	state_script = p_script;
	if (state_script.is_valid()) {
		bool type_valid = false;
		if (state_script->can_instantiate()) {
			auto script_base_type = state_script->get_instance_base_type();
			IF_GDE(if (script_base_type == State::get_class_static()) {
				type_valid = true;
			})
			IF_GDM(if (script_base_type == State::get_class_static() || ClassDB::is_parent_class(State::get_class_static(), script_base_type)) {
				type_valid = true;
			})
		} else {
			if (!state_script->get_source_code().is_empty()) {
				if (auto gds = cast_to<GDScript>(state_script.ptr())) {
					gds->set_source_code(
							"extends State\n\n"
							"func _initialize() -> void:\n\tpass\n\n"
							"func _entry() -> void:\n\tpass\n\n"
							"func _update(delta: float) -> void:\n\tpass\n\n"
							"func _physics_update(delta: float) -> void:\n\tpass\n\n"
							"func _exit() -> void:\n\tpass\n");
					type_valid = true;
				}
#if defined(GDEXTENSION_BUILD) || defined(MODULE_MONO_ENABLED)
				else if (auto csharp = cast_to<CSharpScript>(state_script.ptr())) {
					csharp->set_source_code(
							R"XXX(public partial MyState: Godot.State
{
	private void _initialize()
	{
		// Called after setup internal.
	}

	private void _entry()
	{
		// Called when entered this state.
	}

	private void _update(float p_delta)
	{
		// Called when update this state.
	}

	private void _physics_update(float p_delta)
	{
		// Called when physics update this state.
	}

	private void _exit()
	{
		// Called when exit this state.
	}
}

)XXX");
					type_valid = true;
				}
#endif // defined(GDEXTENSION_BUILD) || defined(MODULE_MONO_ENABLED)
				IF_GDM(
						else {
							auto templates = state_script->get_language()->get_built_in_templates("Object");
							if (templates.size() > 0) {
								state_script->set_source_code(state_script->get_language()->make_template(templates[0].content, "MyState", "State")->get_source_code());

								type_valid = true;
							}
						})
				if (type_valid) {
					ResourceSaver::save(state_script);
					state_script->reload();
				}
			}
		}
		if (!type_valid) {
			// todo 类型错误警告
		}
	}

	emit_changed();
}
Ref<Script> StateRes::get_state_script() const { return state_script; }

void StateRes::set_nested(bool p_nested) {
	nested = p_nested;
	emit_changed();
}
bool StateRes::is_nested() const { return nested; }

void StateRes::set_fsm_res(const Ref<FsmRes> &p_fsm_res) {
	fsm_res = p_fsm_res;
	emit_changed();
}
Ref<FsmRes> StateRes::get_fsm_res() const { return fsm_res; }

void StateRes::set_editor_offset(Vector2 p_offset) {
	editor_offset = p_offset;
	emit_changed();
}
Vector2 StateRes::get_editor_offset() const { return editor_offset; }

StringName StateRes::get_animation_name() const { return animation_name; }
void StateRes::set_animation_name(const StringName &p_anim_name) { animation_name = p_anim_name; }

#ifdef FULL_VERSION
double StateRes::get_animation_blend_time() const { return animation_blend_time; }
void StateRes::set_animation_blend_time(double p_blend_time) { animation_blend_time = p_blend_time; }
double StateRes::get_animation_speed() const { return animation_speed; }
void StateRes::set_animation_speed(double p_speed) { animation_speed = p_speed; }
bool StateRes::get_animation_reverse() const { return animation_reverse; }
void StateRes::set_animation_reverse(bool p_reverse) { animation_reverse = p_reverse; }
#endif

//
Ref<State> StateRes::create_state(HFSM *p_hfsm, Fsm *p_fsm) {
	Ref<State> r;
	r.instantiate();
	r->set_name(state_name);
	r->hfsm = p_hfsm;
	r->type = type;

	r->set_animation_name(animation_name);
	IF_FULL_VERSION({
		r->animation_speed = animation_speed;
		r->animation_blend_time = animation_blend_time;
		r->animation_reverse = animation_reverse;
	})
	// 路径
	// State 一定包含于 Fsm
	r->path.append_array(p_fsm->get_path());

	// collect properties
	// if (r->reset_when_entry) {
	// 	static const StringName mn = "get_property_list";
	// 	TypedArray<Dictionary> properties = r->call(mn);
	// 	for (auto i = 0; i < properties.size(); i++) {
	// 		Dictionary prop = static_cast<Dictionary>(properties[i]);
	// 		if (IS_PROP(prop["usage"])) {
	// 			String prop_name = prop["name"];
	// 			if (HfsmGlobal::get_state_internal_property_names().has(prop_name)) {
	// 				continue;
	// 			}
	// 			r->property_to_defatul_value.insert(prop_name, r->get(prop_name));
	// 		}
	// 	}
	// }

	// 脚本处理
	if (state_script.is_valid()) {
		if (state_script->get_instance_base_type() == State::get_class_static()) {
			if (state_script.is_valid()) {
				auto base_type = state_script->get_instance_base_type();
				IF_GDM(if (ClassDB::is_parent_class(State::get_class_static(), base_type)) {
					r->set_script(state_script);
				} else)

				if (base_type != StringName(State::get_class_static())) {
					r->set_script(state_script);
				}
			}
		} else {
			String path_text;
			for (auto i = 0; i < r->get_path().size(); i++) {
				path_text = path_text + Ref<State>(r->get_path()[i])->get_name();
				if (i != i < r->get_path().size() - 1) {
					path_text = path_text + String("/");
				}
			}
			ERR_FAIL_V_MSG(r, path_text + String(": Script is not extends from 'State'."));
		}
	}
	// 内嵌状态机
	if (fsm_res.is_valid()) {
		r->sub_fsm = static_cast<FsmRes *>(get_fsm_res().ptr())->create_fsm(p_hfsm, r, p_fsm->get_fsm_update_queue());
	}

	// Call initialized (finally allow to access HFSM).
	r->initialize_state();

	return r;
}

#pragma endregion

}; // namespace Hfsm