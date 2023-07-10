#include "variable_res_selector.h"

#include "../../src/hfsm.h"
#include "../../src/hfsm_variable_res.h"

namespace Hfsm {

void _VariableResSelector::GD_(set_create_options)(Object *p_menu_node) {
	PopupMenu *menu = Object::cast_to<PopupMenu>(p_menu_node);
	if (!menu) {
		return;
	}

	Array variable_list = hfsm->get("variable_list");

	variable_res_list.clear();
	auto idx = 0;
	for (size_t i = 0; i < variable_list.size(); i++) {
		Ref<HFSMVariableRes> vr = variable_list[i];
		if (to_compare.is_valid() &&
				(to_compare == vr || to_compare->get_type() != vr->get_type())) {
			continue;
		}
		menu->add_item(
				vformat("%s: %s%s",
						vr->get_variable_name(),
						vr->get_type_text(),
						vr->get_comment().is_empty() ? "" : (" - " + vr->get_comment())),
				idx + op_ofs);

		variable_res_list.push_back(vr);
		idx += 1;
	}

	menu->add_separator();
}

bool _VariableResSelector::GD_(handle_menu_selected)(int p_which) {
	auto idx = p_which - op_ofs;
	if (idx > 0 && idx < variable_res_list.size()) {
		set_edited_resource(variable_res_list[idx]);
		emit_signal(SNAME("resource_changed"), variable_res_list[idx]);
		return true;
	}

	return false;
}

_VariableResSelector::_VariableResSelector(HFSM *p_hfsm, const Ref<HFSMVariableRes> &p_to_compare) {
	set_base_type(HFSMVariableRes::get_class_static());
	hfsm = p_hfsm;
	if (to_compare.is_valid()) {
		to_compare = p_to_compare;
	}
}

//
void VariableResSelector::_bind_methods() {
	GDBIND_BEGIN(VariableResSelector);
	GDBIND_CALBACK(_variable_selected);
}

void VariableResSelector::_variable_selected(const Ref<Resource> &p_res) {
	if (updating) {
		return;
	}
	Ref<HFSMVariableRes> vr = p_res;
	auto obj = get_edited_object();
	auto prop = get_edited_property();
	if (to_compare.is_valid() && (to_compare == vr || to_compare->get_type() != vr->get_type())) {
		return;
	}
	emit_changed(prop, vr);
}

VariableResSelector::VariableResSelector() {
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
	selector = memnew(_VariableResSelector(p_hfsm, p_to_compare));
	selector->connect("resource_changed", TCALLABLE(_variable_selected));
	selector->set_h_size_flags(SizeFlags::SIZE_EXPAND_FILL);
	add_child(selector);
}

void VariableResSelector::update_property_internal() {
	updating = true;
	Ref<HFSMVariableRes> vr = get_edited_object()->get(get_edited_property());
	selector->set_edited_resource(vr);
	updating = false;
}

}; // namespace Hfsm