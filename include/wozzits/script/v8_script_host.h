// include/wozzits/script/v8_script_host.h
#pragma once

#include <memory>

#include <wozzits/script/script_host.h>

namespace wz::script
{
    std::unique_ptr<ScriptHost> create_v8_script_host();
}