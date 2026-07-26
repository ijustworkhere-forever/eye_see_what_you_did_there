#include "ExpressionPose.h"

namespace eyesee {

ExpressionEyelidTarget expressionEyelidTarget(Expression expression) {
    switch (expression) {
        case Expression::Neutral: return ExpressionEyelidTarget{0.85f, 0.85f};
        case Expression::Happy: return ExpressionEyelidTarget{0.5f, 0.7f};
        case Expression::Curious: return ExpressionEyelidTarget{1.0f, 0.9f};
        case Expression::Sleepy: return ExpressionEyelidTarget{0.3f, 0.5f};
        case Expression::Angry: return ExpressionEyelidTarget{0.6f, 0.9f};
        case Expression::Surprised: return ExpressionEyelidTarget{1.0f, 1.0f};
    }
    return ExpressionEyelidTarget{0.85f, 0.85f};
}

}  // namespace eyesee
