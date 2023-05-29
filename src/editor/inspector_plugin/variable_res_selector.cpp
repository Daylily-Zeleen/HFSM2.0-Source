#include "variable_res_selector.hpp"

#include "core/hfsm.hpp"
#include "core/hfsm_variable_res.hpp"

namespace Hfsm {

void VariableResSelector::_bind_methods() {
	ClassDB::bind_method(D_METHOD("__on_btn_pressed"),
			&VariableResSelector::__on_btn_pressed);
	ClassDB::bind_method(D_METHOD("__on_menu_index_pressed"),
			&VariableResSelector::__on_menu_index_pressed);
}

void VariableResSelector::__on_btn_pressed() {
	if (updating) {
		return;
	}
	menu->set_position(btn->get_global_position() +
			Vector2i(0, 1) * btn->get_size().y * 2);
	menu->popup();
}

void VariableResSelector::__on_menu_index_pressed(int32_t index) {
	if (updating) {
		return;
	}
	Ref<HFSMVariableRes> vr = menu->get_item_metadata(index);
	auto obj = get_edited_object();
	auto prop = get_edited_property();
	if (_to_compare.is_valid() && vr.is_valid() &&
			(_to_compare == vr || _to_compare->get_type() != vr->get_type())) {
		return;
	}
	emit_changed(prop, vr);
}

// String VariableResSelector::_to_string() const {
//     return String("[HfsmEditorPlugin:{0}]")
//         .replace("{0}", itos(get_instance_id()));
// }
String VariableResSelector::_get_type_text(Variant::Type type) {
	switch (type) {
		case Variant::NIL:
			return "Trigger";
		case Variant::BOOL:
			return "Bool";
		case Variant::INT:
			return "Int";
		case Variant::STRING:
			return "String";
		default:
			UtilityFunctions::printerr("HFSM: Invald type: %d", type);
			return "Unknowm";
	}
}

VariableResSelector::VariableResSelector() {
	btn = memnew(Button);
	btn->set_text(NULL_TEXT);
	btn->connect("pressed", Callable(this, "__on_btn_pressed"));
	add_child(btn);

	menu = memnew(PopupMenu);
	menu->add_item(NULL_TEXT, 0);
	menu->set_item_metadata(0, Variant());
	add_child(menu);
}

void VariableResSelector::setup(HFSM *hfsm, const Ref<HFSMVariableRes> &to_compare) {
	if (_hfsm) {
		return;
	}
	_hfsm = hfsm;
	if (to_compare.is_valid()) {
		_to_compare = to_compare;
	}
	Array variable_list = _hfsm->get("variable_list");
	for (size_t i = 0; i < variable_list.size(); i++) {
		Ref<HFSMVariableRes> vr = variable_list[i];
		if (_to_compare.is_valid() &&
				(_to_compare == vr || _to_compare->get_type() != vr->get_type())) {
			continue;
		}
		menu->add_item(
				String("{0}: {1}{2}")
						.format(Array::make(
								vr->get_variable_name(),
								_get_type_text(static_cast<Variant::Type>(vr->get_type())),
								vr->get_comment().is_empty()
										? ""
										: (" - " + vr->get_comment()))),
				i + 1);
		menu->set_item_metadata(menu->get_item_count() - 1, vr);
	}
}

void VariableResSelector::_update_property() {
	updating = true;
	Ref<HFSMVariableRes> vr = get_edited_object()->get(get_edited_property());
	if (vr.is_valid()) {
		btn->set_text(
				String("{0}: {1}")
						.format(Array::make(vr->get_variable_name(),
								_get_type_text(static_cast<Variant::Type>(
										vr->get_type())))));
	} else {
		btn->set_text(NULL_TEXT);
	}
	updating = false;
}
}; // namespace Hfsm