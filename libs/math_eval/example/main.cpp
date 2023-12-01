#include <wolv/math_eval/math_evaluator.hpp>

int main() {
    wolv::math_eval::MathEvaluator<long double> evaluator;

    const auto result = evaluator.evaluate("5 * 9 + 3 * (2 + 4)");

    if (result.has_value())
        printf("Result: %Lf\n", result.value());
    else
        printf("Invalid expression\n");
}