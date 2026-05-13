#include <wozzits/script/v8_script_host.h>

#include <cassert>
#include <cstdio>
#include <cstring>

int main()
{
    auto host = wz::script::create_v8_script_host();

    assert(host);
    assert(host->initialize());

    {
        const wz::script::ScriptResult result =
            host->run_source("smoke.js", "'hello ' + 'wozzits';");

        if (!result.ok)
        {
            std::puts(result.error.c_str());
            return 1;
        }

        std::printf("value='%s' len=%zu\n", result.value.c_str(), result.value.size());
        std::puts(result.value.c_str());
        assert(std::strcmp(result.value.c_str(), "hello wozzits") == 0);
    }

    {
        const wz::script::ScriptResult result =
            host->run_source("math.js", "21 * 2;");

        if (!result.ok)
        {
            std::puts(result.error.c_str());
            return 1;
        }

        std::puts(result.value.c_str());
        assert(std::strcmp(result.value.c_str(), "42") == 0);
    }

    {
        const wz::script::ScriptResult result =
            host->run_source("error.js", "throw new Error('intentional failure');");

        assert(!result.ok);
        assert(result.error.c_str() != nullptr);
        assert(std::strlen(result.error.c_str()) > 0);
        std::printf("Captured expected error: %s\n", result.error.c_str());
    }

    host->shutdown();

    return 0;
}