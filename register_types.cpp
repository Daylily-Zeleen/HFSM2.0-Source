/**************************************************************************/
/*  register_types.cpp                                                    */
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

/**************************************************************************/
/*  register_types.cpp                                                    */
/**************************************************************************/
/*                         This file is part of:                          */
/*                   Hierarchical Finite State Machine                    */
/*            https://github.com/Daylily-Zeleen/HFSM2.0-Source            */
/**************************************************************************/
/* Copyright (c) 2022-present Daylily Zeleen.                             */
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

#include "register_types.h"

#ifdef GDEXTENSION_BUILD
#include <gdextension_interface.h>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/defs.hpp>
#include <godot_cpp/godot.hpp>
#endif // GDEXTENSION_BUILD

#include "hfsm_global.h"
#include "src/fsm_config.h"
#include "src/hfsm.h"
#include "src/state.h"
#include "src/state_config.h"
#include "src/transition_config.h"
#include "src/transitions/auto_transition.h"
#include "src/transitions/transition.h"
#include "src/transitions/variable_expressions/variable_expression_config.h"
#include "src/variable.h"
#include "src/variable_config.h"

#ifdef TOOLS_ENABLED
#include "editor/fsm_editor.h"
#include "editor/hfsm_editor.h"
#include "editor/hfsm_editor_plugin.h"
#include "editor/state_node.h"

#ifdef GDEXTENSION_BUILD
#include "editor/hfsm_debugger_plugin.h"
#include "godot_cpp/classes/editor_plugin.hpp"

#else
#include <editor/editor_plugin.h>
#include <scene/gui/view_panner.h>
#endif // GDEXTENSION_BUILD

#endif // TOOLS_ENABLED

using namespace Hfsm;

void register_core_classes() {
	GDREGISTER_CLASS(HFSM);
	GDREGISTER_CLASS(State);
	GDREGISTER_CLASS(Transition);
	GDREGISTER_CLASS(Variable);
	GDREGISTER_CLASS(FSMConfig);
	GDREGISTER_CLASS(StateConfig);
	GDREGISTER_CLASS(TransitionConfig);
	GDREGISTER_CLASS(VariableConfig);
	GDREGISTER_CLASS(VariableExpressionConfig);
}

void register_editor_classes() {
	IF_TOOLS({
		// GDREGISTER_INTERNAL_CLASS(HfsmEditorPlugin);
		// GDREGISTER_INTERNAL_CLASS(EditorPropertyVariableConfig);
		// GDREGISTER_INTERNAL_CLASS(EditorPropertyVariableConfig::VariableConfigSelector);
		// GDREGISTER_INTERNAL_CLASS(StateNode);
		// GDREGISTER_INTERNAL_CLASS(FsmEditor);
		// GDREGISTER_INTERNAL_CLASS(HfsmInspectorPlugin);
		// GDREGISTER_INTERNAL_CLASS(HFSMEditor);
		GDREGISTER_CLASS(HfsmEditorPlugin);
		GDREGISTER_CLASS(EditorPropertyVariableConfig);
		GDREGISTER_CLASS(EditorPropertyVariableConfig::VariableConfigSelector);
		GDREGISTER_CLASS(StateNode);
		GDREGISTER_CLASS(FsmEditor);
		GDREGISTER_CLASS(HfsmInspectorPlugin);
		GDREGISTER_CLASS(HFSMEditor);
		IF_GDE({
			GDREGISTER_ABSTRACT_CLASS(HfsmDebuggerPlugin);
			GDREGISTER_CLASS(HfsmDebugger);
		})
	})
}

void initialize_hfsm_module(ModuleInitializationLevel p_level) {
	IF_TOOLS(
			if (p_level == MODULE_INITIALIZATION_LEVEL_EDITOR) {
				register_editor_classes();

				EditorPlugins::add_by_type<HfsmEditorPlugin>();
			})
	if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
		return;
	}
	register_core_classes();

	HfsmGlobal::init_static();
}

void uninitialize_hfsm_module(ModuleInitializationLevel p_level) {
	IF_TOOLS(
			IF_GDE(
					if (p_level == MODULE_INITIALIZATION_LEVEL_EDITOR) {
						EditorPlugins::remove_by_type<HfsmEditorPlugin>();
					}))

	if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
		return;
	}

	HfsmGlobal::deinit_static();
}

#ifdef GDEXTENSION_BUILD

extern "C" {

// Initialization.
GDExtensionBool GDE_EXPORT hfsm2_library_init(GDExtensionInterfaceGetProcAddress p_get_proc_address,
		GDExtensionClassLibraryPtr p_library, GDExtensionInitialization *r_initialization) {
	godot::GDExtensionBinding::InitObject init_obj(p_get_proc_address, p_library, r_initialization);

	init_obj.register_initializer(initialize_hfsm_module);
	init_obj.register_terminator(uninitialize_hfsm_module);
	init_obj.set_minimum_library_initialization_level(MODULE_INITIALIZATION_LEVEL_SCENE);

	return init_obj.init();
}
}

#endif // GDEXTENSION_BUILD
