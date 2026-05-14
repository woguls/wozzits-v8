#pragma once

#include <wozzits/script/script_host.h>

#include <libplatform/libplatform.h>

#include <v8-context.h>
#include <v8-isolate.h>
#include <v8-persistent-handle.h>

#include <string>
#include <vector>

namespace wz::script
{
    struct ScriptTextPanel
    {
        std::vector<char> title;
        std::vector<char> text;
    };

    struct ScriptStatsRow
    {
        std::vector<char> label;
        std::vector<char> value;
    };

    struct ScriptStatsPanel
    {
        std::vector<char> title;
        std::vector<ScriptStatsRow> rows;
    };

    struct ScriptButtonItem
    {
        std::vector<char> label;
        std::vector<char> action;
    };

    struct ScriptButtonPanel
    {
        std::vector<char> title;
        std::vector<ScriptButtonItem> buttons;
    };

    struct ScriptToolState
    {
        std::vector<ScriptTextPanel> pending_text_panels;
        std::vector<ScriptStatsPanel> pending_stats_panels;
        std::vector<ScriptButtonPanel> pending_button_panels;
    };

    struct ScriptHost
    {
        bool initialized = false;

        v8::Isolate::CreateParams create_params{};
        v8::Isolate* isolate = nullptr;
        v8::Global<v8::Context> context;

        std::string last_value;
        std::string last_error;

        std::vector<std::vector<char>> logs;
        ScriptToolState tools;
    };
}

namespace wz::script::internal
{
    std::string make_string(const char* text);

    std::string to_string(
        v8::Isolate* isolate,
        v8::Local<v8::Context> context,
        v8::Local<v8::Value> value);

    std::vector<char> to_bytes(
        v8::Isolate* isolate,
        v8::Local<v8::Context> context,
        v8::Local<v8::Value> value);

    std::string exception_to_string(
        v8::Isolate* isolate,
        v8::TryCatch& try_catch);

    RunSourceResult make_value_result(const ScriptHost* host);
    RunSourceResult make_error_result(const ScriptHost* host);

    void wz_log_callback(
        const v8::FunctionCallbackInfo<v8::Value>& args);

    void wz_tool_text_panel_callback(
        const v8::FunctionCallbackInfo<v8::Value>& args);

    void wz_tool_stats_panel_callback(
        const v8::FunctionCallbackInfo<v8::Value>& args);

    void wz_tool_button_panel_callback(
        const v8::FunctionCallbackInfo<v8::Value>& args);
}