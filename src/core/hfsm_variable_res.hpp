#ifndef VARIABLE_RES_H
#define VARIABLE_RES_H

#include <godot_cpp/classes/resource.hpp>



using namespace godot;

namespace Hfsm {
class HFSMVariable;
class FsmRes;

class HFSMVariableRes : public Resource {
    GDCLASS(HFSMVariableRes, Resource)

protected:
    static void _bind_methods();
    
    String _to_string() const { return String("[HFSMVariableRes:{0}]").replace("{0}", itos(get_instance_id()));}
public:
    HFSMVariableRes();

    bool _set(const StringName &p_name, const Variant &p_property) ;
    bool _get(const StringName &p_name, Variant &r_property) const ;
	void _get_property_list(List<PropertyInfo> *p_list) const ;
    
    void set_name(const StringName &name);
    StringName get_name();

    void set_type(int32_t t);
    int32_t get_type() const;

    void set_comment(const String &comment);
    String get_comment() const;

    void set_deleted(bool d);
    void set_default_val(Variant default_val);
    Variant get_default_val(); //  { return _default_val; }

    bool is_deleted();
    void delete_self();

    Ref<RefCounted> create_variable();


    void set_fsm_res(Ref<FsmRes> fsm_res);
    Ref<FsmRes> get_fsm_res() const;

    static Ref<HFSMVariableRes> create_new(Ref<FsmRes> fsm_res);

private:
    StringName _name = "variable";
    Ref<FsmRes> _fsm_res;
    Variant::Type _type = Variant::NIL;
    Variant _default_val;
    String _comment = "";
    bool _deleted = false;

};

} // namespace Hfsm

#endif