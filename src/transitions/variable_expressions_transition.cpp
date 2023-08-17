/**************************************************************************/
/*  variable_expressions_transition.cpp                                   */
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

#include "variable_expressions_transition.h"

namespace Hfsm {

#pragma region VariableExpressionsTransition

VariableExpressionsTransition::~VariableExpressionsTransition() {
	for (auto &&e : solo_triggers) {
		memdelete(e);
	}
	for (auto &&e : union_triggers) {
		memdelete(e);
	}
	for (auto &&e : normal_expressions) {
		memdelete(e);
	}
}

bool VariableExpressionsTransition::can_transit() {
	auto ret = false;
	// 独立触发器只有或逻辑
	for (auto &&st : solo_triggers) {
		st->get_result(and_mode, ret);
		if (ret) {
			return ret;
		}
	}
	// 联合触发器只有 and 逻辑
	for (auto &&ut : union_triggers) {
		if (ut->get_result(and_mode, ret)) {
			break;
		}
	}
	if (ret) {
		return ret;
	}
	// 通用表达式如果不单独跳出说明本身结果不能判定总结果
	// 只有其他结果和自身结果相同时才会持续迭代
	for (auto &&e : normal_expressions) {
		if (e->get_result(and_mode, ret)) {
			return ret;
		}
	}
	return ret;
}

#pragma endregion

} // namespace Hfsm
