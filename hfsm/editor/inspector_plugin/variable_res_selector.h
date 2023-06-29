#pragma once

#ifdef GDEXTENSION_BUILD
#include <godot_cpp/classes/editor_property.hpp>
using namespace godot;
#else
#include <editor/editor_properties.h>
#endif // GDEXTENSION_BUILD

namespace Hfsm {
// 变量选择器
class VariableResSelector : public EditorProperty {
	GDCLASS(VariableResSelector, EditorProperty)
private:
	const String NULL_TEXT = "<null>";

	class Button *btn = nullptr;
	class PopupMenu *menu = nullptr;
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

	// void setup(HFSM *p_hfsm, const Ref<HFSMVariableRes> &p_to_compare = nullptr);

#ifdef GDEXTENSION_BUILD
	void _update_property() override { update_property_internal(); }
#else
	void update_property() override { update_property_internal(); }
#endif //  GDEXTENSION_BUILD
};

}; // namespace Hfsm
