#include <wozzits/script/script_host.h>

#include <cassert>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

namespace
{
    struct ToolTextPanel
    {
        std::string title;
        std::string text;
    };

    struct ToolRegistry
    {
        std::vector<ToolTextPanel> text_panels;
    };

    std::string copy_script_string(const char* text, std::size_t size)
    {
        if (text == nullptr || size == 0)
            return {};

        return std::string(text, text + size);
    }

    void set_text_panel(
        ToolRegistry& tools,
        std::string title,
        std::string text)
    {
        for (ToolTextPanel& panel : tools.text_panels)
        {
            if (panel.title == title)
            {
                panel.text = std::move(text);
                return;
            }
        }

        tools.text_panels.push_back(
            ToolTextPanel{
                std::move(title),
                std::move(text),
            });
    }

    bool run_checked(
        wz::script::ScriptHost* host,
        const char* name,
        const char* source)
    {
        const wz::script::RunSourceResult result =
            wz::script::run_source(host, name, source);

        if (!result.ok)
        {
            std::printf(
                "script failed: %s\n",
                result.error ? result.error : "<null>");
            return false;
        }

        if (result.value != nullptr)
        {
            std::printf(
                "script result: %s\n",
                result.value);
        }

        return true;
    }

    void drain_script_logs(wz::script::ScriptHost* host)
    {
        const std::size_t count = wz::script::log_count(host);

        for (std::size_t i = 0; i < count; ++i)
        {
            std::size_t size = 0;
            const char* message =
                wz::script::log_message(host, i, &size);

            std::printf(
                "[script] %.*s\n",
                static_cast<int>(size),
                message ? message : "");
        }

        wz::script::clear_logs(host);
    }

    void apply_pending_text_panels(
        wz::script::ScriptHost* host,
        ToolRegistry& tools)
    {
        const std::size_t count =
            wz::script::pending_text_panel_count(host);

        for (std::size_t i = 0; i < count; ++i)
        {
            std::size_t title_size = 0;
            const char* title =
                wz::script::pending_text_panel_title(
                    host,
                    i,
                    &title_size);

            std::size_t text_size = 0;
            const char* text =
                wz::script::pending_text_panel_text(
                    host,
                    i,
                    &text_size);

            set_text_panel(
                tools,
                copy_script_string(title, title_size),
                copy_script_string(text, text_size));
        }

        wz::script::clear_pending_text_panels(host);
    }

    void render_tool_registry(const ToolRegistry& tools)
    {
        for (const ToolTextPanel& panel : tools.text_panels)
        {
            std::printf(
                "[retained tool panel] %s: %s\n",
                panel.title.c_str(),
                panel.text.c_str());
        }
    }
}

int main()
{
    if (!wz::script::init_v8_platform())
    {
        std::printf("failed to initialize V8 platform\n");
        return 1;
    }

    wz::script::ScriptHost* scripts =
        wz::script::create_v8_script_host();

    ToolRegistry tools;

    if (scripts == nullptr)
    {
        std::printf("failed to create script host\n");
        wz::script::shutdown_v8_platform();
        return 1;
    }

    if (!wz::script::initialize(scripts))
    {
        std::printf("failed to initialize script host\n");
        wz::script::destroy_v8_script_host(scripts);
        wz::script::shutdown_v8_platform();
        return 1;
    }

    // Pretend this is engine startup.
    if (!run_checked(
        scripts,
        "startup.js",
        "wz.log('script system online');"
        "wz.tool.textPanel('Startup', 'Script system online');"
        "'startup complete';"))
    {
        wz::script::destroy_v8_script_host(scripts);
        return 1;
    }

    drain_script_logs(scripts);
    apply_pending_text_panels(scripts, tools);
    render_tool_registry(tools);

    // Pretend these are frames.
    for (int frame = 0; frame < 3; ++frame)
    {
        char source[256] = {};
        std::snprintf(
            source,
            sizeof(source),
            "wz.log('frame %d from js');"
            "wz.tool.textPanel('Frame', 'Current frame: %d');"
            "%d;",
            frame,
            frame,
            frame);

        if (!run_checked(scripts, "frame.js", source))
        {
            wz::script::destroy_v8_script_host(scripts);
            return 1;
        }

        drain_script_logs(scripts);
        apply_pending_text_panels(scripts, tools);
        render_tool_registry(tools);
    }

    // Pretend this is engine shutdown.
    if (!run_checked(
        scripts,
        "shudown.js",
        "wz.log('script system shutdown');"
        "wz.tool.textPanel('Status', 'Shutdown complete');"
        "'done';"))
    {
        wz::script::destroy_v8_script_host(scripts);
        return 1;
    }

    drain_script_logs(scripts);
    apply_pending_text_panels(scripts, tools);
    render_tool_registry(tools);

    wz::script::destroy_v8_script_host(scripts);
    wz::script::shutdown_v8_platform();

    return 0;
}