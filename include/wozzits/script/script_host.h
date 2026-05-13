#pragma once
// include/wozzits/script/script_host.h


#include <cstddef>

namespace wz::script
{
    struct ScriptHost;

    struct RunSourceResult
    {
        // RunSourceResult.value/error remain valid until the next run_source() call or host destruction.
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
}