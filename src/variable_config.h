#pragma once

#include "../hfsm_global.h"

#ifdef GDEXTENSION_BUILD
#include <godot_cpp/classes/resource.hpp>
using namespace godot;
#else
#include <core/io/resource.h>
#endif // GDEXTENSION_BUILD

namespace Hfsm {
class Variable;
// class FSMConfig;

class VariableConfig : public Resource {
	GDCLASS(VariableConfig, Resource)

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

	void set_default_value(const Variant &p_default_val);
	Variant get_default_value() const; //  { return _default_val; }

	Ref<class Variable> create_variable();

	void set_fsm_config(const Ref<class FSMConfig> &p_fsm_config);
	Ref<class FSMConfig> get_fsm_config() const;

	static Ref<VariableConfig> create_new(const Ref<class FSMConfig> &p_fsm_config);

	String get_type_text() const;

private:
	StringName variable_name = "variable";
	Ref<class FSMConfig> fsm_config;
	Variant::Type type = Variant::NIL;
	Variant default_value;
	String comment = "";
};

} // namespace Hfsm
