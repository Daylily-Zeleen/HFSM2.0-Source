#include "trigger_expression.h"

#ifdef TOOLS_ENABLED
#ifdef GDEXTENSION_BUILD
#include <godot_cpp/classes/engine.hpp>
#else
#include <core/config/engine.h>
#endif // GDEXTENSION_BUILD
#endif // TOOLS_ENABLED

namespace Hfsm {

#pragma region TriggerExpression
TriggerExpression::TriggerExpression(const Ref<HFSMVariable> &variable) :
		VariableExpression(variable) {
#ifdef TOOLS_ENABLED
	if (!Engine::get_singleton()->is_editor_hint()) {
		CRASH_COND(variable->get_type() != Variant::NIL);
	}
#endif // TOOLS_ENABLED
}

#pragma endregion

#pragma region SoloTriggerExpression
SoloTriggerExpression::SoloTriggerExpression(const Ref<HFSMVariable> &variable) :
		TriggerExpression(variable) {}

#pragma endregion

#pragma region UnionTrigger
UnionTriggerExpression::UnionTriggerExpression(const Ref<HFSMVariable> &variable) :
		TriggerExpression(variable) {}

#pragma endregion

} // namespace Hfsm
