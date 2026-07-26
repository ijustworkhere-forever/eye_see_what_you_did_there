#pragma once

namespace eyesee {

/** Single source of truth for the firmware version, surfaced in
 * buildStatusJson()'s "firmwareVersion" field (see lib/Protocol/EyeStateJson.cpp).
 * Bump this on every release; see docs/api-reference.md's "Versioning &
 * stability" section for what counts as a breaking vs. additive change
 * under /api/v1/. */
constexpr const char* kFirmwareVersion = "1.0.0";

}  // namespace eyesee
