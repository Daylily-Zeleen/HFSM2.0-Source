#ifndef HFSM_H
#define HFSM_H

#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/classes/ref.hpp>
#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/templates/vector.hpp>
#include <godot_cpp/templates/vmap.hpp>

#include "../hfsm_global.hpp"
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

    String _to_string() const { return String("[HFSM:{0}]").replace("{0}", itos(get_instance_id())); }

public:
    HFSM();
    ~HFSM();

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

    Ref<HFSMVariable> get_var(const StringName &variable_name);
    Array get_vars();
    Variant get_var_value(const StringName &variable_name);
    Dictionary get_vars_value();
    void set_var(const StringName &variable_name, Variant value = Variant());
    void set_trigger(const StringName &trigger_name);
    void set_boolean(const StringName &boolean_name, bool value);
    void set_integer(const StringName &interger_name, int64_t value);
    void set_float(const StringName &float_name, double value);
    void set_string(const StringName &string_name, const String &value);

    Dictionary get_context() { return _context; }
    void set_context(Dictionary context) { _context = context; }
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

    void set_active(bool v);
    bool is_active();

    void set_debug(bool v);
    bool is_debug();
    // void set_agents(Dictionary a);
    Dictionary get_agents() const;

    void set_update_type(UpdateType t);
    UpdateType get_update_type();

    Ref<State> get_current_state();
    Ref<State> get_previous_state();

    void set_root_fsm_res(const Ref<FsmRes> &root_fsm_res);
    Ref<FsmRes> get_root_fsm_res() const;

    // ExpressiontTransition 专用
    PackedStringArray &get_expression_objs_names();
    Array &get_expression_objs();

    // 重写以实现逻辑
    void _ready() override;
    void _process(double delta) override;
    void _physics_process(double delta) override;

    // 信号回调
    void ___on_tree_entered__();
    void ___on_ready__();

#ifdef ROLLBACK_NET_CODE
    virtual Array _save_state();
    virtual void _load_state(const Array &state);
    virtual void _interpolate_state(const Array &old_state, const Array &new_state, real_t weight);
    virtual Array _get_local_input();
    virtual Array _predict_remote_input(const Array &previous_input, int64_t ticks_since_real_input);
    virtual void _network_process(Array &input);
    virtual void _network_preprocess(Array &input);
    virtual void _network_postprocess(Array &input);
    virtual Dictionary &_network_spawn_preprocess(Dictionary &data);
    virtual void _network_spawn(Dictionary &data);
    virtual void _network_despawn();
#endif

private:
    bool _inited = false;
    bool _active = true;
    bool _debug = false;
    Dictionary _agents;
    UpdateType _update_type = UpdateType::UPDATE_TYPE_IDLE_AND_PHYSICS;

    // 高级选项
    bool _disable_rename_to_snake_case = false;
    // bool _force_all_state_entry_behavior = ForceType::NOT_FORCE;
    // bool _force_all_fsm_entry_behavior = ForceType::NOT_FORCE;
    //
    // resource _inspector_res;
    Ref<FsmRes> _root_fsm_res;
    Ref<State> _current_state;  //= ["root"] setget , get_current_path
    Ref<State> _previous_state; // :Array = ["root"] setget , get_previous_path

    Fsm *_root_fsm = nullptr;
    Vector<Ref<HFSMVariable>> _trigger_list;
    VMap<StringName, Ref<HFSMVariable>> _variable_blackboard;

    Vector<Fsm *> *_active_fsm_list = nullptr;

    // 新增 上下文
    Dictionary _context;

    PackedStringArray _expression_objs_names;
    Array _expression_objs;

    void generate_hfsm();
    void flush_trigger();
    // 信号发射器 , 由 fsm 调用
    void updated(Ref<State> &state, double delta);
    void physic_updated(Ref<State> &state, double delta);
    void transited(Ref<State> &from_state, Ref<State> &to_state);
    void entered(Ref<State> &state);
    void exited(Ref<State> &state);

    friend class Fsm;
};

#pragma region 内联实现

inline bool HFSM::is_inited() { return _inited; }
inline void HFSM::set_active(bool v) {
    _active = v;
    // TODO:: 失能处理
    notify_property_list_changed();
}
inline bool HFSM::is_active() { return _active; }
inline void HFSM::set_debug(bool v) {
    _debug = v;
    notify_property_list_changed();
    // TODO:: 添加调试器
}
inline bool HFSM::is_debug() { return _debug; }
// void set_agents(Dictionary a);
inline Dictionary HFSM::get_agents() const { return _agents; }
inline HFSM::UpdateType HFSM::get_update_type() { return _update_type; }

inline Ref<State> HFSM::get_current_state() { return _current_state; }
inline Ref<State> HFSM::get_previous_state() { return _previous_state; }

// ExpressiontTransition 专用
inline PackedStringArray &HFSM::get_expression_objs_names() { return _expression_objs_names; }
inline Array &HFSM::get_expression_objs() { return _expression_objs; }

#pragma endregion

}; // namespace Hfsm

VARIANT_ENUM_CAST(Hfsm::HFSM::UpdateType);

#endif