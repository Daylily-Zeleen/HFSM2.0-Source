#pragma once

#include "../hfsm_global.h"

#ifdef GDEXTENSION_BUILD
#ifdef TOOLS_ENABLED
#include <godot_cpp/classes/engine.hpp>
#endif // TOOLS_ENABLED
#include <godot_cpp/classes/ref_counted.hpp>
using namespace godot;
#else
#ifdef TOOLS_ENABLED
#include <core/config/engine.h>
#endif // TOOLS_ENABLED
#include <core/object/ref_counted.h>

#endif // GDEXTENSION_BUILD

namespace Hfsm {
enum Op {
	OP_EQUAL,
	OP_NOT_EQUAL,
	OP_GREATER,
	OP_GREATER_EQUAL,
	OP_LESS,
	OP_LESS_EQUAL,
};
// 变量类
class HFSMVariable : public RefCounted {
	GDCLASS(HFSMVariable, RefCounted)
protected:
	static void _bind_methods();

public:
	StringName get_variable_name() const;

	Variant::Type get_type() const;

	Variant get_value() const;
	void set_value(const Variant &p_value);

	bool is_trigger() const;
	// 触发器专用
	void trigger();
	void flush_trigger();

	// 无需暴露
	bool compare_with(const Variant &p_val, uint8_t p_op);
	bool compare_with(const HFSMVariable *p_other, uint8_t p_op);

private:
	StringName variable_name = "";
	Variant::Type type = Variant::NIL;
	Variant value;

	friend class HFSMVariableRes;
};

#pragma region 内联实现
inline StringName HFSMVariable::get_variable_name() const { return variable_name; }

inline Variant HFSMVariable::get_value() const { return value; }
inline void HFSMVariable::set_value(const Variant &p_value) {
	// 触发器特殊处理
	if (type == Variant::NIL) {
		value = Variant(true);
		return;
	}

#ifdef TOOLS_ENABLED
	if (!Engine::get_singleton()->is_editor_hint()) {
		CRASH_COND(!Variant::can_convert(p_value.get_type(), type));
	}
#endif // TOOLS_ENABLED
	value = p_value;
}

inline bool HFSMVariable::is_trigger() const { return get_type() == Variant::NIL; }
inline void HFSMVariable::trigger() {
	ERR_FAIL_COND(type != Variant::NIL);
	value = Variant(true);
}

inline Variant::Type HFSMVariable::get_type() const { return type; }

// 触发器专用
inline void HFSMVariable::flush_trigger() { value = false; }
inline bool HFSMVariable::compare_with(const Variant &val, uint8_t op) {
	switch (op) {
		case OP_EQUAL:
			return get_value() == val;
		case OP_NOT_EQUAL:
			return get_value() != val;
		case OP_GREATER:
			return !(get_value() < val || get_value() == val);
		case OP_GREATER_EQUAL:
			return !(get_value() < val);
		case OP_LESS:
			return get_value() < val;
		case OP_LESS_EQUAL:
			return get_value() < val || get_value() == val;

		default:
			return false;
	}
}
inline bool HFSMVariable::compare_with(const HFSMVariable *other, uint8_t op) {
	return compare_with(other->get_value(), op);
}

#pragma endregion
}; // namespace Hfsm
