/**************************************************************************/
/*  compare_expression.h                                              */
/**************************************************************************/
/*                         This file is part of:                          */
/*                   Hierarchical Finite State Machine                    */
/*            https://github.com/Daylily-Zeleen/HFSM2.0-Source            */
/**************************************************************************/
/* Copyright (c) 2023-present Daylily Zeleen.                             */
/*                                                                        */
/* Permission is hereby granted, free of charge, to any person obtaining  */
/* a copy of this software and associated documentation files (the        */
/* "Software"), to deal in the Software without restriction, including    */
/* without limitation the rights to use, copy, modify, merge, publish,    */
/* distribute, sublicense, and/or sell copies of the Software, and to     */
/* permit persons to whom the Software is furnished to do so, subject to  */
/* the following conditions:                                              */
/*                                                                        */
/* The above copyright notice and this permission notice shall be         */
/* included in all copies or substantial portions of the Software.        */
/*                                                                        */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. */
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY   */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,   */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE      */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                 */
/**************************************************************************/

#pragma once

#include "variable_expression.h"

namespace HFSM2 {

/**
 * @brief 比较表达式基类
 *
 */
class CompareExpression : public VariableExpression {
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

	CompareExpression(const Ref<Variable> &p_variable, Comparator p_op);
	virtual ~CompareExpression() = default;

	bool compare_with(const Ref<Variable> &p_a, Comparator p_cmp, const Variant &p_b);
	bool compare_with(const Ref<Variable> &p_a, Comparator p_cmp, const Variable *p_b);
};

/**
 * @brief 与常量比较的表达式
 *
 */
class ConstantCompareExpression : public CompareExpression {
public:
	ConstantCompareExpression(const Ref<Variable> &p_variable, Comparator p_op, const Variant &p_value);
	bool get_result(bool p_and_mode, bool &r_result) override;

private:
	Variant value;
};
/**
 * @brief 与变量比较的表达式
 *
 */
class VariableCompareExpression : public CompareExpression {
public:
	VariableCompareExpression(const Ref<Variable> &p_variable, Comparator p_op, const Ref<Variable> &p_value);

	bool get_result(bool p_and_mode, bool &r_result) override;

private:
	Ref<Variable> value;
};

#pragma region 内联实现
inline bool CompareExpression::compare_with(const Ref<Variable> &p_a, Comparator p_cmp, const Variant &p_b) {
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

inline bool CompareExpression::compare_with(const Ref<Variable> &p_a, Comparator p_cmp, const Variable *p_b) {
	return compare_with(p_a, p_cmp, p_b->get_value());
}

#pragma endregion
} // namespace HFSM2
