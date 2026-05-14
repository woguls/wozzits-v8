#include <wozzits/script/script_host.h>

#include <cassert>
#include <cstdio>
#include <cstring>

static wz::script::ScriptHost* g_host = nullptr;

static void test_stats_panel_basic()
{
    wz::script::clear_pending_stats_panels(g_host);

    const wz::script::RunSourceResult result =
        wz::script::run_source(
            g_host,
            "stats.js",
            "wz.tool.statsPanel('Frame Stats', ["
            "  ['Frame', '42'],"
            "  ['Delta ms', '16.7']"
            "]); 'ok';");

    assert(result.ok);
    assert(std::strcmp(result.value, "ok") == 0);

    assert(wz::script::pending_stats_panel_count(g_host) == 1);

    std::size_t title_size = 0;
    const char* title =
        wz::script::pending_stats_panel_title(g_host, 0, &title_size);

    assert(title != nullptr);
    assert(std::strcmp(title, "Frame Stats") == 0);
    assert(title_size == std::strlen("Frame Stats"));

    assert(wz::script::pending_stats_panel_row_count(g_host, 0) == 2);

    std::size_t label_size = 0;
    const char* label =
        wz::script::pending_stats_panel_row_label(g_host, 0, 0, &label_size);
    assert(label != nullptr);
    assert(std::strcmp(label, "Frame") == 0);

    std::size_t value_size = 0;
    const char* value =
        wz::script::pending_stats_panel_row_value(g_host, 0, 0, &value_size);
    assert(value != nullptr);
    assert(std::strcmp(value, "42") == 0);

    const char* label2 =
        wz::script::pending_stats_panel_row_label(g_host, 0, 1, nullptr);
    assert(label2 != nullptr);
    assert(std::strcmp(label2, "Delta ms") == 0);

    const char* value2 =
        wz::script::pending_stats_panel_row_value(g_host, 0, 1, nullptr);
    assert(value2 != nullptr);
    assert(std::strcmp(value2, "16.7") == 0);

    std::printf("test_stats_panel_basic: ok\n");
}

static void test_stats_panel_multiple()
{
    wz::script::clear_pending_stats_panels(g_host);

    wz::script::run_source(g_host, "s1.js",
        "wz.tool.statsPanel('A', [['x', '1']]); 'ok';");
    wz::script::run_source(g_host, "s2.js",
        "wz.tool.statsPanel('B', [['y', '2'], ['z', '3']]); 'ok';");

    assert(wz::script::pending_stats_panel_count(g_host) == 2);
    assert(wz::script::pending_stats_panel_row_count(g_host, 0) == 1);
    assert(wz::script::pending_stats_panel_row_count(g_host, 1) == 2);

    std::printf("test_stats_panel_multiple: ok\n");
}

static void test_stats_panel_clear()
{
    wz::script::run_source(g_host, "sc.js",
        "wz.tool.statsPanel('X', []); 'ok';");

    assert(wz::script::pending_stats_panel_count(g_host) > 0);

    wz::script::clear_pending_stats_panels(g_host);

    assert(wz::script::pending_stats_panel_count(g_host) == 0);

    std::printf("test_stats_panel_clear: ok\n");
}

static void test_stats_panel_out_of_bounds()
{
    wz::script::clear_pending_stats_panels(g_host);

    assert(wz::script::pending_stats_panel_title(g_host, 0, nullptr) == nullptr);
    assert(wz::script::pending_stats_panel_row_count(g_host, 0) == 0);
    assert(wz::script::pending_stats_panel_row_label(g_host, 0, 0, nullptr) == nullptr);
    assert(wz::script::pending_stats_panel_row_value(g_host, 0, 0, nullptr) == nullptr);

    std::printf("test_stats_panel_out_of_bounds: ok\n");
}

int main()
{
    assert(wz::script::init_v8_platform());

    g_host = wz::script::create_v8_script_host();
    assert(g_host != nullptr);
    assert(wz::script::initialize(g_host));

    test_stats_panel_basic();
    test_stats_panel_multiple();
    test_stats_panel_clear();
    test_stats_panel_out_of_bounds();

    wz::script::destroy_v8_script_host(g_host);
    wz::script::shutdown_v8_platform();

    return 0;
}
