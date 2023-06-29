#pragma once

#ifdef GDEXTENSION_BUILD
#include <core/hfsm_variable_res.hpp>
#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/core/binder_common.hpp>

using namespace godot;
#else
#include "../../hfsm_variable_res.h"
#include <core/io/resource.h>

#endif // GDEXTENSION_BUILD

namespace Hfsm {

class HFSM;
class VariableExpression;

class VariableExpressionRes : public Resource {
	GDCLASS(VariableExpressionRes, Resource)

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
	enum Op {
		OP_EQUAL,
		OP_NOT_EQUAL,
		OP_GREATER,
		OP_GREATER_EQUAL,
		OP_LESS,
		OP_LESS_EQUAL,
	};

	void set_variable_res(const Ref<HFSMVariableRes> &p_variable_res);
	Ref<HFSMVariableRes> get_variable_res() const;

	void set_value(const Variant &p_value);
	Variant get_value() const;

	void set_comparator(int64_t p_op);
	uint8_t get_comparator() const;

	void set_trigger_type(int64_t p_trigger_type);
	uint8_t get_trigger_type() const;

	void set_variable_as_value(bool p_variable_as_value);
	bool is_variable_as_value() const;

	// Dictionary get_valid_and_text();

	// Array get_property_list() const;

	VariableExpression *create_variable_expression(HFSM *p_hfsm);

private:
	Ref<HFSMVariableRes> variable_res;
	Variant value;
	uint8_t comparator = OP_EQUAL;
	// trigger
	uint8_t trigger_type = TRIGGER_TYPE_SOLO;

	// 是否使用另一个 变量资源作为 比较值
	bool variable_as_value = false;
};

} // namespace Hfsm

VARIANT_ENUM_CAST(Hfsm::VariableExpressionRes::TriggerType);
VARIANT_ENUM_CAST(Hfsm::VariableExpressionRes::Op);
