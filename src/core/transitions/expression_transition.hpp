#pragma once

#include <godot_cpp/classes/expression.hpp>

#include "transition_base.hpp"

namespace Hfsm {
// 表达式转换
class ExpressionTransition : public TransitionBase {
public:
	bool can_transit() override;

	void set_expression_text(const String &expression_text);

	bool is_vaild_expression() const;

private:
	HFSM *_hfsm = nullptr;

	Expression _expression;
	// String _expression_text = "";
	bool _valid = false;

	friend class TransitionRes;
};

#pragma region 内联实现

inline bool ExpressionTransition::is_vaild_expression() const { return _valid; }

#pragma endregion
} // namespace Hfsm
