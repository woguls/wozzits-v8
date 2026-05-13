#pragma once

#include <wozzits/script/script_host.h>

#include <libplatform/libplatform.h>

#include <v8-context.h>
#include <v8-isolate.h>
#include <v8-persistent-handle.h>

#include <memory>
#include <string>
#include <vector>

namespace wz::script
{
    struct ScriptTextPanel
    {
        std::string title;
        std::string text;
    };

    struct ScriptToolState
    {
        std::vector<ScriptTextPanel> pending_text_panels;
    };

    struct ScriptHost
    {
        bool initialized = false;

        std::unique_ptr<v8::Platform> platform;
        v8::Isolate::CreateParams create_params{};
        v8::Isolate* isolate = nullptr;
        v8::Global<v8::Context> context;

        std::string last_value;
        std::string last_error;

        std::vector<std::string> logs;
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

    std::string exception_to_string(
        v8::Isolate* isolate,
        v8::TryCatch& try_catch);

    RunSourceResult make_value_result(const ScriptHost* host);
    RunSourceResult make_error_result(const ScriptHost* host);

    void wz_log_callback(
        const v8::FunctionCallbackInfo<v8::Value>& args);

    void wz_tool_text_panel_callback(
        const v8::FunctionCallbackInfo<v8::Value>& args);
}