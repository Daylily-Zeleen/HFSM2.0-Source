#ifndef COMPARATION_EXPRESSION_H
#define COMPARATION_EXPRESSION_H

#include "variable_expression.hpp"

namespace Hfsm {

/**
 * @brief 比较表达式基类
 *
 */
class ComparationExpression : public VariableExpression {
protected:
    ComparationExpression(const Ref<HFSMVariable> &variable, uint8_t op);

    uint8_t _op = OP_EQUAL;
};
/**
 * @brief 与常量比较的表达式
 *
 */
class ConstantComparationExpression : public ComparationExpression {
public:
    ConstantComparationExpression(const Ref<HFSMVariable> &variable, uint8_t op,
                                  const Variant &value);
    bool get_result(bool and_mode, bool &r_result) override;

private:
    Variant _value;
};
/**
 * @brief 与变量比较的表达式
 *
 */
class VariableComparationExpression : public ComparationExpression {
public:
    VariableComparationExpression(Ref<HFSMVariable> variable, uint8_t op,
                                  Ref<HFSMVariable> value);

    bool get_result(bool and_mode, bool &r_result) override;

private:
    Ref<HFSMVariable> _value;
};


#pragma region 内联实现


inline bool ConstantComparationExpression::get_result(bool and_mode,
                                                      bool &r_result) {
    r_result = _variable->compare_with(_value, _op);
    // 与 + 假  or 或 + 真
    if ((and_mode && !r_result) || (!and_mode && r_result))
        return true;
    else
        return false;
}
inline bool VariableComparationExpression::get_result(bool and_mode,
                                                      bool &r_result) {
    r_result = _variable->compare_with(_value.ptr(), _op);
    // 与 + 假  or 或 + 真
    if ((and_mode && !r_result) || (!and_mode && r_result))
        return true;
    else
        return false;
}

#pragma endregion
} // namespace Hfsm

#endif