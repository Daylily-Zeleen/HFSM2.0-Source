#pragma once

#include <godot_cpp/classes/editor_property.hpp>
#include <godot_cpp/classes/ref.hpp>

#include <godot_cpp/classes/button.hpp>
#include <godot_cpp/classes/editor_inspector_plugin.hpp>
#include <godot_cpp/classes/editor_property.hpp>
#include <godot_cpp/classes/popup_menu.hpp>

using namespace godot;

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

	void __on_btn_pressed();

	void __on_menu_index_pressed(int32_t p_index);

	String _get_type_text(Variant::Type p_type);

protected:
	static void _bind_methods();

public:
	VariableResSelector();

	void setup(HFSM *p_hfsm, const Ref<HFSMVariableRes> &p_to_compare = nullptr);

	void _update_property() override;
};

}; // namespace Hfsm
