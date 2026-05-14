#pragma once

#include <cstddef>

namespace wz::script
{
    struct ScriptHost;

    struct RunSourceResult
    {
        bool ok = false;

        // Points to ScriptHost-owned storage.
        // Valid until the next run_source(), clear/shutdown, or destroy_v8_script_host().
        const char* value = nullptr;
        std::size_t value_size = 0;

        // Points to ScriptHost-owned storage, unless the host pointer passed to
        // run_source() was null. Valid until the next run_source(), clear/shutdown,
        // or destroy_v8_script_host().
        const char* error = nullptr;
        std::size_t error_size = 0;
    };

    bool init_v8_platform();
    void shutdown_v8_platform();

    ScriptHost* create_v8_script_host();
    void destroy_v8_script_host(ScriptHost* host);

    bool initialize(ScriptHost* host);
    void shutdown(ScriptHost* host);

    RunSourceResult run_source(
        ScriptHost* host,
        const char* name,
        const char* source);

    struct RunSourceBuffers
    {
        char* value = nullptr;
        std::size_t value_capacity = 0;
        std::size_t value_size = 0;

        char* error = nullptr;
        std::size_t error_capacity = 0;
        std::size_t error_size = 0;

        bool value_truncated = false;
        bool error_truncated = false;
    };

    // Runs source and writes the result into caller-owned buffers.
    // Returns true on success (value written), false on error (error written).
    // Returns false immediately if out is null.
    // If a buffer pointer is null or its capacity is zero the corresponding
    // _size is set to 0 and _truncated is set to true when data was available.
    bool run_source_into(
        ScriptHost* host,
        const char* name,
        const char* source,
        RunSourceBuffers* out);

    void clear_logs(ScriptHost* host);

    std::size_t log_count(const ScriptHost* host);

    // Returns a pointer to ScriptHost-owned log storage.
    // Valid until clear_logs(), shutdown(), destroy_v8_script_host(),
    // or until future API changes introduce log compaction/removal.
    const char* log_message(
        const ScriptHost* host,
        std::size_t index,
        std::size_t* out_size);

    void clear_pending_text_panels(ScriptHost* host);

    std::size_t pending_text_panel_count(const ScriptHost* host);

    // Returns a pointer to ScriptHost-owned text panel title storage.
    // Valid until clear_pending_text_panels(), shutdown(), destroy_v8_script_host(),
    // or until future API changes introduce text panel compaction/removal.
    const char* pending_text_panel_title(
        const ScriptHost* host,
        std::size_t index,
        std::size_t* out_size);

    // Returns a pointer to ScriptHost-owned text panel body storage.
    // Valid until clear_pending_text_panels(), shutdown(), destroy_v8_script_host(),
    // or until future API changes introduce text panel compaction/removal.
    const char* pending_text_panel_text(
        const ScriptHost* host,
        std::size_t index,
        std::size_t* out_size);

    // Stats panels — wz.tool.statsPanel(title, [[label, value], ...])

    void clear_pending_stats_panels(ScriptHost* host);

    std::size_t pending_stats_panel_count(const ScriptHost* host);

    const char* pending_stats_panel_title(
        const ScriptHost* host,
        std::size_t index,
        std::size_t* out_size);

    std::size_t pending_stats_panel_row_count(
        const ScriptHost* host,
        std::size_t panel_index);

    const char* pending_stats_panel_row_label(
        const ScriptHost* host,
        std::size_t panel_index,
        std::size_t row_index,
        std::size_t* out_size);

    const char* pending_stats_panel_row_value(
        const ScriptHost* host,
        std::size_t panel_index,
        std::size_t row_index,
        std::size_t* out_size);

    // Button panels — wz.tool.buttonPanel(title, [[label, action], ...])
    // Clicking a button should cause the engine to handle the action string.

    void clear_pending_button_panels(ScriptHost* host);

    std::size_t pending_button_panel_count(const ScriptHost* host);

    const char* pending_button_panel_title(
        const ScriptHost* host,
        std::size_t index,
        std::size_t* out_size);

    std::size_t pending_button_panel_button_count(
        const ScriptHost* host,
        std::size_t panel_index);

    const char* pending_button_panel_button_label(
        const ScriptHost* host,
        std::size_t panel_index,
        std::size_t button_index,
        std::size_t* out_size);

    const char* pending_button_panel_button_action(
        const ScriptHost* host,
        std::size_t panel_index,
        std::size_t button_index,
        std::size_t* out_size);
}