#include <wozzits/script/v8_script_host.h>

#include <libplatform/libplatform.h>
#include <v8-context.h>
#include <v8-exception.h>
#include <v8-initialization.h>
#include <v8-isolate.h>
#include <v8-local-handle.h>
#include <v8-primitive.h>
#include <v8-script.h>
#include <libplatform/libplatform.h>

#include <cassert>
#include <cstdlib>
#include <memory>
#include <string>
#include <string_view>

namespace wz::script
{
    namespace
    {
#include <cstring>

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

            return make_string("V8 error");
        }

        class V8ScriptHost final : public ScriptHost
        {
        public:
            V8ScriptHost() = default;

            ~V8ScriptHost() override
            {
                shutdown();
            }

            bool initialize() override
            {
                if (initialized_)
                    return true;

                // With v8_use_external_startup_data=false, these are not expected
                // to require external snapshot/blob files, but calling the official
                // initialization hooks keeps us aligned with V8's embedding pattern.
                v8::V8::InitializeICUDefaultLocation(nullptr);
                v8::V8::InitializeExternalStartupData(nullptr);

                platform_ = v8::platform::NewDefaultPlatform();

                if (!platform_)
                    return false;

                v8::V8::InitializePlatform(platform_.get());
                v8::V8::InitializeICUDefaultLocation(nullptr);
                v8::V8::InitializeExternalStartupData(nullptr);
                v8::V8::Initialize();


                create_params_.array_buffer_allocator =
                    v8::ArrayBuffer::Allocator::NewDefaultAllocator();

                isolate_ = v8::Isolate::New(create_params_);

                if (isolate_ == nullptr)
                {
                    delete create_params_.array_buffer_allocator;
                    create_params_.array_buffer_allocator = nullptr;
                    platform_.reset();
                    return false;
                }

                {
                    v8::Isolate::Scope isolate_scope(isolate_);
                    v8::HandleScope handle_scope(isolate_);

                    v8::Local<v8::Context> local_context =
                        v8::Context::New(isolate_);

                    context_.Reset(isolate_, local_context);
                }

                initialized_ = true;
                return true;
            }

            void shutdown() override
            {
                if (!initialized_)
                    return;

                assert(isolate_ != nullptr);

                context_.Reset();

                isolate_->Dispose();
                isolate_ = nullptr;

                delete create_params_.array_buffer_allocator;
                create_params_.array_buffer_allocator = nullptr;

                v8::V8::Dispose();
                v8::V8::DisposePlatform();
                platform_.reset();

                initialized_ = false;
            }

            ScriptResult run_source(
                std::string_view name,
                std::string_view source) override
            {
                ScriptResult result;

                if (!initialized_ || isolate_ == nullptr)
                {
                    result.ok = false;
                    result.error = make_string("Unknown V8 error");
                    return result;
                }

                v8::Isolate::Scope isolate_scope(isolate_);
                v8::HandleScope handle_scope(isolate_);

                v8::Local<v8::Context> local_context =
                    context_.Get(isolate_);

                v8::Context::Scope context_scope(local_context);
                v8::TryCatch try_catch(isolate_);

                v8::MaybeLocal<v8::String> maybe_source =
                    v8::String::NewFromUtf8(
                        isolate_,
                        source.data(),
                        v8::NewStringType::kNormal,
                        static_cast<int>(source.size()));

                v8::Local<v8::String> v8_source;
                if (!maybe_source.ToLocal(&v8_source))
                {
                    result.ok = false;
                    result.error = exception_to_string(isolate_, try_catch);
                    return result;
                }

                v8::MaybeLocal<v8::String> maybe_name =
                    v8::String::NewFromUtf8(
                        isolate_,
                        name.data(),
                        v8::NewStringType::kNormal,
                        static_cast<int>(name.size()));

                v8::Local<v8::String> v8_name;
                if (!maybe_name.ToLocal(&v8_name))
                {
                    result.ok = false;
                    result.error = exception_to_string(isolate_, try_catch);
                    return result;
                }

                v8::ScriptOrigin origin(v8_name);

                v8::ScriptCompiler::Source script_source(v8_source, origin);

                v8::MaybeLocal<v8::Script> maybe_script =
                    v8::ScriptCompiler::Compile(local_context, &script_source);

                v8::Local<v8::Script> script;
                if (!maybe_script.ToLocal(&script))
                {
                    result.ok = false;
                    result.error = exception_to_string(isolate_, try_catch);
                    return result;
                }

                v8::MaybeLocal<v8::Value> maybe_value =
                    script->Run(local_context);

                v8::Local<v8::Value> value;
                if (!maybe_value.ToLocal(&value))
                {
                    result.ok = false;
                    result.error = exception_to_string(isolate_, try_catch);
                    return result;
                }

                result.ok = true;
                result.value = to_string(isolate_, local_context, value);
                return result;
            }

        private:
            bool initialized_ = false;

            std::unique_ptr<v8::Platform> platform_;
            v8::Isolate::CreateParams create_params_{};
            v8::Isolate* isolate_ = nullptr;
            v8::Global<v8::Context> context_;
        };
    }

    std::unique_ptr<ScriptHost> create_v8_script_host()
    {
        return std::unique_ptr<ScriptHost>(new V8ScriptHost());
    }
}