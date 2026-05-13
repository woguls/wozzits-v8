#include <wozzits/script/script_host.h>

#include <cassert>
#include <cstdio>
#include <cstring>

namespace
{
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

    void drain_pending_text_panels(wz::script::ScriptHost* host)
    {
        const std::size_t count =
            wz::script::pending_text_panel_count(host);

        for (std::size_t i = 0; i < count; ++i)
        {
            std::size_t title_size = 0;
            const char* title =
                wz::script::pending_text_panel_title(host, i, &title_size);

            std::size_t text_size = 0;
            const char* text =
                wz::script::pending_text_panel_text(host, i, &text_size);

            std::printf(
                "[tool panel] %.*s: %.*s\n",
                static_cast<int>(title_size),
                title ? title : "",
                static_cast<int>(text_size),
                text ? text : "");
        }

        wz::script::clear_pending_text_panels(host);
    }
}

int main()
{
    wz::script::ScriptHost* scripts =
        wz::script::create_v8_script_host();

    if (scripts == nullptr)
    {
        std::printf("failed to create script host\n");
        return 1;
    }

    if (!wz::script::initialize(scripts))
    {
        std::printf("failed to initialize script host\n");
        wz::script::destroy_v8_script_host(scripts);
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

    // Pretend these are frames.
    for (int frame = 0; frame < 3; ++frame)
    {
        char source[256] = {};
        std::snprintf(
            source,
            sizeof(source),
            "wz.log('frame %d from js'); %d;",
            frame,
            frame);

        if (!run_checked(scripts, "frame.js", source))
        {
            wz::script::destroy_v8_script_host(scripts);
            return 1;
        }

        drain_script_logs(scripts);
        drain_pending_text_panels(scripts);
    }

    // Pretend this is engine shutdown.
    if (!run_checked(
        scripts,
        "shutdown.js",
        "wz.log('script system shutdown');"
        "'done';"))
    {
        wz::script::destroy_v8_script_host(scripts);
        return 1;
    }

    drain_script_logs(scripts);

    wz::script::destroy_v8_script_host(scripts);

    return 0;
}