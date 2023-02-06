#pragma once

#include "../hfsm_global.hpp"

#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/ref_counted.hpp>

using namespace godot;
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
	HFSMVariable();

	Variant get_value() const;
	void set_value(Variant value);

	Variant::Type get_type() const;
	int64_t get_type_int() const;
	// 触发器专用
	void trigger();
	void flush_trigger();

	// 无需暴露
	bool compare_with(const Variant &val, uint8_t op);
	bool compare_with(const HFSMVariable *other, uint8_t op);

private:
	StringName _name = "";
	Variant::Type _type = Variant::NIL;
	Variant _value;

	friend class HFSMVariableRes;
};

#pragma region 内联实现

inline Variant HFSMVariable::get_value() const { return _value; }
inline void HFSMVariable::set_value(Variant value) {
	// 触发器特殊处理
	if (_type == Variant::NIL) {
		_value = Variant(true);
		return;
	}
	if (!Engine::get_singleton()->is_editor_hint()) {
		CRASH_COND(!Variant::can_convert(value.get_type(), _type));
	}
	_value = value;
	notify_property_list_changed();
}
inline void HFSMVariable::trigger() {
	ERR_FAIL_COND(_type != Variant::NIL);
	_value = Variant(true);
}

inline Variant::Type HFSMVariable::get_type() const { return _type; }
inline int64_t HFSMVariable::get_type_int() const { return _type; }
// 触发器专用
inline void HFSMVariable::flush_trigger() { _value = false; }
inline bool HFSMVariable::compare_with(const Variant &val, uint8_t op) {
	switch (op) {
		case OP_EQUAL:
			return get_value().operator==(val);
		case OP_NOT_EQUAL:
			return get_value().operator!=(val);
		case OP_GREATER:
			return !get_value().operator<(val);
		case OP_GREATER_EQUAL:
			return (get_value().operator==(val) || !get_value().operator<(val));
		case OP_LESS:
			return get_value().operator<(val);
		case OP_LESS_EQUAL:
			return (get_value().operator==(val) || get_value().operator<(val));

		default:
			return false;
	}
}
inline bool HFSMVariable::compare_with(const HFSMVariable *other, uint8_t op) {
	return compare_with(other->get_value(), op);
}

#pragma endregion

}; // namespace Hfsm
