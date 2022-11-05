#ifndef TRANSITION_H
#define TRANSITION_H

#include "transition_base.hpp"

using namespace godot;

namespace Hfsm {
// class HFSM;
// class State;

class Transition: public RefCounted, public TransitionBase{
    GDCLASS(Transition, RefCounted)
protected:
    static void _bind_methods();

    String _to_string() { return String("[Transition:{0}]").replace("{0}", itos(get_instance_id()));}
public:
    Transition(){}
    // TODO:: call 是否会引发错误？
    // 能否不走 call 调用真正的虚方法？
    void refresh() override{
        const static StringName sn_refresh = StringName("_refresh");
        call(sn_refresh);
    }
    bool can_transit() override{
        const static StringName sn_can_transit = StringName("_can_transit");
        return bool(call(sn_can_transit));
    }
    virtual void _refresh() {}
    virtual bool _can_transit(){return false;}
    

    Ref<State> get_from_state() override{return TransitionBase::get_from_state();}
    Ref<State> get_to_state() override{return TransitionBase::get_to_state();}

    HFSM *get_hfsm(){ return _hfsm; }
    
    Dictionary get_context(){return _hfsm->get_context();}
private:
    HFSM *_hfsm = nullptr;
    
    friend class TransitionRes;
};
}; // namespace Hfsm

#endif