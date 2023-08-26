/**************************************************************************/
/*  hfsm_editor.h                                                         */
/**************************************************************************/
/*                         This file is part of:                          */
/*                   Hierarchical Finite State Machine                    */
/*            https://github.com/Daylily-Zeleen/HFSM2.0-Source            */
/**************************************************************************/
/* Copyright (c) 2023-present Daylily Zeleen.                             */
/*                                                                        */
/* Permission is hereby granted, free of charge, to any person obtaining  */
/* a copy of this software and associated documentation files (the        */
/* "Software"), to deal in the Software without restriction, including    */
/* without limitation the rights to use, copy, modify, merge, publish,    */
/* distribute, sublicense, and/or sell copies of the Software, and to     */
/* permit persons to whom the Software is furnished to do so, subject to  */
/* the following conditions:                                              */
/*                                                                        */
/* The above copyright notice and this permission notice shall be         */
/* included in all copies or substantial portions of the Software.        */
/*                                                                        */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. */
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY   */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,   */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE      */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                 */
/**************************************************************************/

#pragma once

#ifdef GDEXTENSION_BUILD
#include <godot_cpp/classes/editor_undo_redo_manager.hpp>
#include <godot_cpp/classes/h_box_container.hpp>
#include <godot_cpp/classes/label.hpp>
#include <godot_cpp/classes/panel.hpp>
#include <godot_cpp/classes/timer.hpp>

using namespace godot;

#else
#include <editor/editor_undo_redo_manager.h>
#include <scene/gui/box_container.h>
#include <scene/gui/label.h>
#include <scene/main/timer.h>

#endif // GDEXTENSION_BUILD

namespace HFSM2 {
class FsmEditor;
class StateConfig;
class FSMConfig;
class HFSM;

class HFSMEditor : public Control {
	GDCLASS(HFSMEditor, Control)
protected:
	static void _bind_methods();
	void _notification(int p_what);

public:
	HFSMEditor(bool p_debug_mode = false);

	void edit_hfsm(HFSM *p_hfsm);
	HFSM *get_editing_hfsm();
	void edit_fsm_config_in_hfsm(const Ref<FSMConfig> &p_fsm_config, const Ref<FSMConfig> &p_root_fsm_config = nullptr);

	void debug_highlight_activate_state(const PackedStringArray &p_active_path);

	static HFSMEditor *create_hfsm_editor(bool p_debug_mode = false);

	void queue_refresh();

	void set_error_hint(const String &p_text);

	void add_undo_redo_text(EditorUndoRedoManager *p_undo_redo, const String &p_action_name);

private:
	HBoxContainer *path_button_container = nullptr;
	FsmEditor *fsm_editor = nullptr;
	//  TODO::实际显示内容
	Label *hint_label = nullptr;
	Label *history_label = nullptr;
	//
	Panel *mask_panel = nullptr;
	Label *not_hfsm_label = nullptr;

	HFSM *hfsm = nullptr;
	Timer *hint_timer = nullptr;

	// 调试模式下hfsm为nullptr
	const bool debug_mode = false;

	bool try_set_nested_state_config_for_fsm_config_recursively(const Ref<FSMConfig> &p_fsm_config, Ref<FSMConfig> p_to_search_fsm_config = nullptr);
	void initialize();

	void _inspector_property_edited(const String &p_properrty);
	void _inspector_edited_object_changed();
	void _edit_fsm_requested(const Ref<FSMConfig> &p_fsm_config);
	void _change_hint();

	// Use for setting undo redo text, don't call normally.
	void __do_history(const String &p_text);
	void __undo_history(const String &p_text);

	void set_connect_inspector_signal(bool p_connect);

	Ref<FSMConfig> editing_root_fsm_config;
};

}; // namespace HFSM2
