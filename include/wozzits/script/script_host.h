#pragma once

#include <cstddef>

namespace wz::script
{
    struct ScriptHost;

    struct RunSourceResult
    {
        bool ok = false;

        const char* value = nullptr;
        std::size_t value_size = 0;

        const char* error = nullptr;
        std::size_t error_size = 0;
    };

    ScriptHost* create_v8_script_host();
    void destroy_v8_script_host(ScriptHost* host);

    bool initialize(ScriptHost* host);
    void shutdown(ScriptHost* host);

    RunSourceResult run_source(
        ScriptHost* host,
        const char* name,
        const char* source);

    void clear_logs(ScriptHost* host);
    std::size_t log_count(const ScriptHost* host);

    const char* log_message(
        const ScriptHost* host,
        std::size_t index,
        std::size_t* out_size);
}