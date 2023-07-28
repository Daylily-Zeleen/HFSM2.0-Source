#include "hfsm_editor.h"

#ifdef GDEXTENSION_BUILD
#include <godot_cpp/classes/button.hpp>
#include <godot_cpp/classes/editor_inspector.hpp>
#include <godot_cpp/classes/editor_interface.hpp>
#include <godot_cpp/classes/margin_container.hpp>
#include <godot_cpp/classes/panel_container.hpp>
#include <godot_cpp/classes/v_box_container.hpp>

#else
#include <editor/editor_interface.h>
#include <scene/gui/panel.h>
#include <scene/gui/panel_container.h>

#endif // GDEXTENSION_BUILD

#include "../src/fsm_config.h"
#include "../src/hfsm.h"
#include "../src/state_config.h"

#include "fsm_editor.h"
#include "hfsm_editor_plugin.h"

namespace Hfsm {

void HFSMEditor::initialize() {
	set_custom_minimum_size(Vector2(0, 200));

	set_anchors_preset(LayoutPreset::PRESET_FULL_RECT);

	auto vbox = memnew(VBoxContainer);
	add_child(vbox);
	vbox->set_anchors_preset(LayoutPreset::PRESET_FULL_RECT);
	auto up_panel_container = memnew(PanelContainer);
	vbox->add_child(up_panel_container);
	auto up_margin_contianer = memnew(MarginContainer);

	up_panel_container->add_child(up_margin_contianer);
	path_button_container = memnew(HBoxContainer);
	auto up_label = memnew(Label);
	up_label->set_text(HfsmEditorPlugin::str_localize("Path: "));
	path_button_container->add_child(up_label);
	up_panel_container->add_child(path_button_container);

	//
	fsm_editor = FsmEditor::create_fsm_editor(path_button_container, debug_mode);
	fsm_editor->connect("_edit_fsm_requested", TCALLABLE(_edit_fsm_requested));
	vbox->add_child(fsm_editor);

	auto botton_h_box = memnew(HBoxContainer);
	vbox->add_child(botton_h_box);
	hint_label = memnew(Label);
	hint_label->set_h_size_flags(SizeFlags::SIZE_EXPAND_FILL);
	botton_h_box->add_child(hint_label);

	history_label = memnew(Label);
	history_label->set_h_size_flags(SizeFlags::SIZE_SHRINK_END);
	botton_h_box->add_child(history_label);
	if (debug_mode) {
		botton_h_box->hide();
	}

	//
	mask_panel = memnew(Panel);
	mask_panel->set_anchors_preset(LayoutPreset::PRESET_FULL_RECT);
	mask_panel->set_self_modulate(Color(0, 0, 0, 0.6));
	add_child(mask_panel);

	not_hfsm_label = memnew(Label);
	not_hfsm_label->set_text(HfsmEditorPlugin::str_localize("Plese select a 'HFSM' node to start edit."));
	not_hfsm_label->set_vertical_alignment(VERTICAL_ALIGNMENT_CENTER);
	not_hfsm_label->set_horizontal_alignment(HORIZONTAL_ALIGNMENT_CENTER);
	not_hfsm_label->set_anchors_preset(LayoutPreset::PRESET_FULL_RECT);
	mask_panel->add_child(not_hfsm_label);

	mask_panel->hide();
}

bool HFSMEditor::try_set_nested_state_config_for_fsm_config_recursively(const Ref<FSMConfig> &p_fsm_config, Ref<FSMConfig> p_to_search_fsm_config) {
	if (debug_mode) {
		return false;
	}

	ERR_FAIL_COND_V(p_fsm_config.is_null(), false);

	if (p_to_search_fsm_config.is_null()) {
		p_to_search_fsm_config = get_editing_hfsm()->get_root_fsm_config();
	}

	ERR_FAIL_COND_V(p_to_search_fsm_config.is_null(), false);

	if (p_fsm_config == p_to_search_fsm_config) {
		return true;
	}

	auto sc_list = p_to_search_fsm_config->get_state_config_list();
	for (size_t i = 0; i < sc_list.size(); i++) {
		Ref<StateConfig> sc = sc_list[i];
		if (sc->get_fsm_config().is_valid()) {
			if (sc->get_fsm_config() == p_fsm_config) {
				p_fsm_config->set_nested_state_config(sc);
				return true;
			} else {
				if (try_set_nested_state_config_for_fsm_config_recursively(p_fsm_config, sc->get_fsm_config())) {
					return true;
				}
			}
		}
	}
	return false;
}

void HFSMEditor::edit_fsm_config_in_hfsm(const Ref<FSMConfig> &p_fsm_config, const Ref<FSMConfig> &p_root_fsm_config) {
	if (!debug_mode && p_fsm_config->get_nested_state_config().is_null() && get_editing_hfsm()) {
		try_set_nested_state_config_for_fsm_config_recursively(p_fsm_config);
	}

	if (p_root_fsm_config.is_valid()) {
		// 调试器开始编辑才会带上该参数
		editing_root_fsm_config = p_root_fsm_config;
	}
	ERR_FAIL_COND(editing_root_fsm_config.is_null());

	fsm_editor->edit_fsm_config(p_fsm_config, path_button_container, editing_root_fsm_config);
}

void HFSMEditor::debug_highlight_activate_state(const PackedStringArray &p_active_path) {
	int idx = 0;
	for (auto i = 0; i < path_button_container->get_child_count(); ++i) {
		if (auto btn = cast_to<Button>(path_button_container->get_child(i))) {
			if (idx < p_active_path.size() && p_active_path[idx] == btn->get_text()) {
				btn->set_self_modulate(Color::named("GREEN"));
			} else {
				btn->set_self_modulate(Color::named("WHITE"));
			}
			idx++;
		}
	}

	StringName actived_state;
	if (p_active_path.size() > 0) {
		actived_state = p_active_path[p_active_path.size() - 1];
	}
	fsm_editor->debug_highlight_active_state(actived_state, idx != p_active_path.size() - 1);
}

HFSM *HFSMEditor::get_editing_hfsm() { return hfsm; }

void HFSMEditor::queue_refresh() {
	ERR_FAIL_COND(!fsm_editor);
	if (hfsm) {
		fsm_editor->queue_refresh();
	}
}

void HFSMEditor::set_error_hint(const String &p_text) {
	hint_label->set_self_modulate(Color(1.0, 0.0, 0.0));
	hint_label->set_text(p_text);
	hint_timer->start();
}

void HFSMEditor::add_undo_redo_text(EditorUndoRedoManager *p_undo_redo, const String &p_action_name) {
	auto undo_redo = p_undo_redo;
	ADD_DO_METHOD(this, __do_history, p_action_name);
	ADD_UNDO_METHOD(this, __undo_history, p_action_name);
}

void HFSMEditor::__do_history(const String &p_text) {
	history_label->set_text(p_text);
}

void HFSMEditor::__undo_history(const String &p_text) {
	history_label->set_text((HfsmEditorPlugin::is_zh() ? "撤销: " : "Undo: ") + p_text);
}

void HFSMEditor::_notification(int p_what) {
	if (p_what == NOTIFICATION_READY && !debug_mode) {
		_change_hint();
	}
}

#define SET_CONNECT_INSPECTOR_SIGNAL(m_signal, m_connected)                                          \
	{                                                                                                \
		auto inspector = HfsmEditorPlugin::get_singleton()->get_editor_interface()->get_inspector(); \
		auto cb = TCALLABLE(_inspector_##m_signal);                                                  \
		if ((m_connected) && !inspector->is_connected(#m_signal, cb)) {                              \
			inspector->connect(#m_signal, cb);                                                       \
		} else if (!(m_connected) && inspector->is_connected(#m_signal, cb)) {                       \
			inspector->disconnect(#m_signal, cb);                                                    \
		}                                                                                            \
	}

void HFSMEditor::edit_hfsm(HFSM *p_hfsm) {
	hfsm = p_hfsm;

	mask_panel->set_visible(!p_hfsm);

	if (get_editing_hfsm()) {
		editing_root_fsm_config = hfsm->get_root_fsm_config();
		fsm_editor->edit_fsm_config(hfsm->get_root_fsm_config(), path_button_container, editing_root_fsm_config);
		set_connect_inspector_signal(true);
	} else {
		editing_root_fsm_config.unref();
		fsm_editor->edit_fsm_config(nullptr, path_button_container, nullptr);
		set_connect_inspector_signal(false);
	}
}

HFSMEditor *HFSMEditor::create_hfsm_editor(bool p_debug_mode) {
	auto ret = memnew(HFSMEditor(p_debug_mode));
	ret->initialize();
	return ret;
}

void HFSMEditor::_inspector_property_edited(const String &p_properrty) {
	if (!debug_mode && p_properrty == "root_fsm_config") {
		ERR_FAIL_COND(!get_editing_hfsm());
		if (editing_root_fsm_config != get_editing_hfsm()->get_root_fsm_config()) {
			call_deferred(TNAMEOF(edit_hfsm), get_editing_hfsm());
		}
	}
}

void HFSMEditor::set_connect_inspector_signal(bool p_connect) {
	SET_CONNECT_INSPECTOR_SIGNAL(edited_object_changed, p_connect);
	SET_CONNECT_INSPECTOR_SIGNAL(property_edited, p_connect);
}

void HFSMEditor::_change_hint() {
	const PackedStringArray en = make_arr<PackedStringArray>(
			"Holding \"Shift\" and drag a line from a State to another State by left button to create a Transition.",
			"To rename a state, you should input \"Enter\" after change its name, if not, the nane will be reset.",
			"Right click to popup a menu and refer short cuts.",
			R"(Holding "Alt" and "Middle Button" to draw a line to disconnect Transitions.)",
			"\"Convert To Sub-FSM\" only activated when selecting at least one State and click at a selected State.",
			"A FSM(Finite State Machine) only have a Enty State.",
			"A Entry State can't be change to Noramal or Exit State.",
			"If you set a Normal or Exit State to Entry State, it will change the existing Entry State to Normal State first.",
			"Select and inspect a State, you can set its animation in inspector.",
			"The expression of ExpressionTransition is base on its HFSM node, you can use all built-in singletons in expression.",
			"A Trigger Variable like a bool Variable, but it will be reset to \"false\" after processing the try transit stage.",
			"Trigger a \"Solo Trigger\" will make its Transition can transit without concerning other VariableExpressions.",
			"Only all \"Union Trigger\"s are triggered at the same time can make Transition can transit.");

	const std::array<const char *, 13> zh = {
		"按住Shift和鼠标左键从一个状态连接到另一个状态来实现创建转换流。",
		"必须在修改状态名称后输入\"Enter\"确认,否则将在失去焦点时重置为改变之前的名称。",
		"点击右键弹出操作菜单并查看相关的快捷键。",
		"按住Alt和鼠标中键来绘制一条删除线以删除某个转换流。",
		"只有在选中了至少一个状态并在选中的某个状态上点击右键时，\"转换为子状态机\"选项才有效。",
		"一个FSM(有限状态机)只有一个Entry状态。",
		"一个Entry类型的状态不能被设置为Normal或者Exit状态。",
		"如果你将一个Normal或Exit状态设置为Entry状态, 会自动先将已有的Entry状态设置为Normal状态。",
		"你可以通过选中一个状态，在检查器中编辑其动画属性。",
		"ExpressionTransition的表达式基于所属的HFSM节点,你可以在表达式中使用所有内建单例。",
		"触发器(Trigger)变量与布尔(bool)变量类似,但它会在尝试转换的阶ws段结束后被重置为\"false\"。",
		"触发一个\"Solo Trigger\"会在不考虑其他VariableExpression的前提下使其Transition能够转换。",
		"只有在所有的\"Union Trigger\"均被触发时会使其Transition能够转换(不考虑其他VariableExpression)。"
	};

	IF_DEV(ERR_FAIL_COND(en.size() != zh.size());)

	const int size = en.size() < zh.size() ? en.size() : zh.size();
	static int index = -1;

	if (!is_inside_tree()) {
		return;
	}

	index += 1;
	if (index >= size) {
		index = 0;
	}
	hint_label->set_self_modulate(Color(1, 1, 1));
	hint_label->set_text("Tip: " + (HfsmEditorPlugin::is_zh() ? String::utf8(zh[index]) : en[index]));
	hint_timer->start();
}

void HFSMEditor::_inspector_edited_object_changed() {
	set_connect_inspector_signal(false);
}

void HFSMEditor::_edit_fsm_requested(const Ref<FSMConfig> &p_fsm_config) {
	edit_fsm_config_in_hfsm(p_fsm_config);
}

void HFSMEditor::_bind_methods() {
	GDBIND_BEGIN(HFSMEditor);
	GDBIND_METHOD(edit_hfsm);
	GDBIND_METHOD(__do_history);
	GDBIND_METHOD(__undo_history);

	GDBIND_CALBACK(_inspector_edited_object_changed);
	GDBIND_CALBACK(_inspector_property_edited);
	GDBIND_CALBACK(_edit_fsm_requested);
	GDBIND_CALBACK(_change_hint);
}

HFSMEditor::HFSMEditor(bool p_debug_mode) :
		debug_mode(p_debug_mode) {
	if (debug_mode) {
		return;
	}

	hint_timer = memnew(Timer);
	hint_timer->set_wait_time(10.0);
	hint_timer->connect("timeout", TCALLABLE(_change_hint));
	add_child(hint_timer);
}

}; // namespace Hfsm
