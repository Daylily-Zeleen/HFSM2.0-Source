#pragma once

#include "../hfsm_global.hpp"

#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/templates/vector.hpp>

using namespace godot;
namespace Hfsm {

class Fsm;
class HFSM;
class TransitionBase;

class State : public RefCounted {
	GDCLASS(State, RefCounted)

protected:
	static void _bind_methods();

	String _to_string() const { return String("[State:{0}]").replace("{0}", itos(get_instance_id())); }

public:
	enum StateType {
		STATE_TYPE_NORMAL,
		STATE_TYPE_ENTRY,
		STATE_TYPE_EXIT,
		STATE_TYPE_MAX,
	};
	State();
	~State() override;

	void set_name(const StringName &name);
	StringName get_name();
	// void set_hfsm(HFSM v);
	HFSM *get_hfsm();
	bool is_exited();

	Dictionary get_context();
	// void set_context(Dictionary context){_hfsm->set_context(context);}

	void manual_exit();
	virtual void _initialize();
	virtual void _entry();
	virtual void _update(float delta);
	virtual void _physics_update(float delta);
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
	bool get_animation_reverse() const;
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

	Vector<TransitionBase *> _transition_list;
	// 子状态机
	Fsm *_fsm = nullptr;

private:
	StringName _name = "";
	HFSM *_hfsm = nullptr;

	Array _path;
	bool _exited = false;

	StateType _type = STATE_TYPE_NORMAL;

	bool _reset_when_entry = false;
	bool _reset_nested_fsm_when_entry = false;
	VMap<String, Variant> _property_to_defatul_value;

	// 新特性：动画状态机
	StringName _animation_name = {};
	bool _animation_playing = false;
#ifdef FULL_VERSION
	double _animation_blend_time = 0.0f;
	double _animation_speed = 1.0f;
	bool _animation_reverse = false;
#endif

	void reset();
	void entry();
	void update(real_t delta);
	void physics_update(real_t delta);
	void exit(bool terminated_by_upper_level = false);

	friend class FsmRes;
	friend class StateRes;
	friend class Fsm;
	friend class HFSM;
};

#pragma region 内联实现

inline StringName State::get_name() { return _name; }

inline const Array &State::get_path() const { return _path; }

inline Fsm *State::get_fsm() { return _fsm; }

inline State::StateType State::get_type() const { return _type; }

inline bool State::is_animation_playing() const { return _animation_playing; }
#pragma endregion

}; // namespace Hfsm

VARIANT_ENUM_CAST(Hfsm::State::StateType);
