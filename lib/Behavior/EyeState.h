#pragma once

namespace eyesee {

/** System-level behavior state, owned by BehaviorEngine. */
enum class EyeState {
    Startup,
    Calibration,
    Manual,
    Idle,
    Tracking,
    Sleeping,
    Disabled,
    Error
};

}  // namespace eyesee
