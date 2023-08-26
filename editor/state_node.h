/**************************************************************************/
/*  state_node.h                                                          */
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
#include <godot_cpp/classes/check_box.hpp>
#include <godot_cpp/classes/graph_node.hpp>
#include <godot_cpp/classes/image_texture.hpp>
#include <godot_cpp/classes/line_edit.hpp>
#include <godot_cpp/classes/option_button.hpp>
#include <godot_cpp/classes/script.hpp>

#include <godot_cpp/classes/editor_resource_picker.hpp>
using namespace godot;
#else
#include <scene/gui/graph_node.h>

#include <editor/editor_resource_picker.h>
#include <scene/gui/check_box.h>
#include <scene/gui/line_edit.h>
#include <scene/gui/option_button.h>

#endif // GDEXTENSION_BUILD

#include "../src/fsm_config.h"

namespace HFSM2 {

// class FSMConfig;
class StateConfig;

class StateNode : public GraphNode {
	GDCLASS(StateNode, GraphNode)
protected:
	static void _bind_methods();

	void _notification(int p_what);

private:
	Ref<FSMConfig> nested_fsm_config;
	Ref<StateConfig> state_config;
	// ==================
	LineEdit *name_line_edit = nullptr;
	OptionButton *type_option_btn = nullptr;
	CheckBox *has_sub_fsm_check_box = nullptr;

	Button *sub_fsm_btn = nullptr;
	EditorResourcePicker *script_picker = nullptr;

	void set_state_config(const Ref<StateConfig> &p_state_config);

	// 动画交给监视器
	// ==================
	bool __has_duplicate_name(const String &p_to_test_name);
	// ==================
	void _setup_state_config();
	void __on_configize();
	void _cancel_name_changed();
	void _accept_name_changed(const String &p_new_name);
	void _type_option_btn_item_selected(int32_t p_idx);
	void _set_has_sub_fsm_check_box(bool p_pressed);
	void _request_edit_sub_fsm_config();
	void _script_selected(const Ref<Script> &p_script, bool p_edit);
	void _script_changed(const Ref<Script> &p_script);
	void initialize();
	// ==================
	String str_localize(const String &p_en_key) const;

	const bool debug_mode = false;
	bool debug_actived = false;

public:
	static Ref<ImageTexture> (*get_empty_icon)();
	static Color IN_COLOR() { return Color::named("ORANGE"); }
	static Color OUT_COLOR() { return Color::named("GREEN"); }

	static const int IN_TYPE = 0;
	static const int OUT_TYPE = 1;

	Ref<class StateConfig> get_state_config() const;
	static StateNode *create_state_node(const Ref<StateConfig> &p_target_state_config, const Ref<class FSMConfig> &p_nested_fsm_config, bool p_debug = false);

	void set_debug_actived(bool p_actived);
	bool is_debug_actived() const;

	StateNode(bool p_debug_mode = false);
};

} // namespace HFSM2
