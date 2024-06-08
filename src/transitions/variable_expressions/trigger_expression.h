/**************************************************************************/
/*  trigger_expression.h                                                  */
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
 * @brief 普通触发器
 * 与其他类型的变量表达式一起考虑
 */
class TriggerExpression : public VariableExpression {
public:
	TriggerExpression(const Ref<Variable> &p_variable);
	virtual ~TriggerExpression() = default;

	bool get_result(bool p_and_mode, bool &r_result) override;
};

/**
 * @brief 独立触发器
 * 只考虑自己是否触发
 */
class SoloTriggerExpression : public TriggerExpression {
public:
	SoloTriggerExpression(const Ref<Variable> &p_variable);
	//  独立触发器只关注自己
	bool get_result(bool p_and_mode, bool &r_result) override;
	ExpressionType get_expression_type() override;
};

/**
 * @brief 联合触发器
 * 同时考虑其他 联合触发器（在上层检查时分离）
 */
class UnionTriggerExpression : public TriggerExpression {
public:
	UnionTriggerExpression(const Ref<Variable> &p_variable);

	bool get_result(bool p_and_mode, bool &r_result) override;
	ExpressionType get_expression_type() override;
};

} // namespace HFSM2
