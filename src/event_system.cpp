#include "event_system.h"

#include "godot_cpp/variant/callable.hpp"
#include "godot_cpp/variant/packed_string_array.hpp"
#include "tag_manager.h"
#include <algorithm>

using namespace godot;

void EventSystem::_bind_methods()
{
    ClassDB::bind_method(D_METHOD("watch_event", "event_tag", "callback"), &EventSystem::watch_event);
    ClassDB::bind_method(D_METHOD("unwatch_event", "event_tag", "callback"), &EventSystem::unwatch_event);
    ClassDB::bind_method(D_METHOD("emit_event", "event_tag", "payload"),&EventSystem::emit_event);
}

void EventSystem::_ready()
{
    _tag_manager = TagManager::get_singleton();
    ERR_FAIL_COND_MSG(!_tag_manager, "EventSystem_ready: TagManager not available.");
}

void EventSystem::watch_event(const StringName& p_event_tag, const Callable& p_callback)
{
    uint32_t idx = _tag_manager->get_tag_index(p_event_tag);

    ERR_FAIL_COND_MSG(idx == TagManager::INVALID_INDEX,
        vformat("EventSystem::watch_event: '%s' is not a registered tag.",
            String(p_event_tag)));
 
    _event_watchers[idx].push_back(p_callback);
}

void EventSystem::unwatch_event(const StringName& p_event_tag, const Callable& p_callback)
{
    uint32_t idx = _tag_manager->get_tag_index(p_event_tag);

    ERR_FAIL_COND_MSG(idx == TagManager::INVALID_INDEX,
        vformat("EventSystem::unwatch_event: '%s' is not a registered tag.",
            String(p_event_tag)));

    auto it = _event_watchers.find(idx);
    if (it == _event_watchers.end()) return;

    auto& callbacks = it->second;
    callbacks.erase(
        std::remove_if(
            callbacks.begin(), 
            callbacks.end(), 
            [&](const Callable& callback) {
                return callback == p_callback;
            }
        ),
        callbacks.end()
    );
}

void EventSystem::emit_event(const StringName& p_event_tag, const Variant& p_payload)
{
    _notify_event_watchers(p_event_tag, p_payload);
}

void EventSystem::_notify_event_watchers(const StringName& p_event_tag, const Variant& p_payload)
{
    _notify_event_watcher(p_event_tag, p_payload);

    PackedStringArray parents = _tag_manager->get_parent_tags(p_event_tag);
    for (int i = 0; i < parents.size(); i++) {
        _notify_event_watcher(StringName(parents[i]), p_payload);
    }
}

void EventSystem::_notify_event_watcher(const StringName& p_event_tag, const Variant& p_payload)
{
    uint32_t idx = _tag_manager->get_tag_index(p_event_tag);
    if (idx == TagManager::INVALID_INDEX) return;

    auto it = _event_watchers.find(idx);
    if (it == _event_watchers.end()) return;

    auto& callbacks = it->second;
    size_t write = 0;
    for (size_t read = 0; read < callbacks.size(); read++) {
        if (callbacks[read].is_valid()) {
            callbacks[read].call(p_payload);
            
            if (write != read) callbacks[write] = callbacks[read];
            write++;
        }
    }

    callbacks.resize(write);
}