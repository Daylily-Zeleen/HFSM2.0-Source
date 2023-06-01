#pragma once

#include "variable_expression.hpp"

namespace Hfsm {

/**
 * @brief 普通触发器
 * 与其他类型的变量表达式一起考虑
 */
class TriggerExpression : public VariableExpression {
public:
	TriggerExpression(const Ref<HFSMVariable> &p_variable);

	bool get_result(bool p_and_mode, bool &r_result) override;
};

/**
 * @brief 独立触发器
 * 只考虑自己是否触发
 */
class SoloTriggerExpression : public TriggerExpression {
public:
	SoloTriggerExpression(const Ref<HFSMVariable> &p_variable);
	//  独立触发器只关注自己
	bool get_result(bool p_and_mode, bool &r_result) override;
	ExpressionType get_expression_type() override;
};

/**
 * @brief 联合触发器
 * 同时考虑其他 联合触发器（在山层检查时分离）
 */
class UnionTriggerExpression : public TriggerExpression {
public:
	UnionTriggerExpression(const Ref<HFSMVariable> &p_variable);

	bool get_result(bool p_and_mode, bool &r_result) override;
	ExpressionType get_expression_type() override;
};

#pragma region 内联实现

inline bool TriggerExpression::get_result(bool p_and_mode, bool &r_result) {
	r_result = variable->get_value();
	// 与 + 假  or 或 + 真
	if ((p_and_mode && !r_result) || (!p_and_mode && r_result)) {
		return true;
	} else {
		return false;
	}
}
//  独立触发器只关注自己
inline bool SoloTriggerExpression::get_result(bool p_and_mode, bool &r_result) {
	r_result = variable->get_value();
	if (r_result) {
		return true;
	} else {
		return false;
	}
}
inline VariableExpression::ExpressionType
SoloTriggerExpression::get_expression_type() {
	return ExpressionType::SOLO_TRIGGER;
}
inline bool UnionTriggerExpression::get_result(bool p_and_mode, bool &r_result) {
	r_result = variable->get_value();
	if (!r_result) {
		return true;
	}
	return false;
}
inline VariableExpression::ExpressionType
UnionTriggerExpression::get_expression_type() {
	return ExpressionType::UNION_TRIGGER;
}

#pragma endregion

} // namespace Hfsm
