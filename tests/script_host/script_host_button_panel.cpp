#include <wozzits/script/script_host.h>

#include <cassert>
#include <cstdio>
#include <cstring>

static wz::script::ScriptHost* g_host = nullptr;

static void test_button_panel_basic()
{
    wz::script::clear_pending_button_panels(g_host);

    const wz::script::RunSourceResult result =
        wz::script::run_source(
            g_host,
            "buttons.js",
            "wz.tool.buttonPanel('Asset Tools', ["
            "  ['Reload assets', 'assets.reload_changed'],"
            "  ['Clear logs',    'logs.clear']"
            "]); 'ok';");

    assert(result.ok);
    assert(std::strcmp(result.value, "ok") == 0);

    assert(wz::script::pending_button_panel_count(g_host) == 1);

    std::size_t title_size = 0;
    const char* title =
        wz::script::pending_button_panel_title(g_host, 0, &title_size);

    assert(title != nullptr);
    assert(std::strcmp(title, "Asset Tools") == 0);
    assert(title_size == std::strlen("Asset Tools"));

    assert(wz::script::pending_button_panel_button_count(g_host, 0) == 2);

    std::size_t label_size = 0;
    const char* label =
        wz::script::pending_button_panel_button_label(g_host, 0, 0, &label_size);
    assert(label != nullptr);
    assert(std::strcmp(label, "Reload assets") == 0);

    std::size_t action_size = 0;
    const char* action =
        wz::script::pending_button_panel_button_action(g_host, 0, 0, &action_size);
    assert(action != nullptr);
    assert(std::strcmp(action, "assets.reload_changed") == 0);

    const char* label2 =
        wz::script::pending_button_panel_button_label(g_host, 0, 1, nullptr);
    assert(label2 != nullptr);
    assert(std::strcmp(label2, "Clear logs") == 0);

    const char* action2 =
        wz::script::pending_button_panel_button_action(g_host, 0, 1, nullptr);
    assert(action2 != nullptr);
    assert(std::strcmp(action2, "logs.clear") == 0);

    std::printf("test_button_panel_basic: ok\n");
}

static void test_button_panel_multiple()
{
    wz::script::clear_pending_button_panels(g_host);

    wz::script::run_source(g_host, "b1.js",
        "wz.tool.buttonPanel('Tools A', [['Go', 'a.go']]); 'ok';");
    wz::script::run_source(g_host, "b2.js",
        "wz.tool.buttonPanel('Tools B', [['X', 'b.x'], ['Y', 'b.y']]); 'ok';");

    assert(wz::script::pending_button_panel_count(g_host) == 2);
    assert(wz::script::pending_button_panel_button_count(g_host, 0) == 1);
    assert(wz::script::pending_button_panel_button_count(g_host, 1) == 2);

    std::printf("test_button_panel_multiple: ok\n");
}

static void test_button_panel_clear()
{
    wz::script::run_source(g_host, "bc.js",
        "wz.tool.buttonPanel('X', []); 'ok';");

    assert(wz::script::pending_button_panel_count(g_host) > 0);

    wz::script::clear_pending_button_panels(g_host);

    assert(wz::script::pending_button_panel_count(g_host) == 0);

    std::printf("test_button_panel_clear: ok\n");
}

static void test_button_panel_out_of_bounds()
{
    wz::script::clear_pending_button_panels(g_host);

    assert(wz::script::pending_button_panel_title(g_host, 0, nullptr) == nullptr);
    assert(wz::script::pending_button_panel_button_count(g_host, 0) == 0);
    assert(wz::script::pending_button_panel_button_label(g_host, 0, 0, nullptr) == nullptr);
    assert(wz::script::pending_button_panel_button_action(g_host, 0, 0, nullptr) == nullptr);

    std::printf("test_button_panel_out_of_bounds: ok\n");
}

int main()
{
    assert(wz::script::init_v8_platform());

    g_host = wz::script::create_v8_script_host();
    assert(g_host != nullptr);
    assert(wz::script::initialize(g_host));

    test_button_panel_basic();
    test_button_panel_multiple();
    test_button_panel_clear();
    test_button_panel_out_of_bounds();

    wz::script::destroy_v8_script_host(g_host);
    wz::script::shutdown_v8_platform();

    return 0;
}
