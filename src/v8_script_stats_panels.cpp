#include "script_host_internal.h"

#include <v8-container.h>
#include <v8-context.h>
#include <v8-external.h>
#include <v8-function.h>
#include <v8-local-handle.h>
#include <v8-value.h>

namespace wz::script::internal
{
    void wz_tool_stats_panel_callback(
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

        ScriptStatsPanel panel{};

        if (args.Length() > 0)
            panel.title = to_bytes(isolate, context, args[0]);

        if (args.Length() > 1 && args[1]->IsArray())
        {
            v8::Local<v8::Array> rows =
                v8::Local<v8::Array>::Cast(args[1]);

            const uint32_t count = rows->Length();

            for (uint32_t i = 0; i < count; ++i)
            {
                v8::Local<v8::Value> row_val;
                if (!rows->Get(context, i).ToLocal(&row_val))
                    continue;

                if (!row_val->IsArray())
                    continue;

                v8::Local<v8::Array> row =
                    v8::Local<v8::Array>::Cast(row_val);

                ScriptStatsRow entry{};

                v8::Local<v8::Value> label_val;
                if (row->Get(context, 0).ToLocal(&label_val))
                    entry.label = to_bytes(isolate, context, label_val);

                v8::Local<v8::Value> value_val;
                if (row->Get(context, 1).ToLocal(&value_val))
                    entry.value = to_bytes(isolate, context, value_val);

                panel.rows.push_back(std::move(entry));
            }
        }

        host->tools.pending_stats_panels.push_back(std::move(panel));
    }
}

namespace wz::script
{
    void clear_pending_stats_panels(ScriptHost* host)
    {
        if (host == nullptr)
            return;

        host->tools.pending_stats_panels.clear();
    }

    std::size_t pending_stats_panel_count(const ScriptHost* host)
    {
        if (host == nullptr)
            return 0;

        return host->tools.pending_stats_panels.size();
    }

    const char* pending_stats_panel_title(
        const ScriptHost* host,
        std::size_t index,
        std::size_t* out_size)
    {
        if (out_size != nullptr)
            *out_size = 0;

        if (host == nullptr)
            return nullptr;

        if (index >= host->tools.pending_stats_panels.size())
            return nullptr;

        const std::vector<char>& title =
            host->tools.pending_stats_panels[index].title;

        if (title.empty())
            return nullptr;

        if (out_size != nullptr)
            *out_size = title.size() - 1;

        return title.data();
    }

    std::size_t pending_stats_panel_row_count(
        const ScriptHost* host,
        std::size_t panel_index)
    {
        if (host == nullptr)
            return 0;

        if (panel_index >= host->tools.pending_stats_panels.size())
            return 0;

        return host->tools.pending_stats_panels[panel_index].rows.size();
    }

    const char* pending_stats_panel_row_label(
        const ScriptHost* host,
        std::size_t panel_index,
        std::size_t row_index,
        std::size_t* out_size)
    {
        if (out_size != nullptr)
            *out_size = 0;

        if (host == nullptr)
            return nullptr;

        if (panel_index >= host->tools.pending_stats_panels.size())
            return nullptr;

        const auto& rows =
            host->tools.pending_stats_panels[panel_index].rows;

        if (row_index >= rows.size())
            return nullptr;

        const std::vector<char>& label = rows[row_index].label;

        if (label.empty())
            return nullptr;

        if (out_size != nullptr)
            *out_size = label.size() - 1;

        return label.data();
    }

    const char* pending_stats_panel_row_value(
        const ScriptHost* host,
        std::size_t panel_index,
        std::size_t row_index,
        std::size_t* out_size)
    {
        if (out_size != nullptr)
            *out_size = 0;

        if (host == nullptr)
            return nullptr;

        if (panel_index >= host->tools.pending_stats_panels.size())
            return nullptr;

        const auto& rows =
            host->tools.pending_stats_panels[panel_index].rows;

        if (row_index >= rows.size())
            return nullptr;

        const std::vector<char>& value = rows[row_index].value;

        if (value.empty())
            return nullptr;

        if (out_size != nullptr)
            *out_size = value.size() - 1;

        return value.data();
    }
}
