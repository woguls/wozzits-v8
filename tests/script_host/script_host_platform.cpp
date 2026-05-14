#include <wozzits/script/script_host.h>

#include <cassert>
#include <cstdio>
#include <cstring>

static void test_initialize_without_platform()
{
    wz::script::ScriptHost* host = wz::script::create_v8_script_host();
    assert(host != nullptr);

    const bool result = wz::script::initialize(host);
    assert(!result);

    wz::script::destroy_v8_script_host(host);

    std::printf("test_initialize_without_platform: ok\n");
}

static void test_sequential_hosts()
{
    {
        wz::script::ScriptHost* host = wz::script::create_v8_script_host();
        assert(host != nullptr);
        assert(wz::script::initialize(host));

        const wz::script::RunSourceResult result =
            wz::script::run_source(host, "seq1.js", "'host one';");

        assert(result.ok);
        assert(result.value != nullptr);
        assert(std::strcmp(result.value, "host one") == 0);

        wz::script::destroy_v8_script_host(host);
    }

    {
        wz::script::ScriptHost* host = wz::script::create_v8_script_host();
        assert(host != nullptr);
        assert(wz::script::initialize(host));

        const wz::script::RunSourceResult result =
            wz::script::run_source(host, "seq2.js", "'host two';");

        assert(result.ok);
        assert(result.value != nullptr);
        assert(std::strcmp(result.value, "host two") == 0);

        wz::script::destroy_v8_script_host(host);
    }

    std::printf("test_sequential_hosts: ok\n");
}

int main()
{
    test_initialize_without_platform();

    assert(wz::script::init_v8_platform());

    test_sequential_hosts();

    wz::script::shutdown_v8_platform();

    return 0;
}
