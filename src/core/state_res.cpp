#include "state_res.hpp"
#include "editor/hfsm_editor_plugin.hpp"
#include "fsm.hpp"
#include "fsm_res.hpp"
#include "hfsm.hpp"

namespace Hfsm {

#pragma region StateRes
StateRes::StateRes() = default;
StateRes::~StateRes() = default;

bool StateRes::_set(const StringName &p_name, const Variant &p_property) {
	if (p_name == StringName("state_node") && Object::cast_to<Node>(p_property)) {
		set_meta({ "state_node" }, Object::cast_to<Node>(p_property));
		return true;
	} else if (p_name == StringName{ "animation_name" }) {
		set_animation_name(p_property);
		return true;
	}
#ifdef FULL_VERSION
	else if (p_name == StringName{ "animation_blend_time" }) {
		set_animation_blend_time(p_property);
		return true;
	} else if (p_name == StringName{ "animation_speed" }) {
		set_animation_speed(p_property);
		return true;
	} else if (p_name == StringName{ "animation_reverse" }) {
		set_animation_reverse(p_property);
		return true;
	}
#endif
	return false;
}

Node *StateRes::get_state_node() const {
	if (has_meta({ "state_node" })) {
		auto meta = get_meta({ "state_node" });
		if (UtilityFunctions::is_instance_valid(meta)) {
			return cast_to<Node>(meta);
		}
	}
	return nullptr;
}

bool StateRes::_get(const StringName &p_name, Variant &r_property) const {
	if (p_name == StringName("state_node")) {
		r_property = get_state_node();
		return true;
	} else if (p_name == StringName{ "animation_name" }) {
		r_property = get_animation_name();
		return true;
	}
#ifdef FULL_VERSION
	else if (p_name == StringName{ "animation_blend_time" }) {
		r_property = get_animation_blend_time();
		return true;
	} else if (p_name == StringName{ "animation_speed" }) {
		r_property = get_animation_speed();
		return true;
	} else if (p_name == StringName{ "animation_reverse" }) {
		r_property = get_animation_reverse();
		return true;
	}
#endif
	return false;
}
void StateRes::_get_property_list(List<PropertyInfo> *p_list) const {
	p_list->push_back(PropertyInfo(Variant::OBJECT, "state_node", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_EDITOR));

	p_list->push_back(PropertyInfo(Variant::NIL, "animation", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_GROUP));
	String animations;
	if (HfsmEditorPlugin::get_singleton()) {
		if (HfsmEditorPlugin::get_singleton()->get_hfsm_editor()) {
			if (HfsmEditorPlugin::get_singleton()->get_hfsm_editor()->get_editing_hfsm()) {
				if (HfsmEditorPlugin::get_singleton()->get_hfsm_editor()->get_editing_hfsm()->get_animation_player()) {
					for (auto &&anim : HfsmEditorPlugin::get_singleton()->get_hfsm_editor()->get_editing_hfsm()->get_animation_player()->get_animation_list()) {
						animations += "," + anim;
					}
				}
			}
		}
	}
	p_list->push_back(PropertyInfo(Variant::STRING_NAME, "animation_name", PROPERTY_HINT_ENUM_SUGGESTION, animations));

#ifdef FULL_VERSION
	p_list->push_back(PropertyInfo(Variant::FLOAT, "animation_blend_time"));
	p_list->push_back(PropertyInfo(Variant::FLOAT, "animation_speed"));
	p_list->push_back(PropertyInfo(Variant::BOOL, "animation_reverse"));
#endif
}

void StateRes::_bind_methods() {
	GDBIND_BEGIN(StateRes);
	GDADD_PROPERTY(STRING, state_name);
	GDADD_PROPERTY(INT, type);
	GDADD_PROPERTY_RESOURCE(state_script);
	GDADD_PROPERTY_BOOL(nested);
	GDADD_PROPERTY_RESOURCE(fsm_res);
	GDADD_PROPERTY(VECTOR2, editor_offset);
	GDADD_PROPERTY_BOOL(reset_properties_when_entry);
	GDADD_PROPERTY_BOOL(reset_nested_fsm_when_entry);
	GDADD_PROPERTY(VECTOR2, size_in_editor);
}

void StateRes::set_state_name(const StringName &p_name) {
	state_name = p_name;
	emit_changed();
	// notify_property_list_changed();
}
StringName StateRes::get_state_name() const { return state_name; }

void StateRes::set_type(State::StateType p_state_type) {
	type = p_state_type;
	emit_changed();
	// notify_property_list_changed();
}
State::StateType StateRes::get_type() const { return type; }
void StateRes::set_state_script(const Ref<Script> &p_script) {
	state_script = p_script;
	emit_changed();
	// notify_property_list_changed();
}
Ref<Script> StateRes::get_state_script() const { return state_script; }

void StateRes::set_nested(bool p_nested) {
	nested = p_nested;
	emit_changed();
	// notify_property_list_changed();
}
bool StateRes::is_nested() const { return nested; }

void StateRes::set_fsm_res(const Ref<FsmRes> &p_fsm_res) {
	fsm_res = p_fsm_res;
	emit_changed();
	// notify_property_list_changed();
}
Ref<FsmRes> StateRes::get_fsm_res() const { return fsm_res; }

void StateRes::set_editor_offset(Vector2 p_offset) {
	editor_offset = p_offset;
	emit_changed();
	// notify_property_list_changed();
}
Vector2 StateRes::get_editor_offset() const { return editor_offset; }
void StateRes::set_reset_properties_when_entry(bool p_v) {
	reset_properties_when_entry = p_v;
	emit_changed();
	// notify_property_list_changed();
}
bool StateRes::is_reset_properties_when_entry() const { return reset_properties_when_entry; }
void StateRes::set_reset_nested_fsm_when_entry(bool p_v) {
	reset_nested_fsm_when_entry = p_v;
	emit_changed();
	// notify_property_list_changed();
}
bool StateRes::is_reset_nested_fsm_when_entry() const { return reset_nested_fsm_when_entry; }

StringName StateRes::get_animation_name() const { return animation_name; }
void StateRes::set_animation_name(const StringName &p_anim_name) { animation_name = p_anim_name; }
#ifdef FULL_VERSION
double StateRes::get_animation_blend_time() const { return animation_blend_time; }
void StateRes::set_animation_blend_time(double p_blend_time) { animation_blend_time = p_blend_time; }
double StateRes::get_animation_speed() const { return animation_speed; }
void StateRes::set_animation_speed(double p_speed) { animation_speed = p_speed; }
bool StateRes::get_animation_reverse() const { return animation_reverse; }
void StateRes::set_animation_reverse(bool p_reverse) { animation_reverse = p_reverse; }
#endif

//
Ref<RefCounted> StateRes::create_state(HFSM *p_hfsm, const Fsm *p_fsm) {
	static const StringName state_class_name = "State";
	Ref<State> r;
	r.instantiate();
	r->set_name(state_name);
	r->hfsm = p_hfsm;
	r->type = type;
	r->reset_when_entry = reset_properties_when_entry;
	r->reset_nested_fsm_when_entry = reset_nested_fsm_when_entry;
	// TODO:: 动画

	// 路径
	// State 一定包含于 Fsm
	r->path.append_array(p_fsm->get_path());

	// 脚本处理
	// auto script = script;
	// 校验脚本是否合法
	if (state_script.is_valid()) {
		if (state_script->get_instance_base_type() == state_class_name) {
			r->set_script(state_script);
			if (state_script.is_valid()) {
				// 拾取所有属性
				auto properties = state_script->get_script_property_list();
				auto base_script = state_script->get_base_script();
				while (base_script != nullptr) {
					properties.append_array(base_script->get_script_property_list());
					base_script = base_script->get_base_script();
				}
				// 记录默认值
				for (uint32_t i = 0; i < properties.size(); i++) {
					Dictionary p = static_cast<Dictionary>(properties[i]);
					String p_name = p[Variant("name")];
					if (p_name != "nested_state" && !p_hfsm->get_agents().keys().has(p_name)) {
						// 非代理节点也非内嵌状态对象时，确定为用户自定义属性
						r->property_to_defatul_value.insert(p_name, r->get(p_name));
					}
				}
			}
		} else {
			String path_text;
			for (auto i = 0; i < r->get_path().size(); i++) {
				path_text = path_text + Ref<State>(r->get_path()[i])->get_name();
				if (i != i < r->get_path().size() - 1) {
					path_text = path_text + String("/");
				}
			}
			ERR_FAIL_V_MSG(r, path_text + String(": Script is not extends from 'State'."));
		}
	}
	// 内嵌状态机
	// UtilityFunctions::print(get_name(), " ",_nested);
	if (fsm_res.is_valid()) {
		r->fsm = static_cast<FsmRes *>(get_fsm_res().ptr())->create_fsm(p_hfsm, r, p_fsm->fsm_update_queue);
	}
	return r;
}

#pragma endregion

}; // namespace Hfsm