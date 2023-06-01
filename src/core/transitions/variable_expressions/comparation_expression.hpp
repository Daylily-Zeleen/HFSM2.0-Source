#pragma once

#include "variable_expression.hpp"

namespace Hfsm {

/**
 * @brief 比较表达式基类
 *
 */
class ComparationExpression : public VariableExpression {
protected:
	ComparationExpression(const Ref<HFSMVariable> &p_variable, uint8_t p_op);

	uint8_t op = OP_EQUAL;
};

/**
 * @brief 与常量比较的表达式
 *
 */
class ConstantComparationExpression : public ComparationExpression {
public:
	ConstantComparationExpression(const Ref<HFSMVariable> &p_variable, uint8_t p_op,
			const Variant &p_value);
	bool get_result(bool p_and_mode, bool &r_result) override;

private:
	Variant value;
};
/**
 * @brief 与变量比较的表达式
 *
 */
class VariableComparationExpression : public ComparationExpression {
public:
	VariableComparationExpression(const Ref<HFSMVariable> &p_variable, uint8_t p_op,
			const Ref<HFSMVariable> &p_value);

	bool get_result(bool p_and_mode, bool &r_result) override;

private:
	Ref<HFSMVariable> value;
};

#pragma region 内联实现
inline bool ConstantComparationExpression::get_result(bool p_and_mode,
		bool &r_result) {
	r_result = variable->compare_with(value, op);
	// 与 + 假  or 或 + 真
	if ((p_and_mode && !r_result) || (!p_and_mode && r_result)) {
		return true;
	} else {
		return false;
	}
}
inline bool VariableComparationExpression::get_result(bool p_and_mode,
		bool &r_result) {
	r_result = variable->compare_with(value.ptr(), op);
	// 与 + 假  or 或 + 真
	if ((p_and_mode && !r_result) || (!p_and_mode && r_result)) {
		return true;
	} else {
		return false;
	}
}

#pragma endregion
} // namespace Hfsm
