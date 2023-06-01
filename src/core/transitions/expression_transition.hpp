#pragma once

#include <godot_cpp/classes/expression.hpp>

#include "transition_base.hpp"

namespace Hfsm {
// 表达式转换
class ExpressionTransition : public TransitionBase {
public:
	bool can_transit() override;

	void set_expression_text(const String &expression_text);

	bool is_vaild() const;

private:
	HFSM *hfsm = nullptr;

	Expression expression;
	// String expression_text = "";
	bool valid = false;

	friend class TransitionRes;
};

#pragma region 内联实现

inline bool ExpressionTransition::is_vaild() const { return valid; }

#pragma endregion
} // namespace Hfsm
