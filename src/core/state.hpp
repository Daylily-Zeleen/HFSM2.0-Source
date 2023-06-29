#pragma once

#include <hfsm_global.hpp>

#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/templates/vector.hpp>
#include <godot_cpp/templates/vmap.hpp>

using namespace godot;
namespace Hfsm {

class Fsm;
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

	~State() override;

	void set_name(const StringName &p_name);
	StringName get_name();
	// void set_hfsm(HFSM v);
	HFSM *get_hfsm();
	bool is_exited();

	// Dictionary get_context();
	// void set_context(Dictionary context){_hfsm->set_context(context);}

	void manual_exit();
	virtual void _initialize();
	virtual void _entry();
	virtual void _update(float p_delta);
	virtual void _physics_update(float p_delta);
	virtual void _exit();

	const Array &get_path() const;

	StateType get_type() const;

	Fsm *get_fsm();

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

	Vector<TransitionBase *> transition_list;
	// 子状态机
	Fsm *fsm = nullptr;

private:
	StringName name = "";
	HFSM *hfsm = nullptr;

	Array path;
	bool exited = false;

	StateType type = STATE_TYPE_NORMAL;

	bool reset_when_entry = false;
	bool reset_nested_fsm_when_entry = false;
	VMap<String, Variant> property_to_defatul_value;

	// 新特性：动画状态机
	StringName animation_name = {};
	bool animation_playing = false;
#ifdef FULL_VERSION
	double animation_blend_time = 0.0f;
	double animation_speed = 1.0f;
	bool animation_reverse = false;
#endif

	void reset();
	void entry();
	void update(real_t p_delta);
	void physics_update(real_t p_delta);
	void exit(bool p_terminated_by_upper_level = false);

	// friend class FsmRes;
	friend class StateRes;
	friend class Fsm;
	friend class HFSM;
};

#pragma region 内联实现

inline StringName State::get_name() { return name; }

inline const Array &State::get_path() const { return path; }

inline Fsm *State::get_fsm() { return fsm; }

inline State::StateType State::get_type() const { return type; }

inline bool State::is_animation_playing() const { return animation_playing; }
#pragma endregion

}; // namespace Hfsm

VARIANT_ENUM_CAST(Hfsm::State::StateType);
