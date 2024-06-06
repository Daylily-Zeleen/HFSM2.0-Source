/**************************************************************************/
/*  state_config.h                                                        */
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

#pragma once

#ifdef GDEXTENSION_BUILD
#include <godot_cpp/classes/ref.hpp>
#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/classes/script.hpp>

using namespace godot;
#else // GDEXTENSION_BUILD
#include <core/io/resource.h>
#include <core/object/script_language.h>
#endif // GDEXTENSION_BUILD

#include "state.h"

namespace HFSM2 {

class HFSM;
class FSM;
class State;
class FSMConfig;

class Utils {
	Utils() = default;

public:
	static bool is_script_instacne_type_valid(const Ref<Script> &p_script, const StringName &p_class_name, LocalVector<StringName> (*p_get_require_methods)());

#ifdef TOOLS_ENABLED
#ifdef MODULE_MONO_ENABLED
	static void set_template_if_source_code_is_empty(const Ref<Script> &p_script, const char *p_gds_template, const char *p_charp_template);
#else // MODULE_MONO_ENABLED
	static void set_template_if_source_code_is_empty(const Ref<Script> &p_script, const char *p_gds_template);
#endif // MODULE_MONO_ENABLED
#endif // TOOLS_ENABLED
};

class StateConfig : public Resource {
	GDCLASS(StateConfig, Resource)

#ifdef TOOLS_ENABLED
public:
	void _set_state_node(Node *p_state_nde) { state_node = p_state_nde; }
	Node *_get_state_node() const { return state_node; }

	void set_state_node(Node *p_state_nde) { state_node = p_state_nde; }
	Node *get_state_node() const { return state_node; }

	Array debug_serialize() const;
	static Ref<StateConfig> debug_deserialize(const Array &p_data);

private:
	Node *state_node = nullptr;

#endif // TOOLS_ENABLED

protected:
	static void _bind_methods();

	_TO_STRING()

public:
	static PackedStringArray (*get_animation_list)();

	bool _set(const StringName &p_name, const Variant &p_property);
	bool _get(const StringName &p_name, Variant &r_property) const;
	void _get_property_list(List<PropertyInfo> *p_list) const;

	void set_state_name(const StringName &p_name);
	StringName get_state_name() const;
	void set_type(State::StateType p_state_type);
	State::StateType get_type() const;
	void set_state_script(const Ref<Script> &p_script);
	Ref<Script> get_state_script() const;
	bool is_script_valid() const;
	void set_nested(bool p_nested);
	bool is_nested() const;
	void set_sub_fsm_config(const Ref<FSMConfig> &p_fsm_config);
	Ref<FSMConfig> get_sub_fsm_config() const;

#ifdef TOOLS_ENABLED
	void set_editor_offset(Vector2 p_offset);
	Vector2 get_editor_offset() const;
#endif // TOOLS_ENABLED

	// 新特性 动画状态机
	StringName get_animation_name() const;
	void set_animation_name(const StringName &p_anim_name);

#ifdef FULL_VERSION
	double get_animation_blend_time() const;
	void set_animation_blend_time(double p_blend_time);
	double get_animation_speed() const;
	void set_animation_speed(double p_speed);
	bool get_animation_reverse() const;
	void set_animation_reverse(bool p_reverse);
#endif
	Ref<State> create_state(HFSM *p_hfsm, FSM *p_fsm);

private:
	Ref<Script> state_script;
	bool script_valid = true;

	// 子状态机
	Ref<FSMConfig> sub_fsm_config;
#ifdef TOOLS_ENABLED
	Vector2 editor_offset;
#endif //TOOLS_ENABLED
	StringName state_name = "state";
	State::StateType type = State::STATE_TYPE_NORMAL;
	bool nested = false;

	// 新特性：动画状态机
	StringName animation_name = {};
#ifdef FULL_VERSION
	double animation_blend_time = 0.0f;
	double animation_speed = 1.0f;
	bool animation_reverse = false;
#endif
};

}; // namespace HFSM2
