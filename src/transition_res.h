#pragma once
#include "../hfsm_global.h"

#ifdef GDEXTENSION_BUILD
#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/classes/script.hpp>
#include <godot_cpp/templates/vector.hpp>
using namespace godot;
#else
#include <core/io/resource.h>
#include <core/object/script_language.h>
#include <core/variant/typed_array.h>

#endif // GDEXTENSION_BUILD

#include "transitions/variable_expressions/variable_expression_res.h"

namespace Hfsm {

class HFSM;
// class VariableExpressionRes;
class TransitionBase;
class StateRes;
// class FsmRes;
// class TransitionRes;

// 基类资源
class TransitionRes : public Resource {
	GDCLASS(TransitionRes, Resource)
	// void __on_variable_expression_res_changed(const Ref<VariableExpressionRes> &p_ver);

protected:
	static void _bind_methods();

	_TO_STRING()

public:
	enum TransitionType : uint8_t {
#ifdef FULL_VERSION
		TRANSITION_TYPE_SCRIPT,
#endif
		TRANSITION_TYPE_VARIABLE,
		TRANSITION_TYPE_EXPRESSION,
		TRANSITION_TYPE_AUTO,
	};

	enum AuotoTtransitMode : uint8_t {
		AUTO_TRANSIT_MODE_ANIMATION_FINISH,
		AUTO_TRANSIT_MODE_DELAY_TIMER,
		AUTO_TRANSIT_MODE_FSM_EXIT,
		AUTO_TRANSIT_MODE_MANUAL,
		AUTO_TRANSIT_MODE_UPDATE_TIMES,
		AUTO_TRANSIT_MODE_PHYSICS_UPDATE_TIMES,
		AUTO_TRANSIT_MODE_MAX,
	};

	TransitionBase *create_transition(HFSM *p_hfsm, Ref<StateRes> &r_from_state_res, Ref<StateRes> &r_to_state_res);

	bool _set(const StringName &p_name, const Variant &p_property);
	bool _get(const StringName &p_name, Variant &r_property) const;
	void _get_property_list(List<PropertyInfo> *p_list) const;

	// 公用访问器
	void set_from_state_res(const Ref<StateRes> &p_from_state_res);
	Ref<StateRes> get_from_state_res() const;
	void set_to_state_res(const Ref<StateRes> &p_from_to_res);
	Ref<StateRes> get_to_state_res() const;
	void set_type(TransitionType p_type);
	TransitionType get_type() const;

	// Auto
	void set_auto_mode(AuotoTtransitMode p_auto_mode);
	AuotoTtransitMode get_auto_mode() const;
	void set_auto_delay_msec(uint64_t p_delay_msec);
	uint64_t get_auto_delay_msec() const;
	void set_auto_times(uint64_t p_times);
	int64_t get_auto_times() const;

	// 表达式
	void set_expression_text(const String &p_expression_text);
	String get_expression_text() const;
	void set_expression_comment(const String &p_expression_comment);
	String get_expression_comment() const;

	// 变量表达式
	void set_variable_and_mode(bool p_and_mode);
	bool is_variable_and_mode() const;
	void set_variable_expression_res_list(const Array &p_variable_expression_res_list);
	TypedArray<VariableExpressionRes> get_variable_expression_res_list() const;

#ifdef FULL_VERSION
	// 脚本
	void set_transition_script(const Ref<Script> &p_transition_script);
	Ref<Script> get_transition_script() const;
	bool is_script_valid() const;
#endif // FULL_VERSION

private:
	// 共有属性
	Ref<StateRes> from_state_res;
	Ref<StateRes> to_state_res;
	TransitionType type = TRANSITION_TYPE_AUTO;

	// Auto
	AuotoTtransitMode auto_mode = AUTO_TRANSIT_MODE_DELAY_TIMER;
	uint64_t auto_delay_msec = 1000; // 延迟时间
	uint64_t auto_times = 5; // 重复次数

	// 表达式
	String expression_text = "";
	String expression_comment = "";

#ifdef FULL_VERSION
	// 脚本
	Ref<Script> transition_script;
	bool script_valid = true;
#endif

	// 变量表达式
	bool variable_and_mode = true;
	TypedArray<VariableExpressionRes> variable_expression_res_list; // VariableExpressionRes
};

}; // namespace Hfsm

VARIANT_ENUM_CAST(Hfsm::TransitionRes::TransitionType);
VARIANT_ENUM_CAST(Hfsm::TransitionRes::AuotoTtransitMode);
