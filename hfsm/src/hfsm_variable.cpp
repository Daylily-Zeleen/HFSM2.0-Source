#include "hfsm_variable.h"

namespace Hfsm {

#pragma region HFSMVariable
void HFSMVariable::_bind_methods() {
	GDBIND_BEGIN(HFSMVariable);

	GDADD_PROPERTY(NIL, value);

	GDBIND_METHOD(get_variable_name);
	GDBIND_METHOD(get_type);
	GDBIND_METHOD(is_trigger);
	GDBIND_METHOD(flush_trigger);
}

#pragma endregion

} // namespace Hfsm
