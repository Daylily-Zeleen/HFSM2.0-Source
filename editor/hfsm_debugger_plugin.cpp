/**************************************************************************/
/*  hfsm_debugger_plugin.cpp                                              */
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

#include "hfsm_debugger_plugin.h"

#include "../src/fsm_config.h"
#include "../src/hfsm.h"
#include "hfsm_editor.h"
#include "hfsm_editor_plugin.h"

#ifdef GDEXTENSION_BUILD
#include <godot_cpp/classes/dir_access.hpp>
#include <godot_cpp/classes/editor_debugger_session.hpp>
#include <godot_cpp/classes/editor_interface.hpp>
#include <godot_cpp/classes/editor_paths.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/engine_debugger.hpp>
#include <godot_cpp/classes/file_access.hpp>
#include <godot_cpp/classes/project_settings.hpp>
#include <godot_cpp/classes/resource_loader.hpp>
#include <godot_cpp/classes/resource_saver.hpp>

#else
#include <core/config/project_settings.h>
#include <core/io/dir_access.h>

#include <core/debugger/engine_debugger.h>
#include <editor/editor_interface.h>
#include <editor/editor_paths.h>
#endif //GDEXTENSION_BUILD

namespace HFSM2 {

#define msg_built "built"
#define msg_destory "destory"
#define msg_update_active_path "update_active_path"

#define is_msg(m_msg, m_to_cmp) ((m_msg) == "hfsm:" m_to_cmp)

#define get_node_path(m_data) NodePath((m_data).back())

uint32_t hash(const Variant &p_var) {
	if (auto obj = Object::cast_to<Object>(p_var)) {
		uint32_t ret = 0;
		TypedArray<Dictionary> props = obj->call(SNAME("get_property_list"));
		for (auto i = 0; i < props.size(); ++i) {
			Dictionary p = props[i];
			ret = hash_murmur3_one_32(hash(obj->get(p["name"])), ret);
		}
		return ret;
	} else {
		return p_var.hash();
	}
}

// =======

String HFSMDebuggerPlugin::get_cache_dir() {
	if (ProjectSettings::get_singleton()->get_setting_with_override("application/config/use_hidden_project_data_directory")) {
		return "res://.godot/editor/hfsm/";
	} else {
		return "res://godot/editor/hfsm/";
	}
}

bool HFSMDebuggerPlugin::can_debug() {
	return !Engine::get_singleton()->is_editor_hint() && EngineDebugger::get_singleton() && EngineDebugger::get_singleton()->is_active();
}

void HFSMDebuggerPlugin::send_debug_built(HFSM *p_hfsm) {
	IF_DEBUG({
		if (!can_debug()) {
			return;
		}

		const String cache_dir = get_cache_dir();

		if (!DirAccess::dir_exists_absolute(cache_dir)) {
			DirAccess::make_dir_absolute(cache_dir);
		}
		const auto fsm_config = p_hfsm->get_root_fsm_config();
		auto cache_path = cache_dir.path_join(itos(hash(fsm_config)).md5_text() + ".tres");

		Error err = OK;

		bool existing = false;
		IF_GDM(existing = FileAccess::exists(cache_path);)
		IF_GDE(existing = FileAccess::file_exists(cache_path);)

		if (!existing) {
			IF_GDE(err = ResourceSaver::get_singleton()->save(fsm_config, cache_path);)
			IF_GDM(err = ResourceSaver::save(fsm_config, cache_path);)
		}

		if (err == OK) {
			send_debug_msg(p_hfsm, msg_built, make_arr<Array>(cache_path));
		}
	})
}

void HFSMDebuggerPlugin::send_debug_destroy(HFSM *p_hfsm) {
	IF_DEBUG({
		if (!can_debug()) {
			return;
		}
		send_debug_msg(p_hfsm, msg_destory, Array());
	})
}

void HFSMDebuggerPlugin::send_debug_update_active_path(HFSM *p_hfsm) {
	IF_DEBUG({
		if (!can_debug()) {
			return;
		}
		auto path_state = p_hfsm->get_current_state()->get_path();
		PackedStringArray path_state_name;
		auto current_state = p_hfsm->get_current_state();

		if (current_state.is_valid()) {
			path_state_name.resize(path_state.size() + 2);
			path_state_name.set(0, "root");

			for (auto i = 0; i < path_state.size(); ++i) {
				path_state_name.set(i + 1, Object::cast_to<State>(path_state[i])->get_name());
			}

			path_state_name.set(path_state_name.size() - 1, p_hfsm->get_current_state()->get_name());
		}

		send_debug_msg(p_hfsm, msg_update_active_path, make_arr<Array>(path_state_name));
	})
}

void HFSMDebuggerPlugin::_session_started(SessionID p_session_id) {
	ERR_FAIL_COND(debuggers.has(p_session_id));

	auto session = get_session(p_session_id);
	auto debugger = memnew(HFSMDebugger);
	debuggers.insert(p_session_id, debugger);
	session->add_session_tab(debugger);
}

void HFSMDebuggerPlugin::_session_stoped(SessionID p_session_id) {
	ERR_FAIL_COND(!debuggers.has(p_session_id));

	auto debugger = debuggers[p_session_id];
	debugger->stop();
	auto session = get_session(p_session_id);
	session->remove_session_tab(debugger);
	debuggers.erase(p_session_id);
}

// Without debug valid check.
void HFSMDebuggerPlugin::send_debug_msg(HFSM *p_hfsm, const String &p_msg, Array p_data) {
	if (p_hfsm->is_inside_tree()) {
		p_data.push_back(p_hfsm->get_path());
		EngineDebugger::get_singleton()->send_message("hfsm:" + p_msg, p_data);
	}
}

void HFSMDebuggerPlugin::GD_(setup_session)(int p_idx) {
	ERR_FAIL_COND(debuggers.has(p_idx));

	Ref<EditorDebuggerSession> session = get_session(p_idx);

	session->connect(SNAME("started"), TCALLABLE_BIND(_session_started, p_idx));
	session->connect(SNAME("stopped"), TCALLABLE_BIND(_session_stoped, p_idx));
}

bool HFSMDebuggerPlugin::GD_(capture)(const String &p_message, const Array &p_data, int p_session) {
	if (is_msg(p_message, msg_built)) {
		ERR_FAIL_COND_V(!debuggers.has(p_session), true);

		auto hfsm_node_path = get_node_path(p_data);
		const String fsm_config_path = p_data[0];
		Ref<FSMConfig> root_fsm_config;
		IF_GDE(root_fsm_config = ResourceLoader::get_singleton()->load(fsm_config_path);)
		IF_GDM(root_fsm_config = ResourceLoader::load(fsm_config_path);)
		if (root_fsm_config.is_valid()) {
			debuggers[p_session]->build(hfsm_node_path, root_fsm_config, fsm_config_path);
		}
		return true;
	} else if (is_msg(p_message, msg_destory)) {
		ERR_FAIL_COND_V(!debuggers.has(p_session), true);

		auto hfsm_node_path = get_node_path(p_data);
		debuggers[p_session]->destory(hfsm_node_path);

		return true;
	} else if (is_msg(p_message, msg_update_active_path)) {
		ERR_FAIL_COND_V(!debuggers.has(p_session), true);

		auto hfsm_node_path = get_node_path(p_data);
		const PackedStringArray active_path = p_data[0];
		debuggers[p_session]->update_active_path(hfsm_node_path, active_path);
		return true;
	}
	return false;
}

bool HFSMDebuggerPlugin::GD_(has_capture)(const String &p_capture) const {
	return p_capture == "hfsm";
}

void HFSMDebuggerPlugin::_bind_methods() {
	GDBIND_BEGIN(HFSMDebuggerPlugin);
	GDBIND_CALBACK(_session_stoped);
	GDBIND_CALBACK(_session_started);
}

HFSMDebuggerPlugin::~HFSMDebuggerPlugin() {
	const auto cache_dir = get_cache_dir();
	if (!DirAccess::dir_exists_absolute(cache_dir)) {
		return;
	}

	for (const auto &f : DirAccess::get_files_at(cache_dir)) {
		DirAccess::remove_absolute(cache_dir.path_join(f));
	}
}

// HFSMDebugger
void HFSMDebugger::build(const NodePath &p_path, const Ref<class FSMConfig> &p_root_fsm_config, const String &p_cache_path) {
	ERR_FAIL_COND(datas.has(p_path));
	datas.insert(p_path, { p_root_fsm_config });

	update_node_paths();
}

void HFSMDebugger::destory(const NodePath &p_path) {
	ERR_FAIL_COND(!datas.has(p_path));

	DirAccess::remove_absolute(datas[p_path].root_fsm_config->get_path());

	datas.erase(p_path);

	if (p_path == current_hfsm_path) {
		hfsm_editor->edit_fsm_config_in_hfsm(nullptr, nullptr);
		current_hfsm_path = {};
	}

	update_node_paths();
}

void HFSMDebugger::update_active_path(const NodePath &p_path, const PackedStringArray &p_new_active_path) {
	ERR_FAIL_COND(!datas.has(p_path));
	datas[p_path].current_active_path = p_new_active_path;

	if (p_path == current_hfsm_path) {
		hfsm_editor->debug_highlight_activate_state(p_new_active_path);
	}
}

void HFSMDebugger::stop() {
}

void HFSMDebugger::update_node_paths() {
	node_paths->clear();
	for (const auto &E : datas) {
		node_paths->add_item(E.key);
		node_paths->set_item_metadata(node_paths->get_item_count() - 1, E.key);
	}
}

void HFSMDebugger::_item_activated(int p_idx) {
	current_hfsm_path = node_paths->get_item_metadata(p_idx);
	auto root_fsm_config = datas[current_hfsm_path].root_fsm_config;
	hfsm_editor->edit_fsm_config_in_hfsm(root_fsm_config, root_fsm_config);
	hfsm_editor->debug_highlight_activate_state(datas[current_hfsm_path].current_active_path);
}

void HFSMDebugger::_bind_methods() {
	GDBIND_BEGIN(HFSMDebugger);

	GDBIND_CALBACK(_item_activated);
}

HFSMDebugger::HFSMDebugger() {
	node_paths = memnew(ItemList);
	node_paths->set_v_size_flags(SizeFlags::SIZE_EXPAND_FILL);
	add_child(node_paths);
	node_paths->connect("item_activated", TCALLABLE(_item_activated));

	hfsm_editor = HFSMEditor::create_hfsm_editor(true);
	hfsm_editor->set_h_size_flags(SizeFlags::SIZE_EXPAND_FILL);
	hfsm_editor->set_v_size_flags(SizeFlags::SIZE_EXPAND_FILL);
	add_child(hfsm_editor);

	set_split_offset(100);

	set_name("HFSM");
}

HFSMDebugger::~HFSMDebugger() {
	for (const auto &E : datas) {
		DirAccess::remove_absolute(E.value.root_fsm_config->get_path());
	}
}

} //namespace HFSM2
