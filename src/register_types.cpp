/*************************************************************************/
/*  register_types.cpp                                                   */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2022 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2022 Godot Engine contributors (cf. AUTHORS.md).   */
/*                                                                       */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:                                             */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/

#include "register_types.h"

#include <gdextension_interface.h>

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/defs.hpp>
#include <godot_cpp/godot.hpp>

#include "core/fsm_res.hpp"
#include "core/hfsm.hpp"
#include "core/hfsm_variable.hpp"
#include "core/hfsm_variable_res.hpp"
#include "core/state.hpp"
#include "core/state_res.hpp"
#include "core/transition_res.hpp"
#include "core/transitions/auto_transition.hpp"
#include "core/transitions/transition.hpp"
#include "core/transitions/variable_expressions/variable_expression_res.hpp"
#include <hfsm_global.hpp>

#include "editor/hfsm_editor.hpp"
#include "editor/hfsm_editor_plugin.hpp"
#include "editor/inspector_plugin/variable_res_selector.hpp"
#include "editor/state_node.hpp"
#include "editor/state_nodes_editor.hpp"
#include "godot_cpp/classes/editor_plugin.hpp"

using namespace Hfsm;

void register_core_classes() {
	// return;
	ClassDB::register_class<HFSM>();
	// UtilityFunctions::print("HFSM");
	ClassDB::register_class<State>();
	// UtilityFunctions::print("State");
	ClassDB::register_class<Transition>();
	// UtilityFunctions::print("Transition");
	ClassDB::register_class<HFSMVariable>();
	// UtilityFunctions::print("HFSMVariable");
	ClassDB::register_class<FsmRes>();
	// UtilityFunctions::print("FsmRes");
	ClassDB::register_class<StateRes>();
	// UtilityFunctions::print("StateRes");
	ClassDB::register_class<TransitionRes>();
	// UtilityFunctions::print("TransitionRes");
	ClassDB::register_class<HFSMVariableRes>();
	// UtilityFunctions::print("HFSMVariableRes");
	ClassDB::register_class<VariableExpressionRes>();
	// UtilityFunctions::print("VariableExpressionRes");
}

void register_editor_classes() {
	ClassDB::register_class<HfsmEditorPlugin>();
	ClassDB::register_class<VariableResSelector>();
	ClassDB::register_class<StateNode>();
	ClassDB::register_class<StateNodesEditor>();
	ClassDB::register_class<HfsmInspectorPlugin>();
	ClassDB::register_class<HFSMEditor>();

	EditorPlugins::add_by_type<HfsmEditorPlugin>();
	// ClassDB::register_internal_class<VariableResSelector>();
	// ClassDB::register_internal_class<StateNode>();
	// ClassDB::register_internal_class<StateNodesEditor>();
	// ClassDB::register_internal_class<HfsmInspectorPlugin>();
	// ClassDB::register_internal_class<HFSMEditor>();
}

void initialize_hfsm_module(ModuleInitializationLevel p_level) {
	if (p_level == MODULE_INITIALIZATION_LEVEL_EDITOR) {
		register_editor_classes();
	}
	if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
		return;
	}
	register_core_classes();

	HfsmGlobal::init_static();
}

void uninitialize_hfsm_module(ModuleInitializationLevel p_level) {
	if (p_level == MODULE_INITIALIZATION_LEVEL_EDITOR) {
		EditorPlugins::remove_by_type<HfsmEditorPlugin>();
	}
	if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
		return;
	}

	HfsmGlobal::deinit_static();
}

extern "C" {

// Initialization.
GDExtensionBool GDE_EXPORT hfsm_library_init(GDExtensionInterfaceGetProcAddress p_get_proc_address,
		GDExtensionClassLibraryPtr p_library, GDExtensionInitialization *r_initialization) {
	godot::GDExtensionBinding::InitObject init_obj(p_get_proc_address, p_library, r_initialization);

	init_obj.register_initializer(initialize_hfsm_module);
	init_obj.register_terminator(uninitialize_hfsm_module);
	init_obj.set_minimum_library_initialization_level(MODULE_INITIALIZATION_LEVEL_SCENE);

	return init_obj.init();
}
}
