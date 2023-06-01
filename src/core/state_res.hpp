#pragma once

#include <godot_cpp/classes/Resource.hpp>
#include <godot_cpp/classes/ref.hpp>
#include <godot_cpp/classes/script.hpp>

#include <godot_cpp/core/binder_common.hpp>

#include "state.hpp"

using namespace godot;

namespace Hfsm {

class HFSM;
class Fsm;
class State;
class FsmRes;

class StateRes : public Resource {
	GDCLASS(StateRes, Resource)

protected:
	static void _bind_methods();
	String _to_string() const { return vformat("[StateRes:%d]", get_instance_id()); }

public:
	bool _set(const StringName &p_name, const Variant &p_property);
	bool _get(const StringName &p_name, Variant &r_property) const;
	void _get_property_list(List<PropertyInfo> *p_list) const;

	// enum StateType {
	//     STATE_TYPE_NORMAL,
	//     STATE_TYPE_ENTRY,
	//     STATE_TYPE_EXIT,
	//     STATE_TYPE_MAX,
	// };

	void set_state_node(godot::Node *);
	godot::Node *get_state_node() const;
	void set_state_name(const StringName &p_name);
	StringName get_state_name() const;
	void set_type(State::StateType p_state_type);
	State::StateType get_type() const;
	void set_state_script(const Ref<Script> &p_script);
	Ref<Script> get_state_script() const;
	void set_nested(bool p_nested);
	bool is_nested() const;
	void set_fsm_res(const Ref<FsmRes> &p_fsm_res);
	Ref<FsmRes> get_fsm_res() const;
	void set_editor_offset(Vector2 p_offset);
	Vector2 get_editor_offset() const;
	void set_reset_properties_when_entry(bool p_v);
	bool is_reset_properties_when_entry() const;
	void set_reset_nested_fsm_when_entry(bool p_v);
	bool is_reset_nested_fsm_when_entry() const;

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

	Ref<RefCounted> create_state(HFSM *p_hfsm, const Fsm *p_fsm);

	Vector2 get_size_in_editor() { return size_in_editor; }
	void set_size_in_editor(Vector2 p_new_size) { size_in_editor = p_new_size; }

private:
	Ref<Script> state_script;
	// 避免循环依赖
	Ref<FsmRes> fsm_res;
	Vector2 size_in_editor;
	Vector2 editor_offset;
	StringName state_name = "state";
	State::StateType type = State::STATE_TYPE_NORMAL;
	bool nested = false;
	bool reset_properties_when_entry = true;
	bool reset_nested_fsm_when_entry = false;

	// 新特性：动画状态机
	StringName animation_name = {};
#ifdef FULL_VERSION
	double animation_blend_time = 0.0f;
	double animation_speed = 1.0f;
	bool animation_reverse = false;
#endif

#ifdef TOOL_ENABLED
	Node *state_node = nullptr;
#endif
};

}; // namespace Hfsm