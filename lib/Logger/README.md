# Logger

**Purpose:** Cross-cutting logging abstraction backed by `Serial`.

**Responsibilities:** Provide `debug`/`info`/`warn`/`error` static calls with
a consistent `[LEVEL] tag: message` format.

**Planned features:** None beyond level filtering (e.g. compile-time or
runtime minimum level).

**Future work:** Optional log sinks other than Serial (e.g. a ring buffer
exposed over `/api/v1/status` for remote diagnostics).
