#ifndef VARIABLE_EXPRESSION_RES_H
#define VARIABLE_EXPRESSION_RES_H
#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/core/binder_common.hpp>

#include "../../hfsm_variable_res.hpp"

using namespace godot;

namespace Hfsm {

class HFSM;
class VariableExpression;

class VariableExpressionRes : public Resource {
    GDCLASS(VariableExpressionRes, Resource)

protected:
    static void _bind_methods();

    String _to_string() const {
        return String("[VariableExpressionRes:{0}]")
            .replace("{0}", itos(get_instance_id()));
    }

public:
    bool _set(const StringName &p_name, const Variant &p_property);
    bool _get(const StringName &p_name, Variant &r_property) const;
    void _get_property_list(List<PropertyInfo> *p_list) const;

    enum TriggerType {
        TRIGGER_TYPE_SOLO,
        TRIGGER_TYPE_UNION,
        TRIGGER_TYPE_NORMAL,
        TRIGGER_TYPE_MAX,
    };
    enum Op {
        OP_EQUAL,
        OP_NOT_EQUAL,
        OP_GREATER,
        OP_GREATER_EQUAL,
        OP_LESS,
        OP_LESS_EQUAL,
    };

    VariableExpressionRes();

    void set_variable_res(Ref<HFSMVariableRes> variable_res);
    Ref<HFSMVariableRes> get_variable_res() const;

    void set_value(Variant value);
    Variant get_value() const;

    void set_operator(int64_t op);
    uint8_t get_operator() const;

    void set_trigger_type(int64_t trigger_type);
    uint8_t get_trigger_type() const;

    void set_variable_as_value(bool variable_as_value);
    bool is_variable_as_value() const;

    // Dictionary get_valid_and_text();

    // Array get_property_list() const;

    VariableExpression *create_variable_expression(HFSM *hfsm);

private:
    Ref<HFSMVariableRes> _variable_res;
    Variant _value;
    uint8_t _operator = OP_EQUAL;
    // trigger
    uint8_t _trigger_type = TRIGGER_TYPE_SOLO;

    // 是否使用另一个 变量资源作为 比较值
    bool _variable_as_value = false;
};

} // namespace Hfsm

VARIANT_ENUM_CAST(Hfsm::VariableExpressionRes::TriggerType);
VARIANT_ENUM_CAST(Hfsm::VariableExpressionRes::Op);

#endif