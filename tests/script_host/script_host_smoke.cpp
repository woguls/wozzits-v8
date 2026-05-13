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
        wz::script::clear_logs(host);

        const wz::script::RunSourceResult result =
            wz::script::run_source(
                host,
                "log.js",
                "wz.log('hello from js'); 'logged';");

        if (!result.ok)
        {
            std::printf("error: %s\n", result.error ? result.error : "<null>");
            wz::script::destroy_v8_script_host(host);
            return 1;
        }

        assert(result.value != nullptr);
        assert(std::strcmp(result.value, "logged") == 0);

        assert(wz::script::log_count(host) == 1);

        std::size_t message_size = 0;
        const char* message =
            wz::script::log_message(host, 0, &message_size);

        assert(message != nullptr);
        assert(std::strcmp(message, "hello from js") == 0);

        std::printf(
            "Captured log: %s len=%zu\n",
            message,
            message_size);
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