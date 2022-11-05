#include "variable_expression_res.hpp"
#include "../../hfsm.hpp"
#include "../variable_transition.hpp"
#include <godot_cpp/classes/translation_server.hpp>

namespace Hfsm {
#pragma region VariableExpressionRes
VariableExpressionRes::VariableExpressionRes() {
    UtilityFunctions::print("==VariableExpressionRes");
}

Ref<HFSMVariableRes> VariableExpressionRes::get_variable_res() const {
    return _variable_res;
}

Variant VariableExpressionRes::get_value() const { return _value; }

uint8_t VariableExpressionRes::get_operator() const { return _operator; }

uint8_t VariableExpressionRes::get_trigger_type() const {
    return _trigger_type;
}

bool VariableExpressionRes::is_variable_as_value() const {
    return _variable_as_value;
}

VariableExpression *
VariableExpressionRes::create_variable_expression(HFSM *hfsm) {
    ERR_FAIL_COND_V(!_variable_res.is_valid(), nullptr);
    // 获取变量类型
    auto v = hfsm->get_var(_variable_res->get_name());
    ERR_FAIL_COND_V(!v.is_valid(), nullptr);
    //  触发器
    if (v->get_type() == Variant::NIL) {
        switch (_trigger_type) {
        case TRIGGER_TYPE_NORMAL:
            return memnew(TriggerExpression(v));
        case TRIGGER_TYPE_UNION:
            return memnew(UnionTriggerExpression(v));
        case TRIGGER_TYPE_SOLO:
            return memnew(SoloTriggerExpression(v));
        default:
            break;
        }
    } else {
        if (_variable_as_value) {
            return memnew(VariableComparationExpression(_variable_res,
                                                        _operator, _value));
        } else {
            return memnew(ConstantComparationExpression(_variable_res,
                                                        _operator, _value));
        }
    }
    return nullptr;
}

// bool VariableExpressionRes::is_valid() {
//     if (_variable_res.is_valid()){
//         if (_variable_res->get_type() != Variant::NIL){
//             if(_variable_as_value){
//                 auto vr = Object::cast_to<HFSMVariableRes>(_value);
//                 if(vr) return
//                 Variant::can_convert(Variant::Type(vr->get_type()) ,
//                 Variant::Type(_variable_res->get_type()));
//             }else{
//                 return Variant::can_convert(_value.get_type() ,
//                 Variant::Type(_variable_res->get_type()));
//             }
//         }else{
//             return true;
//         }
//     }
//     return false;
// }

bool VariableExpressionRes::_set(const StringName &p_name,
                                 const Variant &p_property) {
    if (p_name == StringName("comparator")) {
        set_operator(p_property);
        return true;
    } else if (p_name == StringName("variable_as_value")) {
        set_variable_as_value(p_property);
        return true;
    } else if (p_name == StringName("value")) {
        set_value(p_property);
        return true;
    } else if (p_name == StringName("trigger_type")) {
        set_trigger_type(p_property);
        return true;
    }
    return false;
}
bool VariableExpressionRes::_get(const StringName &p_name,
                                 Variant &r_property) const {
    if (p_name == StringName("comparator")) {
        r_property = get_operator();
        return true;
    } else if (p_name == StringName("variable_as_value")) {
        r_property = is_variable_as_value();
        return true;
    } else if (p_name == StringName("value")) {
        r_property = get_value();
        return true;
    } else if (p_name == StringName("trigger_type")) {
        r_property = get_trigger_type();
        return true;
    }
    return false;
}
void VariableExpressionRes::_get_property_list(
    List<PropertyInfo> *p_list) const {
    if (_variable_res.is_valid()) {
        auto comparator_hint = "==,!=,>,>=,<,<=";
        switch (Variant::Type(_variable_res->get_type())) {
        case Variant::BOOL:
        case Variant::STRING:
            comparator_hint = "==,!=";
            break;
        case Variant::INT:
        case Variant::FLOAT:
            comparator_hint = "==,!=,>,>=,<,<=";
            break;
        default:
            comparator_hint = "";
            break;
        }
        p_list->push_back(PropertyInfo(Variant::INT, "comparator",
                                       PROPERTY_HINT_ENUM, comparator_hint));
        p_list->push_back(PropertyInfo(Variant::BOOL, "variable_as_value"));
        if (is_variable_as_value()) {
            p_list->push_back(PropertyInfo(Variant::OBJECT, "value",
                                           PROPERTY_HINT_RESOURCE_TYPE,
                                           "HFSMVariableRes"));
        } else if (_variable_res.is_valid()) {
            if (Variant::Type(int64_t(_variable_res->get("type"))) !=
                Variant::NIL) {
                Variant::Type t =
                    Variant::Type(int64_t(_variable_res->get("type")));
                p_list->push_back(PropertyInfo(t, "value"));
            } else {
                p_list->push_back(PropertyInfo(Variant::INT, "trigger_type",
                                               PROPERTY_HINT_ENUM,
                                               "Solo,Union,Normal"));
            }
        }
    }
}

void VariableExpressionRes::_bind_methods() {
    // 变量资源
    ClassDB::bind_method(D_METHOD("get_variable_res"),
                         &VariableExpressionRes::get_variable_res);
    ClassDB::bind_method(D_METHOD("set_variable_res", "variable_res"),
                         &VariableExpressionRes::set_variable_res);
    ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "variable_res",
                              PROPERTY_HINT_RESOURCE_TYPE, "HFSMVariableRes"),
                 "set_variable_res", "get_variable_res");

    // 操作符
    ClassDB::bind_method(D_METHOD("get_comparator"),
                         &VariableExpressionRes::get_operator);
    ClassDB::bind_method(D_METHOD("set_comparator", "comparator"),
                         &VariableExpressionRes::set_operator);
    // ADD_PROPERTY(PropertyInfo(Variant::INT, "comparator", PROPERTY_HINT_ENUM,
    //                           "==,!=,>,>=,<,<="),
    //              "set_comparator", "get_comparator");

    //
    ClassDB::bind_method(D_METHOD("is_variable_as_value"),
                         &VariableExpressionRes::is_variable_as_value);
    ClassDB::bind_method(D_METHOD("set_variable_as_value", "variable_as_value"),
                         &VariableExpressionRes::set_variable_as_value);
    // ADD_PROPERTY(PropertyInfo(Variant::BOOL, "variable_as_value"),
    //              "set_variable_as_value", "is_variable_as_value");

    // 值
    ClassDB::bind_method(D_METHOD("get_value"),
                         &VariableExpressionRes::get_value);
    ClassDB::bind_method(D_METHOD("set_value", "value"),
                         &VariableExpressionRes::set_value);
    // ADD_PROPERTY(PropertyInfo(Variant::NIL, "value"), "set_value",
    // "get_value");

    // 触发器
    ClassDB::bind_method(D_METHOD("get_trigger_type"),
                         &VariableExpressionRes::get_trigger_type);
    ClassDB::bind_method(D_METHOD("set_trigger_type", "trigger_type"),
                         &VariableExpressionRes::set_trigger_type);
    // ADD_PROPERTY(PropertyInfo(Variant::INT, "trigger_type",
    // PROPERTY_HINT_ENUM,
    //                           "Solo,Union,Normal"),
    //              "set_value", "get_value");

    // 枚举
    BIND_CONSTANT(TRANSITION_TYPE_SCRIPT);
    BIND_CONSTANT(TRANSITION_TYPE_VARIABLE);
    BIND_CONSTANT(TRANSITION_TYPE_EXPRESSION);
    BIND_CONSTANT(TRANSITION_TYPE_AUTO);

    BIND_CONSTANT(OP_EQUAL);
    BIND_CONSTANT(OP_NOT_EQUAL);
    BIND_CONSTANT(OP_GREATER);
    BIND_CONSTANT(OP_GREATER_EQUAL);
    BIND_CONSTANT(OP_LESS);
    BIND_CONSTANT(OP_LESS_EQUAL);
}

void VariableExpressionRes::set_variable_as_value(bool variable_as_value) {
    _variable_as_value = variable_as_value;
    if (_variable_as_value) {
        if ((_value.get_type() != Variant::OBJECT) ||
            Object::cast_to<HFSMVariableRes>(_value) != nullptr) {
            _value = Variant();
            emit_changed();
        }
    } else {
        if (!Variant::can_convert(_value.get_type(),
                                  Variant::Type(_variable_res->get_type())))
            _value = _variable_res->get_default_val();
        emit_changed();
    }
    notify_property_list_changed();
}
void VariableExpressionRes::set_value(Variant value) {
    if (!_variable_res.is_valid()) {
        _value = Variant();
        emit_changed();
        notify_property_list_changed();
    } else {
        if (_variable_as_value && value.get_type() == Variant::OBJECT) {
            // 变量作为比较值
            auto v = Object::cast_to<HFSMVariableRes>(value);
            if (v) {
                if (_value != value &&
                    Variant::can_convert_strict(
                        Variant::Type(_variable_res->get_type()),
                        Variant::Type(v->get_type()))) {
                    _value = value;
                    emit_changed();
                    notify_property_list_changed();
                } else {
                    _value = Variant();
                    emit_changed();
                    notify_property_list_changed();
                }
                return;
            }
        } else {
            // 常量作为比较值
            if (Variant::can_convert_strict(
                    value.get_type(),
                    Variant::Type(_variable_res->get_type()))) {
                if (_value != value) {
                    _value = value;
                    emit_changed();
                    // notify_property_list_changed();
                }
                return;
            }
        }
        // 异常情况用默认值
        if (_value != _variable_res->get_default_val()) {
            _value = _variable_res->get_default_val();
            emit_changed();
            notify_property_list_changed();
        }
    }
}
void VariableExpressionRes::set_operator(int64_t op) {
    if (_variable_res.is_valid()) {
        if (_variable_res->get_type() == Variant::STRING) {
            if (op == OP_EQUAL && op == OP_NOT_EQUAL) {
                if (op != _operator) {
                    _operator = op;
                    emit_changed();
                    notify_property_list_changed();
                    return;
                }
            }
        } else if (_variable_res->get_type() == Variant::INT ||
                   _variable_res->get_type() == Variant::FLOAT) {
            if (op >= OP_EQUAL && op < 6) {
                if (op != _operator) {
                    _operator = op;
                    emit_changed();
                    notify_property_list_changed();
                    return;
                }
            }
        }
    }
    if (_operator != OP_EQUAL) {
        _operator = OP_EQUAL;
        emit_changed();
        notify_property_list_changed();
    }
}
void VariableExpressionRes::set_variable_res(
    Ref<HFSMVariableRes> variable_res) {
    if (_variable_res != variable_res) {
        _variable_res = variable_res;
        set_value(_value);
        set_operator(_operator);
        emit_changed();
        notify_property_list_changed();
    }
}

void VariableExpressionRes::set_trigger_type(int64_t trigger_type) {
    ERR_FAIL_COND(!(trigger_type >= 0 && trigger_type < TRIGGER_TYPE_MAX));
    _trigger_type = uint8_t(trigger_type);
    emit_changed();
    notify_property_list_changed();
}

// Array VariableExpressionRes::get_property_list() {
//     UtilityFunctions::print("call in get_property_list");
//     auto ret = Resource::get_property_list();
//     Dictionary variable_res, value, op, trigger_type, variable_as_value;
//     // 变量资源
//     variable_res[Variant("name")] = Variant("variable_res");
//     variable_res[Variant("type")] = Variant(Variant::OBJECT);
//     variable_res[Variant("hint")] = Variant(PROPERTY_HINT_RESOURCE_TYPE);
//     variable_res[Variant("hint_string")] =
//         Variant("HFSMVariableRes"); // TODO:: 可能要改为 Resource
//     ret.push_back(variable_res);
//     if (_variable_res.is_valid()) {
//         switch (_variable_res->get_type()) {
//         case Variant::NIL:
//             // 触发器类型
//             trigger_type[Variant("name")] = Variant("trigger_type");
//             trigger_type[Variant("type")] = Variant(Variant::INT);
//             trigger_type[Variant("hint")] = Variant(PROPERTY_HINT_ENUM);
//             trigger_type[Variant("hint_string")] =
//             Variant("Solo,Union,Normal"); ret.push_back(trigger_type); break;
//         case Variant::INT:
//         case Variant::BOOL:
//         case Variant::FLOAT:
//         case Variant::STRING:
//             // 比较符
//             op[Variant("name")] = Variant("comparator");
//             op[Variant("type")] = Variant(Variant::INT);
//             op[Variant("hint")] = Variant(PROPERTY_HINT_ENUM);
//             if (_variable_res->get_type() == Variant::STRING ||
//                 _variable_res->get_type() == Variant::BOOL)
//                 op[Variant("hint_string")] = Variant("==,!=");
//             else
//                 op[Variant("hint_string")] = Variant("==,!=,>,>=,<,<=");
//             ret.push_back(op);
//             // 是否使用变量作为值
//             variable_as_value[Variant("name")] =
//             Variant("variable_as_value"); variable_as_value[Variant("type")]
//             = Variant(Variant::BOOL); ret.push_back(variable_as_value);
//             // 值
//             value[Variant("name")] = Variant("value");
//             if (_variable_as_value) {
//                 variable_res[Variant("type")] = Variant(Variant::OBJECT);
//                 variable_res[Variant("hint")] =
//                     Variant(PROPERTY_HINT_RESOURCE_TYPE);
//                 variable_res[Variant("hint_string")] =
//                     Variant("HFSMVariableRes"); // TODO:: 可能要改为 Resource
//                 set_value(_value);
//             } else {
//                 variable_res[Variant("type")] =
//                     Variant(_variable_res->get_type());
//             }
//             ret.push_back(value);
//         default:
//             break;
//         }
//     }
//     return ret;
// }

#pragma endregion

} // namespace Hfsm
