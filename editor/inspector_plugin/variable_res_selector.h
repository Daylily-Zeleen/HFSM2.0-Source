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

#endif // GDEXTENSION_BUILD

namespace Hfsm {
// 变量选择器
class VariableResSelector : public EditorProperty {
	GDCLASS(VariableResSelector, EditorProperty)
private:
	const String NULL_TEXT = "<null>";

	Button *btn = nullptr;
	PopupMenu *menu = nullptr;
	class HFSM *hfsm = nullptr;

	Ref<class HFSMVariableRes> to_compare;
	bool updating = false;

	void _btn_pressed();
	void _menu_index_pressed(int32_t p_index);

	String get_type_text(Variant::Type p_type);
	void update_property_internal();

protected:
	static void _bind_methods();

public:
	VariableResSelector();
	VariableResSelector(HFSM *p_hfsm, const Ref<HFSMVariableRes> &p_to_compare = nullptr);

#ifdef GDEXTENSION_BUILD
	void _update_property() override { update_property_internal(); }
#else
	void update_property() override { update_property_internal(); }
#endif //  GDEXTENSION_BUILD
};

}; // namespace Hfsm
