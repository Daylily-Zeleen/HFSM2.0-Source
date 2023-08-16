#pragma once

#include "variable_expression.h"

namespace Hfsm {

/**
 * @brief 比较表达式基类
 *
 */
class ComparationExpression : public VariableExpression {
public:
	enum Comparator {
		COMPARATOR_EQUAL,
		COMPARATOR_NOT_EQUAL,
		COMPARATOR_GREATER,
		COMPARATOR_GREATER_EQUAL,
		COMPARATOR_LESS,
		COMPARATOR_LESS_EQUAL,
	};

protected:
	Comparator comparator = COMPARATOR_EQUAL;

	ComparationExpression(const Ref<Variable> &p_variable, Comparator p_op);
	bool compare_with(const Ref<Variable> &p_a, Comparator p_cmp, const Variant &p_b);
	bool compare_with(const Ref<Variable> &p_a, Comparator p_cmp, const Variable *p_b);
};

/**
 * @brief 与常量比较的表达式
 *
 */
class ConstantComparationExpression : public ComparationExpression {
public:
	ConstantComparationExpression(const Ref<Variable> &p_variable, Comparator p_op, const Variant &p_value);
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
	VariableComparationExpression(const Ref<Variable> &p_variable, Comparator p_op, const Ref<Variable> &p_value);

	bool get_result(bool p_and_mode, bool &r_result) override;

private:
	Ref<Variable> value;
};

#pragma region 内联实现
inline bool ComparationExpression::compare_with(const Ref<Variable> &p_a, Comparator p_cmp, const Variant &p_b) {
	switch (p_cmp) {
		case COMPARATOR_EQUAL:
			return p_a->get_value() == p_b;
		case COMPARATOR_NOT_EQUAL:
			return p_a->get_value() != p_b;
		case COMPARATOR_GREATER:
			return !(p_a->get_value() < p_b || p_a->get_value() == p_b);
		case COMPARATOR_GREATER_EQUAL:
			return !(p_a->get_value() < p_b);
		case COMPARATOR_LESS:
			return p_a->get_value() < p_b;
		case COMPARATOR_LESS_EQUAL:
			return p_a->get_value() < p_b || p_a->get_value() == p_b;
		default:
			return false;
	}
}

inline bool ComparationExpression::compare_with(const Ref<Variable> &p_a, Comparator p_cmp, const Variable *p_b) {
	return compare_with(p_a, p_cmp, p_b->get_value());
}

#pragma endregion
} // namespace Hfsm
