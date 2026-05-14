#include "script_host_internal.h"

#include <v8-context.h>
#include <v8-external.h>
#include <v8-function.h>
#include <v8-local-handle.h>
#include <v8-value.h>

#include <string>

namespace wz::script::internal
{
    void wz_tool_text_panel_callback(
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

        ScriptTextPanel panel{};

        if (args.Length() > 0)
            panel.title = to_bytes(isolate, context, args[0]);

        if (args.Length() > 1)
            panel.text = to_bytes(isolate, context, args[1]);

        host->tools.pending_text_panels.push_back(std::move(panel));
    }
}

namespace wz::script
{
    void clear_pending_text_panels(ScriptHost* host)
    {
        if (host == nullptr)
            return;

        host->tools.pending_text_panels.clear();
    }

    std::size_t pending_text_panel_count(const ScriptHost* host)
    {
        if (host == nullptr)
            return 0;

        return host->tools.pending_text_panels.size();
    }

    const char* pending_text_panel_title(
        const ScriptHost* host,
        std::size_t index,
        std::size_t* out_size)
    {
        if (out_size != nullptr)
            *out_size = 0;

        if (host == nullptr)
            return nullptr;

        if (index >= host->tools.pending_text_panels.size())
            return nullptr;

        const std::vector<char>& title =
            host->tools.pending_text_panels[index].title;

        if (title.empty())
            return nullptr;

        if (out_size != nullptr)
            *out_size = title.size() - 1;

        return title.data();
    }

    const char* pending_text_panel_text(
        const ScriptHost* host,
        std::size_t index,
        std::size_t* out_size)
    {
        if (out_size != nullptr)
            *out_size = 0;

        if (host == nullptr)
            return nullptr;

        if (index >= host->tools.pending_text_panels.size())
            return nullptr;

        const std::vector<char>& text =
            host->tools.pending_text_panels[index].text;

        if (text.empty())
            return nullptr;

        if (out_size != nullptr)
            *out_size = text.size() - 1;

        return text.data();
    }
}
