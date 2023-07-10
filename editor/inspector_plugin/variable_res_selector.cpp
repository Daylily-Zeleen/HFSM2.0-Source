#include "variable_res_selector.h"

#include "../../src/hfsm.h"
#include "../../src/hfsm_variable_res.h"

namespace Hfsm {

void VariableResSelector::_bind_methods() {
	GDBIND_BEGIN(VariableResSelector);
	GDBIND_CALBACK(_btn_pressed);
	GDBIND_CALBACK(_menu_index_pressed);
}

void VariableResSelector::_btn_pressed() {
	if (updating) {
		return;
	}
	ERR_FAIL_COND(!hfsm);
	menu->clear();
	Array variable_list = hfsm->get("variable_list");
	for (size_t i = 0; i < variable_list.size(); i++) {
		Ref<HFSMVariableRes> vr = variable_list[i];
		if (to_compare.is_valid() &&
				(to_compare == vr || to_compare->get_type() != vr->get_type())) {
			continue;
		}
		menu->add_item(
				vformat("%s: %s%s",
						vr->get_variable_name(),
						get_type_text(static_cast<Variant::Type>(vr->get_type())),
						vr->get_comment().is_empty() ? "" : (" - " + vr->get_comment())),
				i + 1);
		menu->set_item_metadata(menu->get_item_count() - 1, vr);
	}
	menu->reset_size();

	menu->set_position(btn->get_screen_position() + Vector2i(0, 1) * btn->get_size().y);
	menu->popup();
}

void VariableResSelector::_menu_index_pressed(int32_t p_index) {
	if (updating) {
		return;
	}
	Ref<HFSMVariableRes> vr = menu->get_item_metadata(p_index);
	auto obj = get_edited_object();
	auto prop = get_edited_property();
	if (to_compare.is_valid() && vr.is_valid() &&
			(to_compare == vr || to_compare->get_type() != vr->get_type())) {
		return;
	}
	emit_changed(prop, vr);
}

String VariableResSelector::get_type_text(Variant::Type p_type) {
	switch (p_type) {
		case Variant::NIL:
			return "Trigger";
		case Variant::BOOL:
			return "Bool";
		case Variant::INT:
			return "Int";
		case Variant::STRING:
			return "String";
		default:
			ERR_FAIL_V_MSG("Unknowm", "HFSM: Invald type: %d" + itos(p_type));
	}
}

VariableResSelector::VariableResSelector() {
	btn = memnew(Button);
	btn->set_text(NULL_TEXT);
	btn->connect(SNAME("pressed"), TCALLABLE(_btn_pressed));
	add_child(btn);

	menu = memnew(PopupMenu);
	menu->add_item(NULL_TEXT, 0);
	menu->set_item_metadata(0, Variant());
	menu->connect(SNAME("index_pressed"), TCALLABLE(_menu_index_pressed));
	add_child(menu);
}

VariableResSelector::VariableResSelector(HFSM *p_hfsm, const Ref<HFSMVariableRes> &p_to_compare) :
		VariableResSelector() {
	if (hfsm) {
		return;
	}
	hfsm = p_hfsm;
	if (to_compare.is_valid()) {
		to_compare = p_to_compare;
	}
}

void VariableResSelector::update_property_internal() {
	updating = true;
	Ref<HFSMVariableRes> vr = get_edited_object()->get(get_edited_property());
	if (vr.is_valid()) {
		btn->set_text(
				vformat("%s: %s", vr->get_variable_name(),
						get_type_text(static_cast<Variant::Type>(vr->get_type()))));
	} else {
		btn->set_text(NULL_TEXT);
	}
	updating = false;
}

}; // namespace Hfsm