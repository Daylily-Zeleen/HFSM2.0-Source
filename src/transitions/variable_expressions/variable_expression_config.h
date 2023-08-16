#pragma once

#ifdef GDEXTENSION_BUILD
#include <godot_cpp/classes/resource.hpp>

using namespace godot;
#else // GDEXTENSION_BUILD
#include <core/io/resource.h>

#endif // GDEXTENSION_BUILD

#include "../../variable_config.h"

#include "comparation_expression.h"

namespace Hfsm {

class HFSM;
class VariableExpression;

class VariableExpressionConfig : public Resource {
	GDCLASS(VariableExpressionConfig, Resource)

protected:
	static void _bind_methods();

	_TO_STRING()
public:
	bool _set(const StringName &p_name, const Variant &p_property);
	bool _get(const StringName &p_name, Variant &r_property) const;
	void _get_property_list(List<PropertyInfo> *p_list) const;

	enum TriggerType {
		TRIGGER_TYPE_SOLO,
		TRIGGER_TYPE_UNION,
		TRIGGER_TYPE_NORMAL,
		TRIGGER_TYPE_MAX,
	};

	using Comparator = ComparationExpression::Comparator;

	void set_variable_config(const Ref<VariableConfig> &p_variable_config);
	Ref<VariableConfig> get_variable_config() const;

	void set_value(const Variant &p_value);
	Variant get_value() const;

	void set_comparator(Comparator p_cmp);
	Comparator get_comparator() const;

	void set_trigger_type(TriggerType p_trigger_type);
	TriggerType get_trigger_type() const;

	void set_variable_as_value(bool p_variable_as_value);
	bool is_variable_as_value() const;

	VariableExpression *create_variable_expression(HFSM *p_hfsm);

private:
	Ref<VariableConfig> variable_config;
	Variant value;
	Comparator comparator = Comparator::COMPARATOR_EQUAL;
	// trigger
	TriggerType trigger_type = TRIGGER_TYPE_SOLO;

	// 是否使用另一个 变量资源作为 比较值
	bool variable_as_value = false;
};

} // namespace Hfsm

VARIANT_ENUM_CAST(Hfsm::VariableExpressionConfig::TriggerType);
VARIANT_ENUM_CAST(Hfsm::VariableExpressionConfig::Comparator);
