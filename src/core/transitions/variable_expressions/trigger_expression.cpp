#include "trigger_expression.hpp"
#include "../../../hfsm_global.hpp"

#include <godot_cpp/classes/engine.hpp>
namespace Hfsm
{
    

#pragma region TriggerExpression
TriggerExpression::TriggerExpression(const Ref<HFSMVariable> &variable)
    : VariableExpression(variable) {
    if (!Engine::get_singleton()->is_editor_hint()){
        CRASH_COND(variable->get_type() != Variant::NIL);
    }
}


#pragma endregion

#pragma region SoloTriggerExpression
SoloTriggerExpression::SoloTriggerExpression(const Ref<HFSMVariable> &variable)
    : TriggerExpression(variable) {}


#pragma endregion

#pragma region UnionTrigger
UnionTriggerExpression::UnionTriggerExpression(const Ref<HFSMVariable> &variable)
    : TriggerExpression(variable) {}


#pragma endregion

    
} // namespace Hfsm



