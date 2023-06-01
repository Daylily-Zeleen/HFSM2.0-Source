#include "hfsm_variable.hpp"

#include <godot_cpp/variant/utility_functions.hpp>

namespace Hfsm {

#pragma region HFSMVariable
void HFSMVariable::_bind_methods() {
	GDBIND_BEGIN(HFSMVariable);

	GDADD_PROPERTY(NIL, value);
	// ClassDB::bind_method(D_METHOD("get_type"), &HFSMVariable::get_type_int);

	GDBIND_METHOD(get_type);
	GDBIND_METHOD(flush_trigger);
}

HFSMVariable::HFSMVariable() {
	UtilityFunctions::print("==HFSMVariable");
}

#pragma endregion

} // namespace Hfsm
