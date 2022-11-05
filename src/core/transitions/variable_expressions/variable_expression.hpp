#ifndef VARIABLE_EXPRESSION_H
#define VARIABLE_EXPRESSION_H

#include "../../hfsm_variable.hpp"

namespace Hfsm {

class VariableExpression {
protected:
    VariableExpression(const Ref<HFSMVariable> &variable);

public:
    enum ExpressionType {
        NORMAL,
        UNION_TRIGGER,
        SOLO_TRIGGER,
    };
    /**
     * @brief 判断是否可以推进
     *
     * @param and_mode 与其他表达式是否为 与逻辑
     * @param r_result 返回当前的判定结果是否为真
     * @return true 已得到总结果
     * @return false 未能确定总结果
     */
    virtual bool get_result(bool and_mode, bool &r_result) = 0;
    /**
     * @brief Get the expression typ object
     *
     * @return uint8_t 0 通用 1 联合触发器 2 SOLO 触发器
     */
    virtual ExpressionType get_expression_type();

protected:
    // 变量
    Ref<HFSMVariable> _variable;
};


#pragma region 内联实现

inline VariableExpression::ExpressionType VariableExpression::get_expression_type() {
    return ExpressionType::NORMAL;
}



#pragma endregion
} // namespace Hfsm

#endif