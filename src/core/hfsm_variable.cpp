#include "hfsm_variable.hpp"

#include <godot_cpp/variant/utility_functions.hpp>

namespace Hfsm {

#pragma region HFSMVariable
void HFSMVariable::_bind_methods() {
    ClassDB::bind_method(D_METHOD("get_value"), &HFSMVariable::get_value);
    ClassDB::bind_method(D_METHOD("set_value", "value"), &HFSMVariable::set_value);
    ADD_PROPERTY(PropertyInfo(Variant::NIL, "value"), "set_value", "get_value");

    ClassDB::bind_method(D_METHOD("get_type"), &HFSMVariable::get_type_int);
    ClassDB::bind_method(D_METHOD("trigger"), &HFSMVariable::trigger);
    ClassDB::bind_method(D_METHOD("flush_trigger"), &HFSMVariable::flush_trigger);
}

HFSMVariable::HFSMVariable(){
    UtilityFunctions::print("==HFSMVariable");
}



#pragma endregion




} // namespace Hfsm
