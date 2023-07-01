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

namespace Hfsm {

class HFSM;
class Fsm;
class State;
// class FsmRes;

class StateRes : public Resource {
	GDCLASS(StateRes, Resource)

#ifdef TOOLS_ENABLED
public:
	void set_state_node(Node *p_state_nde) { state_node = p_state_nde; }
	Node *get_state_node() const { return state_node; }

private:
	Node *state_node = nullptr;
#endif //TOOLS_ENABLED

protected:
	static void _bind_methods();

	_TO_STRING()

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

	void set_state_name(const StringName &p_name);
	StringName get_state_name() const;
	void set_type(State::StateType p_state_type);
	State::StateType get_type() const;
	void set_state_script(const Ref<Script> &p_script);
	Ref<Script> get_state_script() const;
	void set_nested(bool p_nested);
	bool is_nested() const;
	void set_fsm_res(const Ref<class FsmRes> &p_fsm_res);
	Ref<class FsmRes> get_fsm_res() const;

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
	Ref<State> create_state(HFSM *p_hfsm, Fsm *p_fsm);

private:
	Ref<Script> state_script;
	// 避免循环依赖
	Ref<class FsmRes> fsm_res;
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

}; // namespace Hfsm