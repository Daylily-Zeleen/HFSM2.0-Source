/**************************************************************************/
/*  variable_expression.h                                                 */
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

#include "../../variable.h"

namespace Hfsm {

class VariableExpression {
protected:
	VariableExpression(const Ref<Variable> &p_variable) { variable = p_variable; }

public:
	enum ExpressionType {
		EXPRESSION_TYPE_NORMAL,
		EXPRESSION_TYPE_UNION_TRIGGER,
		EXPRESSION_TYPE_SOLO_TRIGGER,
	};

	/**
	 * @brief 判断是否可以推进
	 *
	 * @param and_mode 与其他表达式是否为 与逻辑
	 * @param r_result 返回当前的判定结果是否为真
	 * @return true 已得到总结果
	 * @return false 未能确定总结果
	 */
	virtual bool get_result(bool p_and_mode, bool &r_result) = 0;

	/**
	 * @brief Get the expression typ object
	 *
	 * @return uint8_t 0 通用 1 联合触发器 2 SOLO 触发器
	 */
	virtual ExpressionType get_expression_type() { return ExpressionType::EXPRESSION_TYPE_NORMAL; }

protected:
	// 变量
	Ref<Variable> variable;
};

} // namespace Hfsm
