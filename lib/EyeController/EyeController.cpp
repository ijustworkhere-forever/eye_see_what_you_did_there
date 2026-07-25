#include "EyeController.h"

namespace eyesee {

EyeController::EyeController(IServoOutput& output, CalibrationManager& calibration)
    : output_(output), calibration_(calibration), currentPose_() {
}

void EyeController::applyPose(const EyePose& pose) {
    currentPose_ = pose;
    output_.write(toServoOutput(pose));
}

EyePose EyeController::currentPose() const {
    return currentPose_;
}

void EyeController::look(float x, float y) {
    EyePose pose = currentPose_;
    pose.lookX = x;
    pose.lookY = y;
    applyPose(pose);
}

void EyeController::closeAllLids() {
    EyePose pose = currentPose_;
    pose.upperLeftLid = 0.0f;
    pose.lowerLeftLid = 0.0f;
    pose.upperRightLid = 0.0f;
    pose.lowerRightLid = 0.0f;
    applyPose(pose);
}

void EyeController::blink() {
    closeAllLids();
    // TODO: reopening after a duration is IAnimationEngine's job (docs/ROADMAP.md v0.3) —
    // EyeController never owns time, so this call only closes the eyelids.
}

void EyeController::winkLeft() {
    EyePose pose = currentPose_;
    pose.upperLeftLid = 0.0f;
    pose.lowerLeftLid = 0.0f;
    applyPose(pose);
}

void EyeController::winkRight() {
    EyePose pose = currentPose_;
    pose.upperRightLid = 0.0f;
    pose.lowerRightLid = 0.0f;
    applyPose(pose);
}

void EyeController::sleep() {
    closeAllLids();
}

void EyeController::wake() {
    EyePose pose = currentPose_;
    pose.upperLeftLid = 1.0f;
    pose.lowerLeftLid = 1.0f;
    pose.upperRightLid = 1.0f;
    pose.lowerRightLid = 1.0f;
    applyPose(pose);
}

void EyeController::setExpression(Expression expression) {
    (void)expression;
    // TODO: per-expression pose blending (docs/ROADMAP.md v0.3). No pose
    // change this pass — a stub must never silently mutate state it wasn't
    // asked to touch (see docs/architecture.md).
}

void EyeController::setIdle() {
    applyPose(EyePose{});
}

void EyeController::update(uint32_t deltaMs) {
    (void)deltaMs;
    // TODO: reserved. EyeController owns no timed state (docs/architecture.md invariant 1).
}

ServoOutput EyeController::toServoOutput(const EyePose& pose) const {
    (void)pose;
    // TODO: real pose -> pulse conversion (pulse scaling, invert, mirror,
    // mechanical offset) is out of scope this pass (docs/architecture.md).
    // Every channel reports its calibrated neutral pulse for now.
    const EyeConfig& config = calibration_.eyeConfig();
    ServoOutput out;
    out.lr = config.lr.neutralPulseUs;
    out.ud = config.ud.neutralPulseUs;
    out.tl = config.tl.neutralPulseUs;
    out.bl = config.bl.neutralPulseUs;
    out.tr = config.tr.neutralPulseUs;
    out.br = config.br.neutralPulseUs;
    return out;
}

}  // namespace eyesee
