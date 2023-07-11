#pragma once

#include "variable_expression.h"

namespace Hfsm {

/**
 * @brief 比较表达式基类
 *
 */
class ComparationExpression : public VariableExpression {
protected:
	ComparationExpression(const Ref<Variable> &p_variable, uint8_t p_op);

	enum Comparator {
		COMOARATOR_EQUAL,
		COMOARATOR_NOT_EQUAL,
		COMOARATOR_GREATER,
		COMOARATOR_GREATER_EQUAL,
		COMOARATOR_LESS,
		COMOARATOR_LESS_EQUAL,
	};

	Comparator op = COMOARATOR_EQUAL;

	bool compare_with(const Ref<Variable> &p_a, Comparator p_cmp, const Variant &p_b);
	bool compare_with(const Ref<Variable> &p_a, Comparator p_cmp, const Variable *p_b);
};

/**
 * @brief 与常量比较的表达式
 *
 */
class ConstantComparationExpression : public ComparationExpression {
public:
	ConstantComparationExpression(const Ref<Variable> &p_variable, uint8_t p_op,
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
	VariableComparationExpression(const Ref<Variable> &p_variable, uint8_t p_op,
			const Ref<Variable> &p_value);

	bool get_result(bool p_and_mode, bool &r_result) override;

private:
	Ref<Variable> value;
};

#pragma region 内联实现
inline bool ComparationExpression::compare_with(const Ref<Variable> &p_a, Comparator p_cmp, const Variant &p_b) {
	switch (p_cmp) {
		case COMOARATOR_EQUAL:
			return p_a->get_value() == p_b;
		case COMOARATOR_NOT_EQUAL:
			return p_a->get_value() != p_b;
		case COMOARATOR_GREATER:
			return !(p_a->get_value() < p_b || p_a->get_value() == p_b);
		case COMOARATOR_GREATER_EQUAL:
			return !(p_a->get_value() < p_b);
		case COMOARATOR_LESS:
			return p_a->get_value() < p_b;
		case COMOARATOR_LESS_EQUAL:
			return p_a->get_value() < p_b || p_a->get_value() == p_b;
		default:
			return false;
	}
}

inline bool ComparationExpression::compare_with(const Ref<Variable> &p_a, Comparator p_cmp, const Variable *p_b) {
	return compare_with(p_a, p_cmp, p_b->get_value());
}

inline bool ConstantComparationExpression::get_result(bool p_and_mode,
		bool &r_result) {
	r_result = compare_with(variable, op, value);
	// 与 + 假  or 或 + 真
	if ((p_and_mode && !r_result) || (!p_and_mode && r_result)) {
		return true;
	} else {
		return false;
	}
}

inline bool VariableComparationExpression::get_result(bool p_and_mode,
		bool &r_result) {
	r_result = compare_with(variable, op, value.ptr());
	// 与 + 假  or 或 + 真
	if ((p_and_mode && !r_result) || (!p_and_mode && r_result)) {
		return true;
	} else {
		return false;
	}
}

#pragma endregion
} // namespace Hfsm
