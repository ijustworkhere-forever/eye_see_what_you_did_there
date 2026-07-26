#include "EyeController.h"

#include <cmath>

#include "ExpressionPose.h"

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
    const ExpressionEyelidTarget target = expressionEyelidTarget(expression);
    EyePose pose = currentPose_;
    pose.upperLeftLid = target.upperLid;
    pose.lowerLeftLid = target.lowerLid;
    pose.upperRightLid = target.upperLid;
    pose.lowerRightLid = target.lowerLid;
    applyPose(pose);
}

void EyeController::setIdle() {
    applyPose(EyePose{});
}

void EyeController::update(uint32_t deltaMs) {
    (void)deltaMs;
    // TODO: reserved. EyeController owns no timed state (docs/architecture.md invariant 1).
}

namespace {
float clampf(float value, float lo, float hi) {
    if (value < lo) return lo;
    if (value > hi) return hi;
    return value;
}
}  // namespace

uint16_t EyeController::gazeChannelPulse(float input, const ServoConfig& config) {
    float effective = input * (config.inverted ? -1.0f : 1.0f) * (config.mirrored ? -1.0f : 1.0f);
    float pulse =
        effective >= 0.0f
            ? static_cast<float>(config.neutralPulseUs) +
                  effective * static_cast<float>(config.maxPulseUs - config.neutralPulseUs)
            : static_cast<float>(config.neutralPulseUs) +
                  effective * static_cast<float>(config.neutralPulseUs - config.minPulseUs);
    pulse = clampf(pulse + static_cast<float>(config.mechanicalOffset), config.minPulseUs,
                   config.maxPulseUs);
    return static_cast<uint16_t>(std::lround(pulse));
}

uint16_t EyeController::eyelidChannelPulse(float input, const ServoConfig& config) {
    float effective = input;
    if (config.inverted) effective = 1.0f - effective;
    if (config.mirrored) effective = 1.0f - effective;
    float pulse = clampf(static_cast<float>(config.minPulseUs) +
                             effective * static_cast<float>(config.maxPulseUs - config.minPulseUs) +
                             static_cast<float>(config.mechanicalOffset),
                         config.minPulseUs, config.maxPulseUs);
    return static_cast<uint16_t>(std::lround(pulse));
}

ServoOutput EyeController::toServoOutput(const EyePose& pose) const {
    const EyeConfig& config = calibration_.eyeConfig();
    ServoOutput out;
    out.lr = gazeChannelPulse(pose.lookX, config.lr);
    out.ud = gazeChannelPulse(pose.lookY, config.ud);
    out.tl = eyelidChannelPulse(pose.upperLeftLid, config.tl);
    out.bl = eyelidChannelPulse(pose.lowerLeftLid, config.bl);
    out.tr = eyelidChannelPulse(pose.upperRightLid, config.tr);
    out.br = eyelidChannelPulse(pose.lowerRightLid, config.br);
    return out;
}

}  // namespace eyesee
