# wozzits-v8

`wozzits-v8` is the V8 embedding layer for Wozzits development tooling.

It provides a small Wozzits-facing script host API and hides V8-specific types
from higher-level engine and devtool modules.

Initial goals:

- initialize/shutdown V8
- run JavaScript source
- report compile/runtime errors cleanly
- expose a small native `wz` object for dev commands

Non-goals:

- Node.js compatibility
- game scripting
- untrusted mod sandboxing
- direct engine object ownership from JavaScript

## Current supported use

`wozzits-v8` currently supports a minimal embedded scripting host:

- create/destroy a `ScriptHost`
- initialize/shutdown V8
- run JavaScript source strings
- read the result as a host-owned string pointer
- capture thrown JavaScript errors
- expose `wz.log(message)` to JavaScript
- inspect and clear host-side script logs

The public API intentionally avoids V8 types and C++ standard-library ownership types. Consumers should include only:

```cpp
#include <wozzits/script/script_host.h>