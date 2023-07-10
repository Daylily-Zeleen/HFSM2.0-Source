#pragma once

#ifdef GDEXTENSION_BUILD
#include <godot_cpp/classes/button.hpp>
#include <godot_cpp/classes/editor_property.hpp>
#include <godot_cpp/classes/popup_menu.hpp>

using namespace godot;
#else // GDEXTENSION_BUILD
#include <editor/editor_properties.h>
#include <scene/gui/button.h>
#include <scene/gui/popup_menu.h>

#include <editor/editor_resource_picker.h>
#endif // GDEXTENSION_BUILD

#include "../../hfsm_global.h"

namespace Hfsm {
// todo:: bug

class _VariableResSelector : public EditorResourcePicker {
private:
	class HFSM *hfsm;

	Ref<class HFSMVariableRes> to_compare;

	Vector<Ref<HFSMVariableRes>> variable_res_list;

public:
	int op_ofs = 10;

	void GD_(set_create_options)(Object *p_menu_node) override;
	bool GD_(handle_menu_selected)(int p_which) override;

	_VariableResSelector() = default;
	_VariableResSelector(HFSM *p_hfsm, const Ref<HFSMVariableRes> &p_to_compare);
};

// 变量选择器
class VariableResSelector : public EditorProperty {
	GDCLASS(VariableResSelector, EditorProperty)
private:
	_VariableResSelector *selector = nullptr;
	// Button *btn = nullptr;
	// PopupMenu *menu = nullptr;
	class HFSM *hfsm = nullptr;

	Ref<class HFSMVariableRes> to_compare;
	bool updating = false;

	void _variable_selected(const Ref<Resource> &p_res);

	void update_property_internal();

protected:
	static void _bind_methods();

public:
	VariableResSelector();
	VariableResSelector(HFSM *p_hfsm, const Ref<HFSMVariableRes> &p_to_compare = nullptr);

	void GD_(update_property)() override { update_property_internal(); }
};

}; // namespace Hfsm
