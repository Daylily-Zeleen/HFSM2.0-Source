#include "hfsm_variable.hpp"

#include <godot_cpp/variant/utility_functions.hpp>

namespace Hfsm {

#pragma region HFSMVariable
void HFSMVariable::_bind_methods() {
	GDBIND_BEGIN(HFSMVariable);

	GDADD_PROPERTY(NIL, value);

	GDBIND_METHOD(get_type);
	GDBIND_METHOD(flush_trigger);
}

#pragma endregion

} // namespace Hfsm
