#pragma once

#include "../hfsm_global.h"

#ifdef GDEXTENSION_BUILD
#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/templates/local_vector.hpp>
#include <godot_cpp/variant/typed_array.hpp>

using namespace godot;

namespace godot {
class Script;
};
#else
#include <core/object/ref_counted.h>
#include <core/variant/typed_array.h>

#include <core/object/script_language.h>
#include <core/object/gdvirtual.gen.inc>

class Script;
#endif // GDEXTENSION_BUILD

namespace Hfsm {

class FSM;
class HFSM;
class TransitionBase;

class State : public RefCounted {
	GDCLASS(State, RefCounted)

protected:
	static void _bind_methods();
	_TO_STRING()

public:
	enum StateType {
		STATE_TYPE_NORMAL,
		STATE_TYPE_ENTRY,
		STATE_TYPE_EXIT,
		STATE_TYPE_MAX,
	};

	StringName get_name();

	HFSM *get_hfsm();
	bool is_exited();

	void manual_exit();

	GDVIRTUAL0(_initialize);
	GDVIRTUAL0(_entry);
	GDVIRTUAL1(_update, real_t);
	GDVIRTUAL1(_physics_update, real_t);
	GDVIRTUAL0(_exit);

	virtual void initialize_state(); // Will be caled after setup internal properties.
	virtual void entry_state();
	virtual void update_state(real_t p_delta);
	virtual void physics_update_state(real_t p_delta);
	virtual void exit_state();

	const TypedArray<State> &get_path() const;

	StateType get_type() const;

	FSM *get_sub_fsm();

	// 新特性 动画状态机
	StringName get_animation_name_for_playing() const;

	StringName get_animation_name() const;
	void set_animation_name(const StringName &p_anim_name);
	bool is_animation_playing() const;
#ifdef FULL_VERSION
	double get_animation_blend_time() const;
	void set_animation_blend_time(double p_blend_time);
	double get_animation_speed() const;
	void set_animation_speed(double p_speed);
	bool is_animation_reverse() const;
	void set_animation_reverse(bool p_reverse);
#endif

#ifdef ROLLBACK_NET_CODE
	Array save_state();
	void load_state(const Array &state);
	void interpolate_state(const Array &old_state, const Array &new_state, real_t weight);
	Array get_local_input();
	Array predict_remote_input(const Array &previous_input, int64_t ticks_since_real_input);
	void network_process(Array &input);
	void network_preprocess(Array &input);
	void network_postprocess(Array &input);
	Dictionary &network_spawn_preprocess(Dictionary &data);
	void network_spawn(Dictionary &data);
	void network_despawn();

	virtual Variant _save_state();
	virtual void _load_state(const Variant &state);
	virtual void _interpolate_state(const Variant &old_state, const Variant &new_state, real_t weight);
	virtual Variant _get_local_input();
	virtual Variant _predict_remote_input(const Variant &previous_input, int64_t ticks_since_real_input);
	virtual void _network_process(Dictionary &input);
	virtual void _network_preprocess(Dictionary &input);
	virtual void _network_postprocess(Dictionary &input);
	virtual Dictionary &_network_spawn_preprocess(Dictionary &data);
	virtual void _network_spawn(Dictionary &data);
	virtual void _network_despawn();
#endif

	State() = default;
	State(const StringName &p_name, HFSM *p_hfsm, StateType p_type, const TypedArray<Hfsm::State> &p_path, const Ref<Script> &p_script, FSM *p_sub_fsm, const LocalVector<FSM *> &p_nested_fsm_update_queue);

	~State() override;

private:
	Vector<TransitionBase *> transition_list;
	// 子状态机
	FSM *sub_fsm = nullptr;

	StringName name = "";
	HFSM *hfsm = nullptr;

	TypedArray<State> path;
	bool exited = false;

	StateType type = STATE_TYPE_NORMAL;

	// 新特性：动画状态机
	bool animation_playing = false;

	StringName anim_name_for_playing;
	StringName animation_name = {};
#ifdef FULL_VERSION
	double animation_blend_time = 0.0f;
	double animation_speed = 1.0f;
	bool animation_reverse = false;
#endif

	void try_play_anim();
	void set_name(const StringName &p_name);

public:
	void entry();
	void update(real_t p_delta);
	void physics_update(real_t p_delta);
	void exit(bool p_terminated_by_upper_level = false);

	void notify_animation_finished(const StringName &p_anim);

	TransitionBase *try_transit();

	// For construct, don't call this in any cases.
	void _add_transition(TransitionBase *p_transtition) { transition_list.append(p_transtition); }
};

#pragma region 内联实现

inline StringName State::get_name() { return name; }

inline const TypedArray<State> &State::get_path() const { return path; }

inline FSM *State::get_sub_fsm() { return sub_fsm; }

inline State::StateType State::get_type() const { return type; }

inline bool State::is_animation_playing() const { return animation_playing; }

inline void State::notify_animation_finished(const StringName &p_anim) {
	if (get_animation_name_for_playing() == p_anim) {
		animation_playing = false;
		static const StringName sn = "animation_finished";
		emit_signal(sn);
	}
}

inline StringName State::get_animation_name_for_playing() const {
	return anim_name_for_playing;
}

#pragma endregion

}; // namespace Hfsm

VARIANT_ENUM_CAST(Hfsm::State::StateType);
