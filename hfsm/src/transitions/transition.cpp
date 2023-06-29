#include "transition.h"

namespace Hfsm {

void Transition::_bind_methods() {
	GDVIRTUAL_BIND(_refresh);
	GDVIRTUAL_BIND(_can_transit);

	GDBIND_BEGIN(Transition);
	GDBIND_METHOD(get_from_state);
	GDBIND_METHOD(get_to_state);
	GDBIND_METHOD(get_hfsm);

	// GDBIND_METHOD(get_context);

#ifdef ROLLBACK_NET_CODE
	BIND_VIRTUAL_METHOD(Transition, _save_state);
	BIND_VIRTUAL_METHOD(Transition, _load_state);
	BIND_VIRTUAL_METHOD(Transition, _interpolate_state);
	BIND_VIRTUAL_METHOD(Transition, _get_local_input);
	BIND_VIRTUAL_METHOD(Transition, _predict_remote_input);
	BIND_VIRTUAL_METHOD(Transition, _network_process);
	BIND_VIRTUAL_METHOD(Transition, _network_preprocess);
	BIND_VIRTUAL_METHOD(Transition, _network_postprocess);
	BIND_VIRTUAL_METHOD(Transition, _network_spawn_preprocess);
	BIND_VIRTUAL_METHOD(Transition, _network_spawn);
	BIND_VIRTUAL_METHOD(Transition, _network_despawn);
#endif
}

}; // namespace Hfsm
