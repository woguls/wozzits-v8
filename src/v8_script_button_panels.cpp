#include "script_host_internal.h"

#include <v8-container.h>
#include <v8-context.h>
#include <v8-external.h>
#include <v8-function.h>
#include <v8-local-handle.h>
#include <v8-value.h>

namespace wz::script::internal
{
    void wz_tool_button_panel_callback(
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

        ScriptButtonPanel panel{};

        if (args.Length() > 0)
            panel.title = to_bytes(isolate, context, args[0]);

        if (args.Length() > 1 && args[1]->IsArray())
        {
            v8::Local<v8::Array> buttons =
                v8::Local<v8::Array>::Cast(args[1]);

            const uint32_t count = buttons->Length();

            for (uint32_t i = 0; i < count; ++i)
            {
                v8::Local<v8::Value> btn_val;
                if (!buttons->Get(context, i).ToLocal(&btn_val))
                    continue;

                if (!btn_val->IsArray())
                    continue;

                v8::Local<v8::Array> btn =
                    v8::Local<v8::Array>::Cast(btn_val);

                ScriptButtonItem item{};

                v8::Local<v8::Value> label_val;
                if (btn->Get(context, 0).ToLocal(&label_val))
                    item.label = to_bytes(isolate, context, label_val);

                v8::Local<v8::Value> action_val;
                if (btn->Get(context, 1).ToLocal(&action_val))
                    item.action = to_bytes(isolate, context, action_val);

                panel.buttons.push_back(std::move(item));
            }
        }

        host->tools.pending_button_panels.push_back(std::move(panel));
    }
}

namespace wz::script
{
    void clear_pending_button_panels(ScriptHost* host)
    {
        if (host == nullptr)
            return;

        host->tools.pending_button_panels.clear();
    }

    std::size_t pending_button_panel_count(const ScriptHost* host)
    {
        if (host == nullptr)
            return 0;

        return host->tools.pending_button_panels.size();
    }

    const char* pending_button_panel_title(
        const ScriptHost* host,
        std::size_t index,
        std::size_t* out_size)
    {
        if (out_size != nullptr)
            *out_size = 0;

        if (host == nullptr)
            return nullptr;

        if (index >= host->tools.pending_button_panels.size())
            return nullptr;

        const std::vector<char>& title =
            host->tools.pending_button_panels[index].title;

        if (title.empty())
            return nullptr;

        if (out_size != nullptr)
            *out_size = title.size() - 1;

        return title.data();
    }

    std::size_t pending_button_panel_button_count(
        const ScriptHost* host,
        std::size_t panel_index)
    {
        if (host == nullptr)
            return 0;

        if (panel_index >= host->tools.pending_button_panels.size())
            return 0;

        return host->tools.pending_button_panels[panel_index].buttons.size();
    }

    const char* pending_button_panel_button_label(
        const ScriptHost* host,
        std::size_t panel_index,
        std::size_t button_index,
        std::size_t* out_size)
    {
        if (out_size != nullptr)
            *out_size = 0;

        if (host == nullptr)
            return nullptr;

        if (panel_index >= host->tools.pending_button_panels.size())
            return nullptr;

        const auto& buttons =
            host->tools.pending_button_panels[panel_index].buttons;

        if (button_index >= buttons.size())
            return nullptr;

        const std::vector<char>& label = buttons[button_index].label;

        if (label.empty())
            return nullptr;

        if (out_size != nullptr)
            *out_size = label.size() - 1;

        return label.data();
    }

    const char* pending_button_panel_button_action(
        const ScriptHost* host,
        std::size_t panel_index,
        std::size_t button_index,
        std::size_t* out_size)
    {
        if (out_size != nullptr)
            *out_size = 0;

        if (host == nullptr)
            return nullptr;

        if (panel_index >= host->tools.pending_button_panels.size())
            return nullptr;

        const auto& buttons =
            host->tools.pending_button_panels[panel_index].buttons;

        if (button_index >= buttons.size())
            return nullptr;

        const std::vector<char>& action = buttons[button_index].action;

        if (action.empty())
            return nullptr;

        if (out_size != nullptr)
            *out_size = action.size() - 1;

        return action.data();
    }
}
