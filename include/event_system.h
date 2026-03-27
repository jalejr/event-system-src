#pragma once

#include "godot_cpp/classes/node.hpp"
#include "godot_cpp/classes/wrapped.hpp"
#include "godot_cpp/variant/string_name.hpp"
#include "identity_hash.h"
#include "godot_cpp/variant/callable.hpp"
#include "tag_manager.h"
#include <cstdint>
#include <unordered_map>

using namespace godot;

class EventSystem : public Node {
    GDCLASS(EventSystem, Node)

    std::unordered_map<uint32_t, std::vector<Callable>, IdentityHash> _event_watchers;
    TagManager* _tag_manager = nullptr;

protected:
    static void _bind_methods();

public:
    EventSystem() = default;

    void _ready() override;

    void watch_event(const StringName& p_event_tag, const Callable& p_callback);
    void unwatch_event(const StringName& p_event_tag, const Callable& p_callback);
    void emit_event(const StringName& p_event_tag, const Variant& p_payload);

private:
    void _notify_event_watchers(const StringName& p_event_tag, const Variant& p_payload);
    void _notify_event_watcher(const StringName& p_event_tag, const Variant& p_payload);
};