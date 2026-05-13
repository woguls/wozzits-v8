#include <wozzits/script/script_host.h>

#include <cassert>
#include <cstdio>
#include <cstring>

int main()
{
    wz::script::ScriptHost* host =
        wz::script::create_v8_script_host();

    assert(host != nullptr);
    assert(wz::script::initialize(host));

    {
        const wz::script::RunSourceResult result =
            wz::script::run_source(
                host,
                "smoke.js",
                "'hello ' + 'wozzits';");

        if (!result.ok)
        {
            std::printf("error: %s\n", result.error ? result.error : "<null>");
            wz::script::destroy_v8_script_host(host);
            return 1;
        }

        std::printf(
            "value='%s' len=%zu\n",
            result.value ? result.value : "<null>",
            result.value_size);

        assert(result.value != nullptr);
        assert(std::strcmp(result.value, "hello wozzits") == 0);
    }

    {
        const wz::script::RunSourceResult result =
            wz::script::run_source(
                host,
                "math.js",
                "21 * 2;");

        if (!result.ok)
        {
            std::printf("error: %s\n", result.error ? result.error : "<null>");
            wz::script::destroy_v8_script_host(host);
            return 1;
        }

        std::printf("%s\n", result.value ? result.value : "<null>");

        assert(result.value != nullptr);
        assert(std::strcmp(result.value, "42") == 0);
    }

    {
        const wz::script::RunSourceResult result =
            wz::script::run_source(
                host,
                "error.js",
                "throw new Error('intentional failure');");

        assert(!result.ok);
        assert(result.error != nullptr);
        assert(std::strlen(result.error) > 0);

        std::printf("Captured expected error: %s\n", result.error);
    }

    wz::script::destroy_v8_script_host(host);

    return 0;
}