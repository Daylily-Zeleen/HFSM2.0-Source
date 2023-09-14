/**************************************************************************/
/*  state_config.cpp                                                      */
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

#include "state_config.h"
#include "fsm.h"
#include "fsm_config.h"
#include "hfsm.h"

#ifdef GDEXTENSION_BUILD
#include <godot_cpp/classes/animation_player.hpp>
#include <godot_cpp/classes/gd_script.hpp>
#include <godot_cpp/classes/resource_saver.hpp>

#ifdef MODULE_MONO_ENABLED
#include <godot_cpp/classes/c_sharp_script.hpp>
#endif // MODULE_MONO_ENABLED

#ifdef TOOLS_ENABLED
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/file_access.hpp>
#endif // TOOLS_ENABLED

#else // GDEXTENSION_BUILD

#include <modules/gdscript/gdscript.h>
#include <scene/animation/animation_player.h>

#ifdef MODULE_MONO_ENABLED
#include <core/io/file_access.h>
#include <modules/mono/csharp_script.h>

#endif // MODULE_MONO_ENABLED

#ifdef TOOLS_ENABLED
#include <core/config/engine.h>
#endif // TOOLS_ENABLED

#endif // GDEXTENSION_BUILD

namespace HFSM2 {

#define GD_TEMPLATE                                             \
	"extends State\n\n\n"                                       \
	"func _initialize() -> void:\n\tpass\n\n\n"                 \
	"func _entry() -> void:\n\tpass\n\n\n"                      \
	"func _update(delta: float) -> void:\n\tpass\n\n\n"         \
	"func _physics_update(delta: float) -> void:\n\tpass\n\n\n" \
	"func _exit() -> void:\n\tpass\n"

#ifdef MODULE_MONO_ENABLED
#define CSHARP_TEMPLATE                                                                                \
	"// Because GDExtension has not ability to generate binding for C#, extends form RefCounted here." \
	"public partial class MyState: Godot.RefCounted\n"                                                 \
	"{\n"                                                                                              \
	"	private void _initialize()\n"                                                                    \
	"	{\n"                                                                                             \
	"		// Called after setup internal.\n"                                                              \
	"	}\n\n"                                                                                           \
	"	private void _entry()\n"                                                                         \
	"	{\n"                                                                                             \
	"		// Called when entered this state.\n"                                                           \
	"	}\n\n"                                                                                           \
	"	private void _update(float p_delta)\n"                                                           \
	"	{\n"                                                                                             \
	"		// Called when update this state.\n"                                                            \
	"	}\n\n"                                                                                           \
	"	private void _physics_update(float p_delta)\n"                                                   \
	"	{\n"                                                                                             \
	"		// Called when physics update this state.\n"                                                    \
	"	}\n\n"                                                                                           \
	"	private void _exit()\n"                                                                          \
	"	{\n"                                                                                             \
	"		// Called when exit this state.\n"                                                              \
	"	}\n"                                                                                             \
	"}\n\n"
#endif // MODULE_MONO_ENABLED

// Script verify
bool Utils::is_script_instacne_type_valid(const Ref<Script> &p_script, const StringName &p_class_name, LocalVector<StringName> (*p_get_require_methods)()) {
	if (p_script.is_null()) {
		return false;
	}

	// (GDE) Check C# script on disk or not.
	IF_GDE(IF_MONO(IF_TOOLS({
		// (C# is not support builtin mode).
		if (auto csharp_script = Object::cast_to<CSharpScript>(p_script.ptr())) {
			ERR_FAIL_COND_V_MSG(!FileAccess::file_exists(csharp_script->get_path()), false, vformat("HFSM: The CSharp script \"%s\" is not support builtin mode and not present on disk.", csharp_script->get_path()));
		}
	})))

	// (GDM) Check script which unsupport builtin mode.
	IF_GDM({
		if (!p_script->get_language()->supports_builtin_mode()) {
			ERR_FAIL_COND_V_MSG(!FileAccess::exists(p_script->get_path()), false, vformat("HFSM: The script \"%s\" is not support builtin mode and not present on disk.", p_script->get_path()));
		}
	})

	// Strictly type match.
	if (p_script->get_instance_base_type() == State::get_class_static()) {
		return true;
	}
	IF_GDM(else {
		if (ClassDB::is_parent_class(script_base_type, State::get_class_static())) {
			return true;
		}
	})

	// // GDScript require type correct.
	// bool gds_instance_base_type_valid = Object::cast_to<GDScript>(p_script.ptr());
	// ERR_FAIL_COND_V(!gds_instance_base_type_valid, false);

	// Type not match, check virtual methods only.
	// TODO:: Because of the limitation of GDExtension, here using a stupid way to check methods.
	// 		Waiting for the ClassDB singleton/static methods.
	auto script_methods = p_script->get_script_method_list();
	auto find_method_info = [script_methods](const StringName &p_method_name) {
		for (auto i = 0; i < script_methods.size(); ++i) {
			Dictionary m = script_methods[i];
			if (StringName(m["name"]) == p_method_name) {
				return m;
			}
		}
		return Dictionary();
	};

	String suffix = "";
	IF_MONO(if (auto csharp_script = Object::cast_to<CSharpScript>(p_script.ptr())) {
		String suffix = " please fix it and rebuild project.";
	})
	for (const StringName &method_name : p_get_require_methods()) {
		if (!p_script->has_method(method_name)) {
			continue;
		}
		const auto mi = ClassDB::get_method(p_class_name, method_name);
		ERR_FAIL_COND_V(!mi, false); // Should never happen.
		Dictionary m = find_method_info(method_name);
		ERR_FAIL_COND_V(m.is_empty(), false);

		TypedArray<Dictionary> args = m["args"];
		// Check count.
		ERR_FAIL_COND_V_MSG(args.size() != mi->get_argument_count(), false, vformat("The method \"%s\" of script \"%s\" argument count is require %d.", method_name, p_script->get_path(), mi->get_argument_count()) + suffix);
		// Check types.
		for (auto i = 0; i < mi->get_argument_count(); ++i) {
			Dictionary arg = args[i];
			const auto arg_info = mi->get_argument_info(i);
			ERR_FAIL_COND_V_MSG(int(arg["type"]) != arg_info.type, false, vformat("The method \"%s\" argument %d of script \"%s\" is require %s.", method_name, i + 1, p_script->get_path(), Variant::get_type_name(arg_info.type)) + suffix);
		}
	}

	return true;
}

#ifdef TOOLS_ENABLED
#ifdef MODULE_MONO_ENABLED
void Utils::set_template_if_source_code_is_empty(const Ref<Script> &p_script, const char *p_gds_template, const char *p_charp_template)
#else // MODULE_MONO_ENABLED
void Utils::set_template_if_source_code_is_empty(const Ref<Script> &p_script, const char *p_gds_template)
#endif // MODULE_MONO_ENABLED
{
	if (!p_script->get_source_code().is_empty()) {
		return;
	}

	IF_MONO({
		// CSharp
		if (auto csharp = Object::cast_to<CSharpScript>(p_script.ptr())) {
			bool (*file_existed)(const String &);
			IF_GDE(file_existed = &FileAccess::file_exists);
			IF_GDM(file_existed = &FileAccess::exists);
			if (file_existed(csharp->get_path())) {
				// Only print a warning if is a valid C sharp script.
				WARN_PRINT(vformat("The CSharpScript \"%s\" is empty, or you need rebuild CSharp project.", csharp->get_path()));
				WARN_PRINT("You can use script template:");
				WARN_PRINT(p_charp_template);
				return;
			}
		}
	})

	// GDScript
	if (auto gds = Object::cast_to<GDScript>(p_script.ptr())) {
		gds->set_source_code(p_gds_template);
	}

	IF_GDM(else {
		// Others
		auto templates = p_script->get_language()->get_built_in_templates(Object::get_class_static());
		if (templates.size() > 0) {
			auto s = p_script->get_language()->make_template(templates[0].content, "MyState", State::get_class_static());
			if (s->is_valid()) {
				p_script->set_source_code(s->get_source_code());
			}
		}
	})

	// Try save and reload.
	if (!p_script->get_path().is_empty()) {
		IF_GDE(ResourceSaver::get_singleton()->save(p_script);)
		IF_GDM(ResourceSaver::save(p_script);)
		p_script->reload();
	}
}
#endif // TOOLS_ENABLED

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
	GDADD_PROPERTY_RESOURCE(sub_fsm_config, PROPERTY_USAGE_STORAGE);
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

		IF_TOOLS({
			IF_MONO(Utils::set_template_if_source_code_is_empty(state_script, GD_TEMPLATE, CSHARP_TEMPLATE);)
			IF_NOT_MONO(Utils::set_template_if_source_code_is_empty(state_script, GD_TEMPLATE);)
		})

		script_valid = Utils::is_script_instacne_type_valid(state_script, State::get_class_static(), [] {static const LocalVector<StringName> methods =  { "_initialize", "_entry", "_update", "_physics_update", "_exit" };return methods; });

		if (!script_valid) {
			ED_MSG("HFSM: The Script \"%s\" set to State is invalid (not extended from \"%s\" (GDScript) or have illegal methods).", state_script->get_path(), State::get_class_static());
		}
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

void StateConfig::set_sub_fsm_config(const Ref<FSMConfig> &p_fsm_config) {
	sub_fsm_config = p_fsm_config;
	emit_changed();
}
Ref<FSMConfig> StateConfig::get_sub_fsm_config() const { return sub_fsm_config; }

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
	if (sub_fsm_config.is_valid()) {
		sub_fsm = get_sub_fsm_config()->create_fsm(p_hfsm);
	}

	auto ret = memnew(State(state_name, p_hfsm, type, p_fsm->get_path(), sub_fsm, p_fsm->get_fsm_update_queue()));

	if (unlikely(!script_valid)) {
		WARN_PRINT(vformat("\"%s\" is not a valid script for State, will create a State without script.", state_script->get_path()));
	} else {
		ret->set_script(state_script);
	}

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
}; // namespace HFSM2
