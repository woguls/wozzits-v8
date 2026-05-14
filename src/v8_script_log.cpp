#include "script_host_internal.h"

#include <v8-external.h>
#include <v8-function.h>
#include <v8-local-handle.h>
#include <v8-value.h>

namespace wz::script::internal
{
    void wz_log_callback(
        const v8::FunctionCallbackInfo<v8::Value>& args)
    {
        v8::Isolate* isolate = args.GetIsolate();

        if (args.Data().IsEmpty() || !args.Data()->IsExternal())
            return;

        v8::Local<v8::External> external =
            v8::Local<v8::External>::Cast(args.Data());

        auto* host =
            static_cast<wz::script::ScriptHost*>(
                external->Value(v8::kExternalPointerTypeTagDefault));

        if (host == nullptr)
            return;

        v8::Local<v8::Context> context = isolate->GetCurrentContext();

        if (args.Length() <= 0)
        {
            host->logs.emplace_back();
            return;
        }

        host->logs.push_back(to_bytes(isolate, context, args[0]));
    }
}

namespace wz::script
{
    void clear_logs(ScriptHost* host)
    {
        if (host == nullptr)
            return;

        host->logs.clear();
    }

    std::size_t log_count(const ScriptHost* host)
    {
        if (host == nullptr)
            return 0;

        return host->logs.size();
    }

    const char* log_message(
        const ScriptHost* host,
        std::size_t index,
        std::size_t* out_size)
    {
        if (out_size != nullptr)
            *out_size = 0;

        if (host == nullptr)
            return nullptr;

        if (index >= host->logs.size())
            return nullptr;

        const std::vector<char>& message = host->logs[index];

        if (message.empty())
            return nullptr;

        if (out_size != nullptr)
            *out_size = message.size() - 1;

        return message.data();
    }
}
