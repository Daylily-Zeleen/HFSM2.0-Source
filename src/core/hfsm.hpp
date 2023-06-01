#pragma once

#include <godot_cpp/classes/animation_player.hpp>
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/classes/ref.hpp>
#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/templates/vector.hpp>
#include <godot_cpp/templates/vmap.hpp>

#include "../hfsm_global.hpp"
#include "godot_cpp/templates/pair.hpp"
#include "state.hpp"

#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;
namespace Hfsm {

class FsmRes;
class Fsm;
// class State;
class HFSMVariable;
// 考虑状态机是非运行时设计完成的一种东西，在运行时改变状态是不合理的设计
// 因此取消运行时改变状态类型的功能
class HFSM : public Node {
	GDCLASS(HFSM, Node)

protected:
	static void _bind_methods();

	String _to_string() const { return vformat("[HFSM:%d]", get_instance_id()); }

public:
	HFSM();
	~HFSM() override;

	bool _set(const StringName &p_name, const Variant &p_property);
	bool _get(const StringName &p_name, Variant &r_property) const;
	void _get_property_list(List<PropertyInfo> *p_list) const;

	enum UpdateType {
		UPDATE_TYPE_IDLE_AND_PHYSICS,
		UPDATE_TYPE_IDLE,
		UPDATE_TYPE_PHYSICS,
		UPDATE_TYPE_MANUAL,
	};

	void manual_update();
	void manual_physics_update();
	void restart();

	Ref<HFSMVariable> get_var(const StringName &p_variable_name);
	Array get_vars();
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
	bool is_inited();

	void set_active(bool p_v);
	bool is_active();

	void set_debug(bool p_v);
	bool is_debug();
	// void set_agents(Dictionary a);
	// Dictionary get_agents() const;

	void set_update_type(UpdateType p_t);
	UpdateType get_update_type();

	Ref<State> get_current_state();
	Ref<State> get_previous_state();

	void set_root_fsm_res(const Ref<FsmRes> &p_root_fsm_res);
	Ref<FsmRes> get_root_fsm_res() const;

	void set_animation_player(AnimationPlayer *p_animtion_player);
	AnimationPlayer *get_animation_player() const { return animation_player; }

	// ExpressiontTransition 专用
	PackedStringArray &get_expression_objs_names();
	Array &get_expression_objs();

	// 重写以实现逻辑
	void _ready() override;
	void _process(double p_delta) override;
	void _physics_process(double p_delta) override;

	// 信号回调
	void ___on_tree_entered__();
	void ___on_ready__();

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

private:
	bool inited = false;
	bool active = true;
	bool debug = false;
	// Dictionary agents;
	UpdateType update_type = UpdateType::UPDATE_TYPE_IDLE_AND_PHYSICS;

	// 高级选项
	bool disable_rename_to_snake_case = false;
	// bool force_all_state_entry_behavior = ForceType::NOT_FORCE;
	// bool force_all_fsm_entry_behavior = ForceType::NOT_FORCE;
	//
	// resource inspector_res;
	Ref<FsmRes> root_fsm_res;
	Ref<State> current_state; //= ["root"] setget , get_current_path
	Ref<State> previous_state; // :Array = ["root"] setget , get_previous_path

	Fsm *root_fsm = nullptr;
	Vector<Ref<HFSMVariable>> trigger_list;
	VMap<StringName, Ref<HFSMVariable>> variable_blackboard;

	Vector<Fsm *> *active_fsm_list = nullptr;

	AnimationPlayer *animation_player = nullptr;

	// // 新增 上下文
	// Dictionary context;

	PackedStringArray expression_objs_names;
	Array expression_objs;

	void generate_hfsm();
	void flush_trigger();
	// 信号发射器 , 由 fsm 调用
	void updated(Ref<State> &p_state, double p_delta);
	void physic_updated(Ref<State> &state, double p_delta);
	void transited(Ref<State> &p_from_state, Ref<State> &p_to_state);
	void entered(Ref<State> &p_state);
	void exited(Ref<State> &p_state);

	// 新特性：动画状态机
	void __on_animation_finished(const StringName &p_anim_name);

	friend class Fsm;
};

#pragma region 内联实现

inline bool HFSM::is_inited() { return inited; }
inline void HFSM::set_active(bool p_v) {
	active = p_v;
	// TODO:: 失能处理
	notify_property_list_changed();
}
inline bool HFSM::is_active() { return active; }
inline void HFSM::set_debug(bool p_v) {
	debug = p_v;
	notify_property_list_changed();
	// TODO:: 添加调试器
}
inline bool HFSM::is_debug() { return debug; }
// void set_agents(Dictionary a);
// inline Dictionary HFSM::get_agents() const { return agents; }
inline HFSM::UpdateType HFSM::get_update_type() { return update_type; }

inline Ref<State> HFSM::get_current_state() { return current_state; }
inline Ref<State> HFSM::get_previous_state() { return previous_state; }

// ExpressiontTransition 专用
inline PackedStringArray &HFSM::get_expression_objs_names() { return expression_objs_names; }
inline Array &HFSM::get_expression_objs() { return expression_objs; }

// 新特性：动画状态机

inline void HFSM::__on_animation_finished(const StringName &p_anim_name) {
	if (current_state.is_valid() && current_state->get_animation_name_for_playing() == p_anim_name) {
		current_state->animation_playing = false;
	}
}
#pragma endregion

}; // namespace Hfsm

VARIANT_ENUM_CAST(Hfsm::HFSM::UpdateType);
