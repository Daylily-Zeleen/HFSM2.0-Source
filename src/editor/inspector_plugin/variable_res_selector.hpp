#pragma once

#include <godot_cpp/classes/editor_property.hpp>
#include <godot_cpp/classes/ref.hpp>

#include <godot_cpp/classes/button.hpp>
#include <godot_cpp/classes/editor_inspector_plugin.hpp>
#include <godot_cpp/classes/editor_property.hpp>
#include <godot_cpp/classes/popup_menu.hpp>

using namespace godot;

namespace Hfsm {
class HFSM;
class HFSMVariableRes;
// 变量选择器
class VariableResSelector : public EditorProperty {
	GDCLASS(VariableResSelector, EditorProperty)
private:
	const String NULL_TEXT = "<null>";

	Button *btn = nullptr;
	PopupMenu *menu = nullptr;
	HFSM *_hfsm = nullptr;

	Ref<HFSMVariableRes> _to_compare;
	bool updating = false;

	void __on_btn_pressed();

	void __on_menu_index_pressed(int32_t index);

	String _get_type_text(Variant::Type type);

protected:
	static void _bind_methods();

	// String _to_string() const;

public:
	VariableResSelector();

	void __setup(HFSM *hfsm, Ref<HFSMVariableRes> to_compare = nullptr);

	void _update_property();
};

}; // namespace Hfsm
