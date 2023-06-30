#include "variable_transition.h"

namespace Hfsm {

#pragma region VariableTransition

VariableTransition::~VariableTransition() {
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

bool VariableTransition::can_transit() {
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
