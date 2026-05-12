// include/wozzits/script/script_host.h
#pragma once

#include <string_view>

#include <wozzits/script/script_result.h>

namespace wz::script
{
    class ScriptHost
    {
    public:
        virtual ~ScriptHost() = default;

        virtual bool initialize() = 0;
        virtual void shutdown() = 0;

        virtual ScriptResult run_source(
            std::string_view name,
            std::string_view source) = 0;
    };
}