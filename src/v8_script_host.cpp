// src/v8_script_host.cpp

#include "script_host_internal.h"

#include <libplatform/libplatform.h>

#include <v8-context.h>
#include <v8-exception.h>
#include <v8-initialization.h>
#include <v8-isolate.h>
#include <v8-local-handle.h>
#include <v8-primitive.h>
#include <v8-script.h>
#include <v8-external.h>
#include <v8-function.h>
#include <v8-template.h>

#include <cassert>
#include <cstdio>
#include <cstring>




namespace wz::script::internal 
{
    std::string make_string(const char* text)
    {
        if (text == nullptr)
            return {};

        return std::string(text, std::strlen(text));
    }

    void append_literal(std::string& out, const char* text)
    {
        if (text == nullptr)
            return;

        out.append(text, std::strlen(text));
    }

    std::string to_string(
        v8::Isolate* isolate,
        v8::Local<v8::Context> context,
        v8::Local<v8::Value> value)
    {
        if (value.IsEmpty())
            return {};

        v8::Local<v8::String> str;

        if (!value->ToString(context).ToLocal(&str))
            return {};

        v8::String::Utf8Value utf8(isolate, str);

        if (*utf8 == nullptr || utf8.length() <= 0)
            return {};

        const char* data = *utf8;
        const size_t size = static_cast<size_t>(utf8.length());

        return std::string(data, data + size);
    }

    std::string exception_to_string(
        v8::Isolate* isolate,
        v8::TryCatch& try_catch)
    {
        if (!try_catch.HasCaught())
            return make_string("Unknown V8 error");

        v8::Local<v8::Context> context = isolate->GetCurrentContext();

        if (!try_catch.Exception().IsEmpty())
        {
            return to_string(isolate, context, try_catch.Exception());
        }

        return internal::make_string("V8 error");
    }


    wz::script::RunSourceResult make_value_result(
        const wz::script::ScriptHost* host)
    {
        wz::script::RunSourceResult result{};
        result.ok = true;
        result.value = host->last_value.c_str();
        result.value_size = host->last_value.size();
        return result;
    }

    wz::script::RunSourceResult make_error_result(
        const wz::script::ScriptHost* host)
    {
        wz::script::RunSourceResult result{};
        result.ok = false;
        result.error = host->last_error.c_str();
        result.error_size = host->last_error.size();
        return result;
    }
}

namespace wz::script
{
    ScriptHost* create_v8_script_host()
    {
        return new ScriptHost();
    }

    void destroy_v8_script_host(ScriptHost* host)
    {
        if (host == nullptr)
            return;

        shutdown(host);
        delete host;
    }

    bool initialize(ScriptHost* host)
    {
        if (host == nullptr)
            return false;

        if (host->initialized)
            return true;

        v8::V8::InitializeICUDefaultLocation(nullptr);
        v8::V8::InitializeExternalStartupData(nullptr);

        host->platform = v8::platform::NewDefaultPlatform();

        if (!host->platform)
            return false;

        v8::V8::InitializePlatform(host->platform.get());
        v8::V8::Initialize();

        host->create_params.array_buffer_allocator =
            v8::ArrayBuffer::Allocator::NewDefaultAllocator();

        host->isolate = v8::Isolate::New(host->create_params);

        if (host->isolate == nullptr)
        {
            delete host->create_params.array_buffer_allocator;
            host->create_params.array_buffer_allocator = nullptr;

            v8::V8::Dispose();
            v8::V8::DisposePlatform();

            host->platform.reset();

            return false;
        }

        {
            v8::Isolate::Scope isolate_scope(host->isolate);
            v8::HandleScope handle_scope(host->isolate);

            v8::Local<v8::ObjectTemplate> global_template =
                v8::ObjectTemplate::New(host->isolate);

            v8::Local<v8::ObjectTemplate> wz_template =
                v8::ObjectTemplate::New(host->isolate);

            v8::Local<v8::External> host_external =
                v8::External::New(
                    host->isolate,
                    host,
                    v8::kExternalPointerTypeTagDefault);

            v8::Local<v8::FunctionTemplate> log_function =
                v8::FunctionTemplate::New(
                    host->isolate,
                    internal::wz_log_callback,
                    host_external);

            v8::Local<v8::String> log_name =
                v8::String::NewFromUtf8Literal(host->isolate, "log");

            wz_template->Set(log_name, log_function);

            v8::Local<v8::ObjectTemplate> tool_template =
                v8::ObjectTemplate::New(host->isolate);

            v8::Local<v8::FunctionTemplate> text_panel_function =
                v8::FunctionTemplate::New(
                    host->isolate,
                    internal::wz_tool_text_panel_callback,
                    host_external);

            v8::Local<v8::String> text_panel_name =
                v8::String::NewFromUtf8Literal(host->isolate, "textPanel");

            tool_template->Set(text_panel_name, text_panel_function);

            v8::Local<v8::String> tool_name =
                v8::String::NewFromUtf8Literal(host->isolate, "tool");

            wz_template->Set(tool_name, tool_template);

            v8::Local<v8::String> wz_name =
                v8::String::NewFromUtf8Literal(host->isolate, "wz");

            global_template->Set(wz_name, wz_template);

            v8::Local<v8::Context> local_context =
                v8::Context::New(
                    host->isolate,
                    nullptr,
                    global_template);

            host->context.Reset(host->isolate, local_context);
        }

        host->initialized = true;
        return true;
    }

    void shutdown(ScriptHost* host)
    {
        if (host == nullptr || !host->initialized)
            return;

        host->context.Reset();

        if (host->isolate != nullptr)
        {
            host->isolate->Dispose();
            host->isolate = nullptr;
        }

        delete host->create_params.array_buffer_allocator;
        host->create_params.array_buffer_allocator = nullptr;

        v8::V8::Dispose();
        v8::V8::DisposePlatform();

        host->platform.reset();

        host->last_value.clear();
        host->last_error.clear();
        host->logs.clear();
        host->tools.pending_text_panels.clear();

        host->initialized = false;
    }

    RunSourceResult run_source(
        ScriptHost* host,
        const char* name,
        const char* source)
    {
        RunSourceResult result{};

        if (host == nullptr)
        {
            result.ok = false;
            result.error = "ScriptHost is null";
            result.error_size = std::strlen(result.error);
            return result;
        }

        host->last_value.clear();
        host->last_error.clear();

        if (!host->initialized || host->isolate == nullptr)
        {
            host->last_error = internal::make_string("V8 ScriptHost is not initialized");
            return internal::make_error_result(host);
        }

        if (name == nullptr)
            name = "<source>";

        if (source == nullptr)
            source = "";

        v8::Isolate::Scope isolate_scope(host->isolate);
        v8::HandleScope handle_scope(host->isolate);

        v8::Local<v8::Context> local_context =
            host->context.Get(host->isolate);

        v8::Context::Scope context_scope(local_context);
        v8::TryCatch try_catch(host->isolate);

        v8::MaybeLocal<v8::String> maybe_source =
            v8::String::NewFromUtf8(
                host->isolate,
                source,
                v8::NewStringType::kNormal,
                static_cast<int>(std::strlen(source)));

        v8::Local<v8::String> v8_source;
        if (!maybe_source.ToLocal(&v8_source))
        {
            host->last_error =
                internal::exception_to_string(host->isolate, try_catch);

            return internal::make_error_result(host);
        }

        v8::MaybeLocal<v8::String> maybe_name =
            v8::String::NewFromUtf8(
                host->isolate,
                name,
                v8::NewStringType::kNormal,
                static_cast<int>(std::strlen(name)));

        v8::Local<v8::String> v8_name;
        if (!maybe_name.ToLocal(&v8_name))
        {
            host->last_error =
                internal::exception_to_string(host->isolate, try_catch);

            return internal::make_error_result(host);
        }

        v8::ScriptOrigin origin(v8_name);
        v8::ScriptCompiler::Source script_source(v8_source, origin);

        v8::MaybeLocal<v8::Script> maybe_script =
            v8::ScriptCompiler::Compile(local_context, &script_source);

        v8::Local<v8::Script> script;
        if (!maybe_script.ToLocal(&script))
        {
            host->last_error =
                internal::exception_to_string(host->isolate, try_catch);

            return internal::make_error_result(host);
        }

        v8::MaybeLocal<v8::Value> maybe_value =
            script->Run(local_context);

        v8::Local<v8::Value> value;
        if (!maybe_value.ToLocal(&value))
        {
            host->last_error =
                internal::exception_to_string(host->isolate, try_catch);

            return internal::make_error_result(host);
        }

        host->last_value =
            internal::to_string(host->isolate, local_context, value);

        return internal::make_value_result(host);
    }


} // namespace wz::script