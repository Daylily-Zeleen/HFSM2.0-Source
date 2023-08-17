/**************************************************************************/
/*  hfsm_debugger_plugin.h                                                */
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

#include "../hfsm_global.h"

#ifdef GDEXTENSION_BUILD
#include <godot_cpp/classes/editor_debugger_plugin.hpp>
#include <godot_cpp/classes/h_split_container.hpp>
#include <godot_cpp/classes/item_list.hpp>
#include <godot_cpp/templates/hash_map.hpp>

using namespace godot;
#else // GDEXTENSION_BUILD
#include <editor/plugins/editor_debugger_plugin.h>
#include <scene/gui/item_list.h>
#include <scene/gui/split_container.h>

#endif // GDEXTENSION_BUILD

namespace Hfsm {
class HfsmDebugger;

class HfsmDebuggerPlugin : public EditorDebuggerPlugin {
	GDCLASS(HfsmDebuggerPlugin, EditorDebuggerPlugin)

	using SessionID = int;

	HashMap<SessionID, HfsmDebugger *> debuggers;

	void _session_started(SessionID p_session_id);
	void _session_stoped(SessionID p_session_id);

	static void send_debug_msg(class HFSM *p_hfsm, const String &p_msg, Array p_data);

	static String get_cache_dir();

	static bool can_debug();

protected:
	static void _bind_methods();

public:
	void GD_(setup_session)(int p_idx) override;
	bool GD_(capture)(const String &p_message, const Array &p_data, int p_session) override;
	bool GD_(has_capture)(const String &p_capture) const override;

	// Called by runtime.
	static void send_debug_built(class HFSM *p_hfsm);
	static void send_debug_destroy(class HFSM *p_hfsm);
	static void send_debug_update_active_path(class HFSM *p_hfsm);

	~HfsmDebuggerPlugin() override;
};

class HfsmDebugger : public HSplitContainer {
	GDCLASS(HfsmDebugger, HSplitContainer)
	struct NodeData {
		Ref<class FSMConfig> root_fsm_config;
		PackedStringArray current_active_path;
	};
	HashMap<NodePath, NodeData> datas;

	NodePath current_hfsm_path;

	ItemList *node_paths = nullptr;
	class HFSMEditor *hfsm_editor = nullptr;

	void _item_activated(int p_idx);

	void update_node_paths();

protected:
	static void _bind_methods();

public:
	void build(const NodePath &p_path, const Ref<class FSMConfig> &p_root_fsm_config, const String &p_cache_path);
	void destory(const NodePath &p_path);
	void update_active_path(const NodePath &p_path, const PackedStringArray &p_new_active_path);

	void stop();

	HfsmDebugger();
	~HfsmDebugger() override;
};

} //namespace Hfsm
