#include "transition.hpp"

namespace Hfsm{

void Transition::_bind_methods() {
    BIND_VIRTUAL_METHOD(Transition, _refresh);
    BIND_VIRTUAL_METHOD(Transition, _can_transit);

    ClassDB::bind_method(D_METHOD("get_from_state"),
                         &Transition::get_from_state);
    ClassDB::bind_method(D_METHOD("get_to_state"), &Transition::get_to_state);
    ClassDB::bind_method(D_METHOD("get_hfsm"), &Transition::get_hfsm);
    ClassDB::bind_method(D_METHOD("get_context"), &Transition::get_context);

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

