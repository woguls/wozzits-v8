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