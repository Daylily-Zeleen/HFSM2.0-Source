#pragma once

#include <godot_cpp/classes/resource.hpp>
#include <hfsm_global.hpp>

using namespace godot;

namespace Hfsm {
class HFSMVariable;
class FsmRes;

class HFSMVariableRes : public Resource {
	GDCLASS(HFSMVariableRes, Resource)

protected:
	static void _bind_methods();

	_TO_STRING()

public:
	bool _set(const StringName &p_name, const Variant &p_property);
	bool _get(const StringName &p_name, Variant &r_property) const;
	void _get_property_list(List<PropertyInfo> *p_list) const;

	void set_variable_name(const StringName &p_name);
	StringName get_variable_name();

	void set_type(Variant::Type p_t);
	Variant::Type get_type() const;

	void set_comment(const String &p_comment);
	String get_comment() const;

	void set_deleted(bool p_d);
	void set_default_value(const Variant &p_default_val);
	Variant get_default_value(); //  { return _default_val; }

	bool is_deleted();
	void delete_self();

	Ref<RefCounted> create_variable();

	void set_fsm_res(const Ref<FsmRes> &p_fsm_res);
	Ref<FsmRes> get_fsm_res() const;

	static Ref<HFSMVariableRes> create_new(const Ref<FsmRes> &p_fsm_res);

private:
	StringName variable_name = "variable";
	Ref<FsmRes> fsm_res;
	Variant::Type type = Variant::NIL;
	Variant default_value;
	String comment = "";
	bool deleted = false;
};

} // namespace Hfsm
