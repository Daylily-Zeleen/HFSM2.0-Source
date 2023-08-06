#pragma once

#include "../hfsm_global.h"

#ifdef GDEXTENSION_BUILD
#include <godot_cpp/classes/animation_player.hpp>
#include <godot_cpp/templates/vmap.hpp>
using namespace godot;
#else
#include <core/templates/vmap.h>
#include <scene/animation/animation_player.h>

#endif // GDEXTENSION_BUILD

#include "fsm_config.h"
#include "state.h"

namespace Hfsm {

// class FSMConfig;
class FSM;
// class State;
class Variable;
// 考虑状态机是非运行时设计完成的一种东西，在运行时改变状态是不合理的设计
// 因此取消运行时改变状态类型的功能
class HFSM : public Node {
	GDCLASS(HFSM, Node)

protected:
	bool _set(const StringName &p_name, const Variant &p_property);
	bool _get(const StringName &p_name, Variant &r_property) const;
	void _get_property_list(List<PropertyInfo> *p_list) const;

	static void _bind_methods();
	void _notification(int p_what);

	_TO_STRING()
public:
	HFSM();
	~HFSM() override;

	enum HFSMUpdateType {
		HFSM_UPDATE_TYPE_IDLE_AND_PHYSICS,
		HFSM_UPDATE_TYPE_IDLE,
		HFSM_UPDATE_TYPE_PHYSICS,
		HFSM_UPDATE_TYPE_MANUAL,
	};

	void manual_update();
	void manual_physics_update();
	void restart();

	Ref<Variable> get_var(const StringName &p_variable_name);
	TypedArray<Variable> get_vars();
	Variant get_var_value(const StringName &p_variable_name);
	Dictionary get_vars_value();
	void set_var(const StringName &p_variable_name, const Variant &p_value = Variant());
	void set_trigger(const StringName &p_trigger_name);
	void set_boolean(const StringName &p_boolean_name, bool p_value);
	void set_integer(const StringName &p_interger_name, int64_t p_value);
	void set_float(const StringName &p_float_name, double p_value);
	void set_string(const StringName &p_string_name, const String &p_value);

	// Dictionary get_context() { return context; }
	// void set_context(const Dictionary &p_context) { context = p_context; }
	// // 以下段落考虑弃用
	// void set_entry_state(String state_name, Array fsm_path = root_path);
	// void set_exit_state(String state_name, Array fsm_path = root_path);
	// void set_unique_exit_state(String state_name, Array fsm_path =
	// root_path); void set_normal_state(String state_name, Array fsm_path =
	// root_path);
	// // 以下段落考虑弃用
	// void force_entry(Array fsm_path = root_path, String state_name = "");
	// void force_exit(Array fsm_path = root_path);
	// void force_transit(String target_state , Array fsm_path = root_path);

	// virtual PackedStringArray _get_configuration_warnings() const override;
	// Array get_property_list() const;

	// setget

	void set_active(bool p_v);
	bool is_active();

	void set_update_type(HFSMUpdateType p_t);
	HFSMUpdateType get_update_type();

	Ref<State> get_current_state();
	Ref<State> get_previous_state();

	void set_root_fsm_config(const Ref<FSMConfig> &p_root_fsm_config);
	Ref<FSMConfig> get_root_fsm_config() const;

	void set_animation_player(AnimationPlayer *p_animtion_player);
	AnimationPlayer *get_animation_player() const { return animation_player; }

	// 重写以实现逻辑
	void process_internal(double p_delta);
	void physics_process_internal(double p_delta);

#ifdef ROLLBACK_NET_CODE
	virtual Array _save_state();
	virtual void _load_state(const Array &p_state);
	virtual void _interpolate_state(const Array &p_old_state, const Array &p_new_state, real_t p_weight);
	virtual Array _get_local_input();
	virtual Array _predict_remote_input(const Array &p_previous_input, int64_t p_ticks_since_real_input);
	virtual void _network_process(Array &p_input);
	virtual void _network_preprocess(Array &p_input);
	virtual void _network_postprocess(Array &p_input);
	virtual Dictionary &_network_spawn_preprocess(Dictionary &p_data);
	virtual void _network_spawn(Dictionary &p_data);
	virtual void _network_despawn();
#endif

	bool rebuild_hfsm();

private:
	bool active = true;

	// Dictionary agents;
	HFSMUpdateType update_type = HFSMUpdateType::HFSM_UPDATE_TYPE_IDLE_AND_PHYSICS;

	// 高级选项
	bool disable_rename_to_snake_case = false;
	// bool force_all_state_entry_behavior = ForceType::NOT_FORCE;
	// bool force_all_fsm_entry_behavior = ForceType::NOT_FORCE;
	//
	// resource inspector_config;
	Ref<FSMConfig> root_fsm_config;

	Ref<State> current_state; //= ["root"] setget , get_current_path
	Ref<State> previous_state; // :Array = ["root"] setget , get_previous_path

	FSM *root_fsm = nullptr;
	Vector<Ref<Variable>> trigger_list;
	VMap<StringName, Ref<Variable>> variable_blackboard;

	LocalVector<FSM *> *active_fsm_list = nullptr;

	AnimationPlayer *animation_player = nullptr;

	void flush_trigger();
	// 信号发射器 , 由 fsm 调用
	void emit_updated(const Ref<State> &p_state, double p_delta);
	void emit_physic_updated(const Ref<State> &state, double p_delta);
	void emit_transited(const Ref<State> &p_from_state, const Ref<State> &p_to_state);
	void emit_entered(const Ref<State> &p_state);
	void emit_exited(const Ref<State> &p_state);

	// 新特性：动画状态机
	void _animation_finished(const StringName &p_anim_name);

	friend class FSM;
};

#pragma region 内联实现
inline void HFSM::set_active(bool p_v) {
	active = p_v;
	notify_property_list_changed();
}
inline bool HFSM::is_active() { return active; }

inline HFSM::HFSMUpdateType HFSM::get_update_type() { return update_type; }

inline Ref<State> HFSM::get_current_state() { return current_state; }
inline Ref<State> HFSM::get_previous_state() { return previous_state; }

inline void HFSM::_animation_finished(const StringName &p_anim_name) {
	if (current_state.is_valid()) {
		current_state->notify_animation_finished(p_anim_name);
	}
}
#pragma endregion

}; // namespace Hfsm

VARIANT_ENUM_CAST(Hfsm::HFSM::HFSMUpdateType);
