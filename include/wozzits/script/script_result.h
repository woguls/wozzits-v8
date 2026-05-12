// include/wozzits/script/script_result.h
#pragma once

#include <string>

namespace wz::script
{
    struct ScriptResult
    {
        bool ok = false;
        std::string value;
        std::string error;
    };
}