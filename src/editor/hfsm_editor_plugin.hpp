#ifndef HFSM_EDITOR_PLUGIN_H
#define HFSM_EDITOR_PLUGIN_H

#include <godot_cpp/classes/editor_inspector_plugin.hpp>
#include <godot_cpp/classes/editor_plugin.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/translation_server.hpp>

#include <godot_cpp/classes/editor_undo_redo_manager.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include <godot_cpp/templates/hash_map.hpp>

#include "hfsm_editor.hpp"

#ifndef HACK
#define HACK
#include "src/core/hfsm.hpp"
#endif

using namespace godot;
namespace Hfsm {
class HFSM;

class HfsmInspectorPlugin : public EditorInspectorPlugin {
    GDCLASS(HfsmInspectorPlugin, EditorInspectorPlugin)
protected:
    static void _bind_methods();

public:
    HfsmInspectorPlugin();
    [[nodiscard]] bool _can_handle(const Variant &object) const override;
    bool _parse_property(Object *object, Variant::Type type, const String &name, PropertyHint hint_type, const String &hint_string, BitField<PropertyUsageFlags> usage_flags, bool wide) override;
};

#ifndef HACK
class HfsmEditorPlugin : public EditorPlugin {
    GDCLASS(HfsmEditorPlugin, EditorPlugin)
protected:
    static void _bind_methods();
    String _to_string() const;

private:
    static HfsmEditorPlugin *instance;
    Ref<HfsmInspectorPlugin> inspector_plugin;
    HFSMEditor *hfsm_editor = nullptr;
    Button *hfsm_editor_btn = nullptr;

    friend class HfsmInspectorPlugin;

    // ===== HACK ======
    friend class HfsmEditorPluginBridge;

public:
    HfsmEditorPlugin();

    static HfsmEditorPlugin *get_singleton() { return instance; }

    virtual bool _handles(const Variant &object) const;

    void _enter_tree();
    void _exit_tree();

    HFSMEditor *get_hfsm_editor() { return hfsm_editor; }
    static bool is_zh();
};
#else
class HfsmEditorPlugin : public RefCounted {
    GDCLASS(HfsmEditorPlugin, RefCounted)
protected:
    static void _bind_methods() {
        ClassDB::bind_method(D_METHOD("initiate", "editor_plugin"), &HfsmEditorPlugin::initiate);
        ClassDB::bind_method(D_METHOD("handles", "object"), &HfsmEditorPlugin::handles);
        ClassDB::bind_method(D_METHOD("enter_tree_action"), &HfsmEditorPlugin::enter_tree_action);
        ClassDB::bind_method(D_METHOD("exit_tree_action"), &HfsmEditorPlugin::exit_tree_action);
    }

    [[nodiscard]] String _to_string() const { return String("[HfsmEditorPlugin:{0}]").replace("{0}", uitos(get_instance_id())); }

private:
    static HfsmEditorPlugin *instance;
    Ref<HfsmInspectorPlugin> inspector_plugin;
    HFSMEditor *hfsm_editor = nullptr;
    Button *hfsm_editor_btn = nullptr;

    friend class HfsmInspectorPlugin;

    EditorPlugin *plugin;

    HashMap<String, const char *> translation;

public:
    HfsmEditorPlugin() {
        translation.insert("HFSM Editor", "HFSM 编辑器");
        translation.insert("Plese select a 'HFSM' node to start edit.", "请选中一个 HFSM 节点开始编辑");
        translation.insert("HFSM: has duplicated State name: ", "HFSM: 存在重复的状态名称: ");
        translation.insert("Change state name", "改变状态名称");
        translation.insert(": this state is Entry State, can't set to other type.", ": 该状态当前为入口状态，不能设置为其他类型。");
        translation.insert("Change state type", "改变状态类型");
        translation.insert("set Sub-FSM", "设置子状态机");
        translation.insert("Attach state script", "附加状态脚本");
        translation.insert("resized", "重设尺寸");
        translation.insert("move states", "移动状态");
        translation.insert("Sub FSM", "子状态机");
        translation.insert("select states", "选择状态");
        translation.insert("Delete State Transitions", "删除状态转换");
        translation.insert("HFSM::Invalid FsmRes", "HFSM::非法情况，要编辑的 FsmRes 无效");
        translation.insert("The current FSM has not contain a State.\n\n ", "当前状态机不存在状态\n\n ");
        translation.insert("Click here to create a Entry State", "点击创建一个 起始状态");
        translation.insert("Add State", "添加状态");
        translation.insert("Cut States", "剪切状态");
        translation.insert("Copy States", "复制状态");
        translation.insert("Paste States", "粘贴状态");
        translation.insert("Duplicate States", "创建状态副本");
        translation.insert("Delete States", "删除状态");
        translation.insert("Delete State Transitions", "删除状态转换");
        translation.insert("Convert To Sub-FSM", "转换为子状态机");
        translation.insert("Create State Transition", "创建状态转换");
        translation.insert("Delete", "删除");
        translation.insert("Edit Sub-FSM", "编辑子状态机");
        translation.insert("Select State Transitions", "选择转换");
        translation.insert("Deselect", "取消选择");
        translation.insert(R"("value" can't convert to the type of "HFSMVariableRes".)", "\"value\"的无法转化为\"HFSMVariableRes\"的类型");
        translation.insert(R"("value" is not a valid "HFSMVariableRes".)", "\"value\" 不是一个有效的 \"HFSMVariableRes\"");
        translation.insert(R"("value" can't convert to the type of "variable_res".)", "\"value\"的无法转化为\"variable_res\"的类型");
        translation.insert("Trigger: ", "触发器: ");
        translation.insert("Solo Trigger: ", "独立触发器: ");
        translation.insert("Union Trigger: ", "联合触发器: ");
        translation.insert("Invalid Trigger Type:", "无效触发器类型");
        translation.insert("Has not valid 'variable_res'", "没有有效的变量资源");
        translation.insert("Script: ", "脚本: ");
        translation.insert("Script isn't extends from 'Transition'.", "脚本不是扩展自'Transition'");
        translation.insert("You can use other type of script if this is intended.", "如果是有意的，请使用其他类型的脚本");
        translation.insert("Script is invalid!", "脚本无效!");
        translation.insert("Empty expression!", "表达式为空!");
        translation.insert("Have not valid HFSMVariable Expression.", "没有合法的变量表达式");
        translation.insert("HFSMVariable Expressions: ", "变量表达式:");
        translation.insert("Invalid \"VariableExpressionRes\".", "无效的\"VariableExpressionRes\"");
        translation.insert("Auto: ", "自动: ");
        translation.insert("Delay ", "延迟 ");
        translation.insert(" msec.", " 毫秒");
        translation.insert("When sub Fsm exit.", "子状态机退出时");
        translation.insert("After calling \"manual_exit()\".", "调用\"manual_exit()\"后");
        translation.insert("After \"_update()\" being called ", "\"_update()\"被调用 ");
        translation.insert("After \"_physics_update()\" being called ", "\"_physics_update()\"被调用 ");
        translation.insert(" times.", " 次后");
    }

    void initiate(Object *editor_plugin) {
        plugin = static_cast<EditorPlugin *>(editor_plugin);
        inspector_plugin.instantiate();
        instance = this;
    }

    static HfsmEditorPlugin *get_singleton() { return instance; }

    EditorInterface *get_editor_interface() { return plugin->get_editor_interface(); }

    bool handles(const Variant &object) {
        auto obj = Object::cast_to<Node>(object);
        if (obj) {
            if (obj->is_class("HFSM")) {
                hfsm_editor->edit(Object::cast_to<HFSM>(obj));
            } else {
                hfsm_editor->edit(nullptr);
            }
            return true;
        }
        return false;
    }
    void enter_tree_action() {
        if (!Engine::get_singleton()->is_editor_hint()) {
            return;
        }
        hfsm_editor = HFSMEditor::create_hfsm_editor();
        hfsm_editor_btn = plugin->add_control_to_bottom_panel(hfsm_editor, str_localize("HFSM Editor"));
        plugin->add_inspector_plugin(inspector_plugin);
    }
    void exit_tree_action() {
        if (!Engine::get_singleton()->is_editor_hint()) {
            return;
        }
        plugin->remove_inspector_plugin(inspector_plugin);
        plugin->remove_control_from_bottom_panel(hfsm_editor);
        hfsm_editor->queue_free();
    }

    HFSMEditor *get_hfsm_editor() { return hfsm_editor; }

    static bool is_zh() { return TranslationServer::get_singleton()->get_tool_locale().substr(0, 2) == "zh"; }

    static String str_localize(const String &en_key) {
        if (is_zh() && get_singleton()->translation.has(en_key)) {
            return String::utf8(get_singleton()->translation[en_key]);
        }
        return en_key;
    }

    static EditorUndoRedoManager *create_action(const String &action_name) {
        auto undo_redo = get_singleton()->plugin->get_undo_redo();
        undo_redo->create_action(str_localize(action_name), UndoRedo::MERGE_DISABLE, get_singleton()->get_hfsm_editor()->get_editing_hfsm());
        return undo_redo;
    }
};
#endif

} // namespace Hfsm
#endif