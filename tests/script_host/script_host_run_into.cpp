#include <wozzits/script/script_host.h>

#include <cassert>
#include <cstdio>
#include <cstring>

static wz::script::ScriptHost* g_host = nullptr;

static void test_value_copy()
{
    char buf[64] = {};
    wz::script::RunSourceBuffers out;
    out.value = buf;
    out.value_capacity = sizeof(buf);

    const bool ok = wz::script::run_source_into(
        g_host, "val.js", "'hello wozzits';", &out);

    assert(ok);
    assert(out.value_size == std::strlen("hello wozzits"));
    assert(std::strcmp(out.value, "hello wozzits") == 0);
    assert(!out.value_truncated);

    std::printf("test_value_copy: ok\n");
}

static void test_error_copy()
{
    char buf[64] = {};
    wz::script::RunSourceBuffers out;
    out.error = buf;
    out.error_capacity = sizeof(buf);

    const bool ok = wz::script::run_source_into(
        g_host, "err.js", "throw new Error('boom');", &out);

    assert(!ok);
    assert(out.error_size > 0);
    assert(std::strstr(out.error, "boom") != nullptr);
    assert(!out.error_truncated);

    std::printf("test_error_copy: ok\n");
}

static void test_value_truncation()
{
    char buf[4] = {};  // capacity 4 → at most 3 chars + null
    wz::script::RunSourceBuffers out;
    out.value = buf;
    out.value_capacity = sizeof(buf);

    const bool ok = wz::script::run_source_into(
        g_host, "trunc.js", "'hello';", &out);

    assert(ok);
    assert(out.value_truncated);
    assert(out.value_size == 3);
    assert(std::strncmp(out.value, "hel", 3) == 0);
    assert(out.value[3] == '\0');

    std::printf("test_value_truncation: ok\n");
}

static void test_null_out_returns_false()
{
    const bool ok = wz::script::run_source_into(
        g_host, "null.js", "1;", nullptr);

    assert(!ok);

    std::printf("test_null_out_returns_false: ok\n");
}

static void test_null_buffer_sets_truncated()
{
    wz::script::RunSourceBuffers out;
    // value buffer left null/zero — data available but nowhere to write

    const bool ok = wz::script::run_source_into(
        g_host, "nobuf.js", "'data';", &out);

    assert(ok);
    assert(out.value_size == 0);
    assert(out.value_truncated);

    std::printf("test_null_buffer_sets_truncated: ok\n");
}

int main()
{
    assert(wz::script::init_v8_platform());

    g_host = wz::script::create_v8_script_host();
    assert(g_host != nullptr);
    assert(wz::script::initialize(g_host));

    test_value_copy();
    test_error_copy();
    test_value_truncation();
    test_null_out_returns_false();
    test_null_buffer_sets_truncated();

    wz::script::destroy_v8_script_host(g_host);
    wz::script::shutdown_v8_platform();

    return 0;
}
