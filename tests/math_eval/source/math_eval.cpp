#include <wolv/test/tests.hpp>
#include <wolv/math_eval/math_evaluator.hpp>
#include <wolv/utils/core.hpp>

#include <cmath>
#include <limits>
#include <optional>
#include <numbers>
#include <clocale>

using wolv::u64, wolv::i64, wolv::u128, wolv::i128;

namespace {
    consteval i128 operator""_i128(const char* str) {
        i128 result = 0;
        for (; *str != '\0'; ++str) {
            result = result * 10 + (*str - '0');
        }
        return result;
    }
    consteval u128 operator""_u128(const char* str) {
        u128 result = 0;
        for (; *str != '\0'; ++str) {
            result = result * 10 + (*str - '0');
        }
        return result;
    }

    [[nodiscard]] bool expect_negative_zero(std::optional<long double> res) {
        return res == 0.0 && std::signbit(*res);
    }
    [[nodiscard]] bool expect_positive_zero(std::optional<long double> res) {
        return res == 0.0 && !std::signbit(*res);
    }
    [[nodiscard]] bool expect_nan(std::optional<long double> res) {
        return res.has_value() && std::isnan(*res);
    }
    [[nodiscard]] bool expect_approx_eq(std::optional<long double> res, long double expected) {
        return res.has_value() && std::isfinite(*res) && std::abs(*res - expected) <= 1e-11L;
    }
}

TEST_SEQUENCE("FloatParsing") {
    wolv::math_eval::MathEvaluator<long double> eval;

    TEST_ASSERT(eval.evaluate("") == std::nullopt);
    TEST_ASSERT(eval.evaluate(" ") == std::nullopt);
    TEST_ASSERT(eval.evaluate("  \v \n \t   ") == std::nullopt);

    TEST_ASSERT(eval.evaluate("1") == 1.L);
    TEST_ASSERT(eval.evaluate("  3") == 3.L);
    TEST_ASSERT(eval.evaluate(" 6   ") == 6.L);
    TEST_ASSERT(eval.evaluate("-1") == -1.L);
    TEST_ASSERT(eval.evaluate("+1") == 1.L);
    TEST_ASSERT(expect_positive_zero(eval.evaluate("+0")));
    TEST_ASSERT(expect_negative_zero(eval.evaluate("-0")));
    TEST_ASSERT(eval.evaluate("0") == 0.L);
    TEST_ASSERT(eval.evaluate("001") == 1.L);
    TEST_ASSERT(eval.evaluate("00000") == 0.L);

    TEST_ASSERT(eval.evaluate("0.0") == 0.L);
    TEST_ASSERT(eval.evaluate("0.5") == 0.5L);
    TEST_ASSERT(eval.evaluate(".0") == 0.L);
    TEST_ASSERT(eval.evaluate(".25") == 0.25L);
    TEST_ASSERT(eval.evaluate(".375") == 0.375L);
    TEST_ASSERT(eval.evaluate(".000") == 0.L);
    TEST_ASSERT(eval.evaluate("1.2.3") == std::nullopt);
    TEST_ASSERT(eval.evaluate(".2.3") == std::nullopt);
    TEST_ASSERT(eval.evaluate(".") == std::nullopt);

    TEST_ASSERT(eval.evaluate("0x0") == 0.L);
    TEST_ASSERT(eval.evaluate("0x1") == 1.L);
    TEST_ASSERT(eval.evaluate("0xA") == 10.L);
    TEST_ASSERT(eval.evaluate("0xF") == 15.L);
    TEST_ASSERT(eval.evaluate("0xABC") == 2748.L);
    TEST_ASSERT(eval.evaluate("00x100") == 256.L); // bug

    TEST_ASSERT(eval.evaluate("0X0") == 0.L);
    TEST_ASSERT(eval.evaluate("0X1") == 1.L);
    TEST_ASSERT(eval.evaluate("0XF") == 15.L);
    TEST_ASSERT(eval.evaluate("X0") == std::nullopt);
    TEST_ASSERT(eval.evaluate("X") == std::nullopt);

    TEST_ASSERT(eval.evaluate("x") == std::nullopt);
    TEST_ASSERT(eval.evaluate("x0") == std::nullopt);
    TEST_ASSERT(eval.evaluate("1x0") == std::nullopt);

    TEST_ASSERT(eval.evaluate("0e0") == 0.L);
    TEST_ASSERT(eval.evaluate("1e0") == 1.L);
    TEST_ASSERT(eval.evaluate("5e0") == 5.L);
    TEST_ASSERT(eval.evaluate("6e1") == 60.L);
    TEST_ASSERT(eval.evaluate("1e4") == 10000.L);
    TEST_ASSERT(eval.evaluate("1E4") == 10000.L);
    TEST_ASSERT(eval.evaluate("2e+5") == 200000.L);
    TEST_ASSERT(eval.evaluate("6.25e-2") == 0.0625L);
    TEST_ASSERT(eval.evaluate("325E-2") == 3.25L);

    TEST_ASSERT(eval.evaluate("1.5e+308") == 1.5e+308L);
    TEST_ASSERT(eval.evaluate("3e+10000") == std::numeric_limits<long double>::infinity()); // bug?
    TEST_ASSERT(eval.evaluate("-3e+20000") == -std::numeric_limits<long double>::infinity()); // bug?
    TEST_ASSERT(expect_positive_zero(eval.evaluate("3e-20000")));
    TEST_ASSERT(expect_negative_zero(eval.evaluate("-3e-20000")));

    TEST_ASSERT(eval.evaluate("0x0.0") == 0.L);
    TEST_ASSERT(eval.evaluate("0x1.0") == 1.L);
    TEST_ASSERT(eval.evaluate("0xF.0") == 15.L);
    TEST_ASSERT(eval.evaluate("0xF.A") == 15.625L);
    TEST_ASSERT(eval.evaluate("0xF.123A") == 0xF.123Ap0L);

    TEST_ASSERT(eval.evaluate("0x0.0p0") == 0.L);
    TEST_ASSERT(eval.evaluate("0xBp0") == 11.L);
    TEST_ASSERT(eval.evaluate("0xBp1") == 22.L);
    TEST_ASSERT(eval.evaluate("0xBp2") == 44.L);
    TEST_ASSERT(eval.evaluate("0xBp10") == 11264.L);
    TEST_ASSERT(eval.evaluate("0xFp15") == 491520.L);
    TEST_ASSERT(eval.evaluate("0xCp-3") == 1.5L);
    TEST_ASSERT(eval.evaluate("0x123.456p78") == 0x123.456p78L);
    TEST_ASSERT(eval.evaluate("0xp78") == std::nullopt);
    TEST_ASSERT(eval.evaluate("0x0p") == std::nullopt);

    TEST_ASSERT(eval.evaluate("0xFp999999") == std::numeric_limits<long double>::infinity()); // bug?
    TEST_ASSERT(eval.evaluate("-0xABCDEFp999999") == -std::numeric_limits<long double>::infinity()); // bug?
    TEST_ASSERT(expect_positive_zero(eval.evaluate("0xF.ABCp-888888")));
    TEST_ASSERT(expect_negative_zero(eval.evaluate("-0xF.ABCDp-888888")));

    TEST_ASSERT(eval.evaluate("INF") == std::nullopt);
    TEST_ASSERT(eval.evaluate("INFINITY") == std::nullopt);
    TEST_ASSERT(eval.evaluate("-INF") == std::nullopt);
    TEST_ASSERT(eval.evaluate("inf") == std::nullopt);
    TEST_ASSERT(eval.evaluate("infinity") == std::nullopt);
    TEST_ASSERT(eval.evaluate("NAN") == std::nullopt);
    TEST_ASSERT(eval.evaluate("nan") == std::nullopt);
    TEST_ASSERT(eval.evaluate("-nan") == std::nullopt);
    TEST_ASSERT(eval.evaluate("nan(hello)") == std::nullopt);
    TEST_ASSERT(eval.evaluate("NaN(hello_world123)") == std::nullopt);

    // binary
    TEST_ASSERT(eval.evaluate("0b1") == std::nullopt);
    TEST_ASSERT(eval.evaluate("0b10011") == std::nullopt);
    TEST_ASSERT(eval.evaluate("0b1") == std::nullopt);
    // octal
    TEST_ASSERT(eval.evaluate("0o1") == std::nullopt);
    TEST_ASSERT(eval.evaluate("0o01234567") == std::nullopt);

    TEST_ASSERT(eval.evaluate("123_456") == std::nullopt);
    TEST_ASSERT(eval.evaluate("1_2_3_456_0") == std::nullopt);

    {
        const char* prev_locale = std::setlocale(LC_NUMERIC, nullptr);
        if (std::setlocale(LC_NUMERIC, "fr_FR.UTF-8")) { // skip if invalid locale
            TEST_ASSERT(eval.evaluate("12,25") == 12.25L); // bug?

            std::setlocale(LC_NUMERIC, prev_locale); // restore locale
        }
    }
    TEST_SUCCESS();
};

TEST_SEQUENCE("IntegerParsing") {
    {
        wolv::math_eval::MathEvaluator<i64> eval;
        TEST_ASSERT(eval.evaluate("") == std::nullopt);
        TEST_ASSERT(eval.evaluate(" ") == std::nullopt);

        TEST_ASSERT(eval.evaluate("0") == 0);
        TEST_ASSERT(eval.evaluate("-0") == 0);
        TEST_ASSERT(eval.evaluate("1") == 1);
        TEST_ASSERT(eval.evaluate("-1") == -1);
        TEST_ASSERT(eval.evaluate("025") == 25);
        TEST_ASSERT(eval.evaluate("-044") == -44);

        TEST_ASSERT(eval.evaluate(".") == std::nullopt);
        TEST_ASSERT(eval.evaluate("0.") == std::nullopt);
        TEST_ASSERT(eval.evaluate(".0") == std::nullopt);
        TEST_ASSERT(eval.evaluate("0.0") == std::nullopt);
        TEST_ASSERT(eval.evaluate("-1.1") == std::nullopt);

        TEST_ASSERT(eval.evaluate("1234567890") == 1234567890);
        TEST_ASSERT(eval.evaluate("-1234567890") == -1234567890);

        TEST_ASSERT(eval.evaluate("9223372036854775807") == 9223372036854775807LL);
        TEST_ASSERT(eval.evaluate("9223372036854775808") == 9223372036854775807LL); // bug
        TEST_ASSERT(eval.evaluate("9999999999999999999999999") == 9223372036854775807LL); // bug
        TEST_ASSERT(eval.evaluate("-9223372036854775807") == -9223372036854775807LL);
        TEST_ASSERT(eval.evaluate("-9223372036854775808") == -9223372036854775807LL); // bug
        TEST_ASSERT(eval.evaluate("-9223372036854775809") == -9223372036854775807LL); // bug
        TEST_ASSERT(eval.evaluate("-9999999999999999999999999") == -9223372036854775807LL); // bug

        TEST_ASSERT(eval.evaluate("0x0") == 0);
        TEST_ASSERT(eval.evaluate("0x") == std::nullopt);
        TEST_ASSERT(eval.evaluate("x") == std::nullopt);
        TEST_ASSERT(eval.evaluate("x0") == std::nullopt);
        TEST_ASSERT(eval.evaluate("0x1") == 1);
        TEST_ASSERT(eval.evaluate("-0x1") == -1);

        TEST_ASSERT(eval.evaluate("1x2") == std::nullopt);
        TEST_ASSERT(eval.evaluate("-1x90") == std::nullopt);
        TEST_ASSERT(eval.evaluate("9x55") == std::nullopt);
        TEST_ASSERT(eval.evaluate("01x4") == std::nullopt);
        TEST_ASSERT(eval.evaluate("00x5") == 5); // bug
        TEST_ASSERT(eval.evaluate("-00x5") == -5); // bug
        TEST_ASSERT(eval.evaluate("1234567890x6") == 6); // bug
        TEST_ASSERT(eval.evaluate("-1234567890x6") == -6); // bug

        TEST_ASSERT(eval.evaluate("0b1") == std::nullopt);
        TEST_ASSERT(eval.evaluate("0o551") == std::nullopt);
        TEST_ASSERT(eval.evaluate("123_456") == std::nullopt);
    }

    {
        wolv::math_eval::MathEvaluator<u64> eval;
        TEST_ASSERT(eval.evaluate("") == std::nullopt);
        TEST_ASSERT(eval.evaluate(" ") == std::nullopt);

        TEST_ASSERT(eval.evaluate("0") == 0);
        TEST_ASSERT(eval.evaluate("-0") == 0);
        TEST_ASSERT(eval.evaluate("1") == 1);
        TEST_ASSERT(eval.evaluate("-1") == 18446744073709551615ULL);
        TEST_ASSERT(eval.evaluate("025") == 25);
        TEST_ASSERT(eval.evaluate("-044") == 18446744073709551572ULL);

        TEST_ASSERT(eval.evaluate(".") == std::nullopt);
        TEST_ASSERT(eval.evaluate("0.") == std::nullopt);
        TEST_ASSERT(eval.evaluate(".0") == std::nullopt);
        TEST_ASSERT(eval.evaluate("0.0") == std::nullopt);
        TEST_ASSERT(eval.evaluate("-1.1") == std::nullopt);

        TEST_ASSERT(eval.evaluate("1234567890") == 1234567890);
        TEST_ASSERT(eval.evaluate("-1234567890") == 18446744072474983726ULL);

        TEST_ASSERT(eval.evaluate("18446744073709551615") == 18446744073709551615ULL);
        TEST_ASSERT(eval.evaluate("18446744073709551616") == 18446744073709551615ULL); // bug
        TEST_ASSERT(eval.evaluate("99999999999999999999999999") == 18446744073709551615ULL); // bug
        TEST_ASSERT(eval.evaluate("-18446744073709551615") == 1ULL);
        TEST_ASSERT(eval.evaluate("-18446744073709551616") == 1ULL); // bug
        TEST_ASSERT(eval.evaluate("-99999999999999999999999999") == 1ULL); // bug

        TEST_ASSERT(eval.evaluate("0x0") == 0);
        TEST_ASSERT(eval.evaluate("0x") == std::nullopt);
        TEST_ASSERT(eval.evaluate("x") == std::nullopt);
        TEST_ASSERT(eval.evaluate("x0") == std::nullopt);
        TEST_ASSERT(eval.evaluate("0x1") == 1);
        TEST_ASSERT(eval.evaluate("-0x1") == 18446744073709551615ULL);

        TEST_ASSERT(eval.evaluate("1x2") == std::nullopt);
        TEST_ASSERT(eval.evaluate("-1x90") == std::nullopt);
        TEST_ASSERT(eval.evaluate("9x55") == std::nullopt);
        TEST_ASSERT(eval.evaluate("01x4") == std::nullopt);
        TEST_ASSERT(eval.evaluate("00x5") == 5); // bug
        TEST_ASSERT(eval.evaluate("-00x5") == 18446744073709551611ULL); // bug
        TEST_ASSERT(eval.evaluate("-1234567890x6") == 18446744073709551610ULL); // bug

        TEST_ASSERT(eval.evaluate("0b1") == std::nullopt);
        TEST_ASSERT(eval.evaluate("0o551") == std::nullopt);
        TEST_ASSERT(eval.evaluate("123_456") == std::nullopt);
    }

    {
        wolv::math_eval::MathEvaluator<i128> eval;
        TEST_ASSERT(eval.evaluate("") == std::nullopt);
        TEST_ASSERT(eval.evaluate(" ") == std::nullopt);

        TEST_ASSERT(eval.evaluate("0") == 0);
        TEST_ASSERT(eval.evaluate("-0") == 0);
        TEST_ASSERT(eval.evaluate("1") == 1);
        TEST_ASSERT(eval.evaluate("-1") == -1);
        TEST_ASSERT(eval.evaluate("025") == 25);
        TEST_ASSERT(eval.evaluate("-044") == -44);

        TEST_ASSERT(eval.evaluate("1234567890") == 1234567890);
        TEST_ASSERT(eval.evaluate("-1234567890") == -1234567890);

        TEST_ASSERT(eval.evaluate("9223372036854775807") == 9223372036854775807LL);
        TEST_ASSERT(eval.evaluate("9223372036854775808") == 9223372036854775807LL); // bug
        TEST_ASSERT(eval.evaluate("9999999999999999999999999") == 9223372036854775807LL); // bug
        TEST_ASSERT(eval.evaluate("-9223372036854775807") == -9223372036854775807LL);
        TEST_ASSERT(eval.evaluate("-9223372036854775808") == -9223372036854775807LL); // bug
        TEST_ASSERT(eval.evaluate("-9223372036854775809") == -9223372036854775807LL); // bug
        TEST_ASSERT(eval.evaluate("-9999999999999999999999999") == -9223372036854775807LL); // bug

        TEST_ASSERT(eval.evaluate("0x0") == 0);
        TEST_ASSERT(eval.evaluate("0x") == std::nullopt);
        TEST_ASSERT(eval.evaluate("x") == std::nullopt);
        TEST_ASSERT(eval.evaluate("x0") == std::nullopt);
        TEST_ASSERT(eval.evaluate("0x1") == 1);
        TEST_ASSERT(eval.evaluate("-0x1") == -1);

        TEST_ASSERT(eval.evaluate("0b1") == std::nullopt);
        TEST_ASSERT(eval.evaluate("0o551") == std::nullopt);
        TEST_ASSERT(eval.evaluate("123_456") == std::nullopt);
    }

    {
        wolv::math_eval::MathEvaluator<u128> eval;
        TEST_ASSERT(eval.evaluate("") == std::nullopt);
        TEST_ASSERT(eval.evaluate(" ") == std::nullopt);
        TEST_ASSERT(eval.evaluate("025") == 25);
        TEST_ASSERT(eval.evaluate("-044") == 340282366920938463463374607431768211412_u128);

        TEST_ASSERT(eval.evaluate("0") == 0);
        TEST_ASSERT(eval.evaluate("-0") == 0);
        TEST_ASSERT(eval.evaluate("1") == 1);
        TEST_ASSERT(eval.evaluate("-1") == 340282366920938463463374607431768211455_u128);

        TEST_ASSERT(eval.evaluate("1234567890") == 1234567890);
        TEST_ASSERT(eval.evaluate("-1234567890") == 340282366920938463463374607430533643566_u128);

        TEST_ASSERT(eval.evaluate("18446744073709551615") == 18446744073709551615ULL);
        TEST_ASSERT(eval.evaluate("18446744073709551616") == 18446744073709551615ULL); // bug
        TEST_ASSERT(eval.evaluate("99999999999999999999999999") == 18446744073709551615ULL); // bug
        TEST_ASSERT(eval.evaluate("-18446744073709551615") == 340282366920938463444927863358058659841_u128);
        TEST_ASSERT(eval.evaluate("-18446744073709551616") == 340282366920938463444927863358058659841_u128); // bug
        TEST_ASSERT(eval.evaluate("-99999999999999999999999999") == 340282366920938463444927863358058659841_u128); // bug

        TEST_ASSERT(eval.evaluate("0x0") == 0);
        TEST_ASSERT(eval.evaluate("0x") == std::nullopt);
        TEST_ASSERT(eval.evaluate("x") == std::nullopt);
        TEST_ASSERT(eval.evaluate("x0") == std::nullopt);
        TEST_ASSERT(eval.evaluate("0x1") == 1);
        TEST_ASSERT(eval.evaluate("-0x1") == 340282366920938463463374607431768211455_u128);

        TEST_ASSERT(eval.evaluate("0b1") == std::nullopt);
        TEST_ASSERT(eval.evaluate("0o551") == std::nullopt);
        TEST_ASSERT(eval.evaluate("123_456") == std::nullopt);
    }

    TEST_SUCCESS();
};

TEST_SEQUENCE("Arithmetic") {
    {
        wolv::math_eval::MathEvaluator<long double> eval;

        TEST_ASSERT(eval.evaluate("1+1") == 2.L);
        TEST_ASSERT(eval.evaluate("3 4 +") == 7.L); // bug
        TEST_ASSERT(eval.evaluate("3 5+") == 8.L); // bug
        TEST_ASSERT(eval.evaluate("1 + 2") == 3.L);
        TEST_ASSERT(eval.evaluate("1+2+3+4+5+6+7+8+9") == 45.L);
        TEST_ASSERT(eval.evaluate("1.2+3.4+5.6+7.8+9.0") == 27.L);
        TEST_ASSERT(eval.evaluate("1.2+") == std::nullopt);
        TEST_ASSERT(eval.evaluate("+123.34375") == 123.34375L);
        TEST_ASSERT(eval.evaluate("1+ 0") == 1.L);
        TEST_ASSERT(eval.evaluate("\t 0  \v+\t0\n \n \t") == 0.L);

        TEST_ASSERT(eval.evaluate("1e100+1") == 1e100L);
        TEST_ASSERT(eval.evaluate("1+1e100") == 1e100L);

        TEST_ASSERT(eval.evaluate(" 7 - 10") == -3.L);
        TEST_ASSERT(eval.evaluate("2 1 -") == 1.L); // bug
        TEST_ASSERT(eval.evaluate("1-2-3-4") == -8.L);
        TEST_ASSERT(eval.evaluate("1e100 - 1e50") == 1e100L);

        TEST_ASSERT(expect_approx_eq(eval.evaluate("2.3*5.7*11.13*17.19*23"), 57690.136791L));
        TEST_ASSERT(eval.evaluate("1e100*2") == 2e100L);
        TEST_ASSERT(eval.evaluate("2(3)") == std::nullopt);
        TEST_ASSERT(eval.evaluate("(4)(6)") == std::nullopt);

        TEST_ASSERT(eval.evaluate("1 2 + 3 *") == 7.L); // bug
        TEST_ASSERT(eval.evaluate("1 2 3 + *") == 7.L); // bug

        TEST_ASSERT(eval.evaluate("3.5 / 1") == 3.5L);
        TEST_ASSERT(eval.evaluate("234 / 1e10000") == 0.L);
        TEST_ASSERT(eval.evaluate("100 / 2.5") == 40.L);
        TEST_ASSERT(eval.evaluate("0/34") == 0.L);
        TEST_ASSERT(eval.evaluate("1/0") == std::nullopt);
        TEST_ASSERT(eval.evaluate("1/-0") == std::nullopt);
        TEST_ASSERT(eval.evaluate("0/0") == std::nullopt);

        TEST_ASSERT(eval.evaluate("2*3+4") == 10.L);
        TEST_ASSERT(eval.evaluate("2+3*4") == 14.L);
        TEST_ASSERT(eval.evaluate("(10+11)*12") == 252.L);
        TEST_ASSERT(expect_approx_eq(eval.evaluate("100/99/98"), 0.010307153164296021439L));

        TEST_ASSERT(eval.evaluate("12 % 0") == std::nullopt);
        TEST_ASSERT(eval.evaluate("5 % 4") == 1.L);
        TEST_ASSERT(eval.evaluate("-5 % 3") == -2.L);
        TEST_ASSERT(eval.evaluate("5 % -3") == 2.L);
        TEST_ASSERT(eval.evaluate("5 % 3") == 2.L);

        TEST_ASSERT(expect_approx_eq(eval.evaluate("2 ** 10"), 1024.L));
        TEST_ASSERT(expect_approx_eq(eval.evaluate("2 ** 3 ** 2"), 512.L));
        TEST_ASSERT(expect_approx_eq(eval.evaluate("2 ** 0.5"), 1.4142135623730950488L));
        TEST_ASSERT(expect_approx_eq(eval.evaluate("2 ** -1"), 0.5L));

        TEST_ASSERT(eval.evaluate("-(1 + 2)") == -3.L);
        TEST_ASSERT(eval.evaluate("+(1 + 2)") == 3.L);
        TEST_ASSERT(eval.evaluate("--5") == std::nullopt);
        TEST_ASSERT(eval.evaluate("1--2") == 3.L);
        TEST_ASSERT(eval.evaluate("1+-2") == -1.L);
        TEST_ASSERT(eval.evaluate("1-+2") == -1.L);
        TEST_ASSERT(eval.evaluate("1++2") == 3.L);
        TEST_ASSERT(eval.evaluate("1+++2") == 3.L);
        TEST_ASSERT(eval.evaluate("+.4") == 0.4L);
        TEST_ASSERT(eval.evaluate("-.4") == -0.4L);

        TEST_ASSERT(eval.evaluate("(((123.34765625)))") == 123.34765625L);
        TEST_ASSERT(eval.evaluate("1+(2*(3+4)-(5+6))") == 4.L);

        TEST_ASSERT(eval.evaluate("1+") == std::nullopt);
        TEST_ASSERT(eval.evaluate("+1+") == std::nullopt);
        TEST_ASSERT(eval.evaluate("+") == std::nullopt);
        TEST_ASSERT(eval.evaluate("5-") == std::nullopt);
        TEST_ASSERT(eval.evaluate("5--") == std::nullopt);
        TEST_ASSERT(eval.evaluate("*5") == std::nullopt);
        TEST_ASSERT(eval.evaluate("5*") == std::nullopt);
    }

    {
        wolv::math_eval::MathEvaluator<i64> eval;

        // TEST_ASSERT(eval.evaluate("9223372036854775807+1") == -9223372036854775807LL - 1LL); // UB!
        // TEST_ASSERT(eval.evaluate("-9223372036854775807-2") == -9223372036854775807LL - 2LL); // UB!
        TEST_ASSERT(eval.evaluate("-9223372036854775807-1") == -9223372036854775807LL - 1LL);

        // TEST_ASSERT(eval.evaluate("(-9223372036854775807-1)*-1") == 0LL); // UB!
        // TEST_ASSERT(eval.evaluate("(-9223372036854775807-1) % -1") == 0LL); // UB!
        // TEST_ASSERT(eval.evaluate("(-9223372036854775807-1) / -1") == 0LL); // UB!
        // TEST_ASSERT(eval.evaluate("9223372036854775807*2") == 0LL); // UB!

        TEST_ASSERT(eval.evaluate("-5 / 2") == -2);
        TEST_ASSERT(eval.evaluate("5 / -2") == -2);
        TEST_ASSERT(eval.evaluate("-5 / -2") == 2);

        TEST_ASSERT(eval.evaluate("5 % 4") == 1);
        TEST_ASSERT(eval.evaluate("-5 % 3") == -2);
        TEST_ASSERT(eval.evaluate("5 % -3") == 2);
        TEST_ASSERT(eval.evaluate("5 % 3") == 2);

        TEST_ASSERT(eval.evaluate("2 ** -1") == 0);
        TEST_ASSERT(eval.evaluate("12345 ** -3") == 0);
        TEST_ASSERT(eval.evaluate("12345 ** -123") == 0);

        TEST_ASSERT(eval.evaluate("2 ** 3 ** 2") == 512);
        TEST_ASSERT(eval.evaluate("-2 ** 2") == 4);
        TEST_ASSERT(eval.evaluate("(-2) ** 2") == 4);
    }

    {
        wolv::math_eval::MathEvaluator<u64> eval;

        TEST_ASSERT(eval.evaluate("18446744073709551615+1") == 0);
        TEST_ASSERT(eval.evaluate("0-1") == 18446744073709551615ULL);
        TEST_ASSERT(eval.evaluate("5 % 4") == 1);
        TEST_ASSERT(eval.evaluate("-5 % 3") == 2);
        TEST_ASSERT(eval.evaluate("5 % -3") == 5);
        TEST_ASSERT(eval.evaluate("5 % 3") == 2);

        TEST_ASSERT(eval.evaluate("2 ** -1") == 0);
        TEST_ASSERT(eval.evaluate("12345 ** -3") == 17317727759540968665ULL); // bug
        TEST_ASSERT(eval.evaluate("12345 ** -123") == 3916649491026205593ULL); // bug
    }

    {
        wolv::math_eval::MathEvaluator<i128> eval;

        TEST_ASSERT(eval.evaluate("9223372036854775807+1") == 9223372036854775808_i128);
        TEST_ASSERT(eval.evaluate("-9223372036854775807-1") == -9223372036854775808_i128);

        // TEST_ASSERT(eval.evaluate("9223372036854775807*9223372036854775807*9223372036854775807") == 85070591730234615893513767968506380287_i128); // UB!
        TEST_ASSERT(eval.evaluate("9223372036854775807*2") == 18446744073709551614_i128);
        TEST_ASSERT(eval.evaluate("9223372036854775808*2") == 18446744073709551614_i128); // bug
        TEST_ASSERT(eval.evaluate("99999999999999999999999999*2") == 18446744073709551614_i128); // bug

        TEST_ASSERT(eval.evaluate("2 ** -1") == 0);
        TEST_ASSERT(eval.evaluate("12345 ** -3") == 0);
        TEST_ASSERT(eval.evaluate("12345 ** -123") == 0);
    }

    {
        wolv::math_eval::MathEvaluator<u128> eval;

        TEST_ASSERT(eval.evaluate("18446744073709551615+1") == 18446744073709551616_u128);
        TEST_ASSERT(eval.evaluate("18446744073709551616+1") == 18446744073709551616_u128); // bug
        TEST_ASSERT(eval.evaluate("9999999999999999999999999999+1") == 18446744073709551616_u128); // bug
        TEST_ASSERT(eval.evaluate("0-1") == 340282366920938463463374607431768211455_u128);

        TEST_ASSERT(eval.evaluate("18446744073709551615*18446744073709551615") == 340282366920938463426481119284349108225_u128);
        TEST_ASSERT(eval.evaluate("18446744073709551615*2") == 36893488147419103230_u128);

        TEST_ASSERT(eval.evaluate("2 ** -1") == 0);
        TEST_ASSERT(eval.evaluate("3 ** -3") == 252061012534028491454351561060569045523_u128); // bug
        TEST_ASSERT(eval.evaluate("12345 ** -3") == 317573833925137075581670601247388622041_u128); // bug
        TEST_ASSERT(eval.evaluate("12345 ** -123") == 243236865493588613981336659141491743641_u128); // bug
    }

    TEST_SUCCESS();
};

TEST_SEQUENCE("LogicalOperations") {
    {
        wolv::math_eval::MathEvaluator<u64> eval;

        TEST_ASSERT(eval.evaluate("1 == 1") == 1);
        TEST_ASSERT(eval.evaluate("1 == 2") == 0);
        TEST_ASSERT(eval.evaluate("1 != 2") == 1);
        TEST_ASSERT(eval.evaluate("1 != 1") == 0);
        TEST_ASSERT(eval.evaluate("2 > 1") == 1);
        TEST_ASSERT(eval.evaluate("1 > 2") == 0);
        TEST_ASSERT(eval.evaluate("2 < 3") == 1);
        TEST_ASSERT(eval.evaluate("3 < 2") == 0);
        TEST_ASSERT(eval.evaluate("2 >= 2") == 1);
        TEST_ASSERT(eval.evaluate("3 >= 2") == 1);
        TEST_ASSERT(eval.evaluate("2 >= 3") == 0);
        TEST_ASSERT(eval.evaluate("2 <= 2") == 1);
        TEST_ASSERT(eval.evaluate("2 <= 3") == 1);
        TEST_ASSERT(eval.evaluate("2 <= 1") == 0);

        TEST_ASSERT(eval.evaluate("!0") == 1);
        TEST_ASSERT(eval.evaluate("!1") == 0);
        TEST_ASSERT(eval.evaluate("!9223372036854775807") == 0);
        TEST_ASSERT(eval.evaluate("!9223372036854775808") == 0);

        TEST_ASSERT(eval.evaluate("1 && 0") == 0);
        TEST_ASSERT(eval.evaluate("1 && 2") == 1);
        TEST_ASSERT(eval.evaluate("1 && 9223372036854775807") == 1);
        TEST_ASSERT(eval.evaluate("1 && 9223372036854775808") == 1);

        TEST_ASSERT(eval.evaluate("0 || 2") == 1);
        TEST_ASSERT(eval.evaluate("0 || 0") == 0);
        TEST_ASSERT(eval.evaluate("0 || 9223372036854775807") == 1);
        TEST_ASSERT(eval.evaluate("0 || 9223372036854775808") == 1);

        TEST_ASSERT(eval.evaluate("1 ^^ 0") == 1);
        TEST_ASSERT(eval.evaluate("1 ^^ 1") == 0);
        TEST_ASSERT(eval.evaluate("1 ^^ 4") == 1); // bug

        TEST_ASSERT(eval.evaluate("1==") == std::nullopt);
        TEST_ASSERT(eval.evaluate("==2") == std::nullopt);

        TEST_ASSERT(eval.evaluate("1~") == 18446744073709551614ULL); // bug
        TEST_ASSERT(eval.evaluate("123~~") == 123); // bug
        TEST_ASSERT(eval.evaluate("2!") == 0); // bug
        TEST_ASSERT(eval.evaluate("0!") == 1); // bug
        TEST_ASSERT(eval.evaluate("0!!") == 0); // bug
    }

    {
        wolv::math_eval::MathEvaluator<long double> eval;

        TEST_ASSERT(eval.evaluate("!0") == 1.L);
        TEST_ASSERT(eval.evaluate("!(-1)") == 0.L);
        TEST_ASSERT(eval.evaluate("!-1") == std::nullopt); // bug
        TEST_ASSERT(eval.evaluate("!2") == 0.L);
        TEST_ASSERT(eval.evaluate("!0.5") == 1.L); // bug?
        TEST_ASSERT(eval.evaluate("!0.99") == 1.L); // bug?
        TEST_ASSERT(eval.evaluate("!1") == 0.L);
        TEST_ASSERT(eval.evaluate("!1.5") == 0.L); // bug?

        TEST_ASSERT(eval.evaluate("1 && 0.5") == 0); // bug?
        TEST_ASSERT(eval.evaluate("1 && 1") == 1);
        TEST_ASSERT(eval.evaluate("1 && 1.5") == 1); // bug?
    }

    {
        wolv::math_eval::MathEvaluator<u128> eval;

        TEST_ASSERT(eval.evaluate("!0") == 1.L);
        TEST_ASSERT(eval.evaluate("!2") == 0.L);
        TEST_ASSERT(eval.evaluate("!18446744073709551615") == 0.L);
        TEST_ASSERT(eval.evaluate("!(18446744073709551615+1)") == 1.L); // bug

        TEST_ASSERT(eval.evaluate("1 && 18446744073709551615") == 1);
        TEST_ASSERT(eval.evaluate("1 && (18446744073709551615+1)") == 0); // bug
        TEST_ASSERT(eval.evaluate("18446744073709551615 && 1") == 1);
        TEST_ASSERT(eval.evaluate("(18446744073709551615+1) && 1") == 0); // bug

        TEST_ASSERT(eval.evaluate("0 || 2") == 1);
        TEST_ASSERT(eval.evaluate("0 || 0") == 0);
        TEST_ASSERT(eval.evaluate("0 || 18446744073709551615") == 1);
        TEST_ASSERT(eval.evaluate("0 || (18446744073709551615+1)") == 0); // bug

        TEST_ASSERT(eval.evaluate("1 ^^ (18446744073709551615+1)") == 1); // bug
        TEST_ASSERT(eval.evaluate("1 ^^ (18446744073709551615+2)") == 0); // bug
    }

    TEST_SUCCESS();
};

TEST_SEQUENCE("BitwiseOperations") {
    wolv::math_eval::MathEvaluator<u128> eval;

    TEST_ASSERT(eval.evaluate("5 & 3") == 1);
    TEST_ASSERT(eval.evaluate("9223372036854775807 & 9223372036854775807") == 9223372036854775807_u128);
    TEST_ASSERT(eval.evaluate("9223372036854775808 & 9223372036854775808") == 340282366920938463454151235394913435648_u128); // bug
    TEST_ASSERT(eval.evaluate("18446744073709551615 & 18446744073709551615") == 340282366920938463463374607431768211455_u128); // bug
    TEST_ASSERT(eval.evaluate("(18446744073709551615+1) & (18446744073709551615+1)") == 0); // bug

    TEST_ASSERT(eval.evaluate("5 | 3") == 7);
    TEST_ASSERT(eval.evaluate("0 | 18446744073709551615") == 340282366920938463463374607431768211455_u128); // bug
    TEST_ASSERT(eval.evaluate("0 | (18446744073709551615+1)") == 0); // bug

    TEST_ASSERT(eval.evaluate("5 ^ 3") == 6);
    TEST_ASSERT(eval.evaluate("0 ^ 18446744073709551615") == 340282366920938463463374607431768211455_u128); // bug
    TEST_ASSERT(eval.evaluate("0 ^ (18446744073709551615+1)") == 0); // bug

    TEST_ASSERT(eval.evaluate("~0") == 340282366920938463463374607431768211455_u128);
    TEST_ASSERT(eval.evaluate("~1") == 340282366920938463463374607431768211454_u128);
    TEST_ASSERT(eval.evaluate("~9223372036854775807") == 340282366920938463454151235394913435648_u128);
    TEST_ASSERT(eval.evaluate("~9223372036854775808") == 9223372036854775807_u128); // bug
    TEST_ASSERT(eval.evaluate("~18446744073709551615") == 0); // bug
    TEST_ASSERT(eval.evaluate("~(18446744073709551615+1)") == 340282366920938463463374607431768211455_u128); // bug

    TEST_ASSERT(eval.evaluate("1 << 4") == 16);
    // TEST_ASSERT(eval.evaluate("2 << -1") == std::nullopt); // UB!
    // TEST_ASSERT(eval.evaluate("1 << 64") == 1); // UB!
    // TEST_ASSERT(eval.evaluate("1024 << 63") == 9444732965739290427392_u128); // bug

    TEST_ASSERT(eval.evaluate("12345678 >> 4") == 771604);
    TEST_ASSERT(eval.evaluate("-123 >> 10") == -1); // bug
    // TEST_ASSERT(eval.evaluate("2 >> -1") == std::nullopt); // UB!
    // TEST_ASSERT(eval.evaluate("12345678 >> 64") == 771604); // UB!
    TEST_ASSERT(eval.evaluate("((18446744073709551615+1)*1234) >> 0") == 0); // bug
    TEST_ASSERT(eval.evaluate("((18446744073709551615+1)*1234) >> 30") == 0); // bug

    TEST_ASSERT(eval.evaluate("2 ## 5") == 21);
    TEST_ASSERT(eval.evaluate("18446744073709551615 ## 0") == 18446744073709551615ULL);
    // TEST_ASSERT(eval.evaluate("0 ## 18446744073709551615") == 18446744073709551615ULL); // UB!
    // TEST_ASSERT(eval.evaluate("0 ## -1") == 18446744073709551615ULL); // UB!
    TEST_ASSERT(eval.evaluate("(18446744073709551615+1) ## 0") == 0); // bug
    TEST_ASSERT(eval.evaluate("0 ## (18446744073709551615+1)") == 0); // bug
    TEST_ASSERT(eval.evaluate("1125899906842624 ## 1073741824") == 1073741824ULL); // bug

    TEST_SUCCESS();
};

TEST_SEQUENCE("Variables")
{
    {
        wolv::math_eval::MathEvaluator<long double> eval;
        eval.registerStandardVariables();

        TEST_ASSERT(eval.evaluate("ans") == 0.L);

        TEST_ASSERT(eval.evaluate("pi") == std::numbers::pi); // should be std::numbers::pi_v<long double>
        TEST_ASSERT(eval.evaluate("e") == std::numbers::e); // should be std::numbers::e_v<long double>
        TEST_ASSERT(eval.evaluate("phi") == std::numbers::phi); // should be std::numbers::phi_v<long double>

        TEST_ASSERT(eval.evaluate("2pi") == std::nullopt);
        TEST_ASSERT(eval.evaluate("pi e") == std::nullopt);
    }

    {
        wolv::math_eval::MathEvaluator<u64> eval;
        eval.registerStandardVariables();

        TEST_ASSERT(eval.evaluate("ans") == 0);

        TEST_ASSERT(eval.evaluate("pi") == 3); // bug
        TEST_ASSERT(eval.evaluate("e") == 2); // bug
        TEST_ASSERT(eval.evaluate("phi") == 1); // bug
    }

    TEST_SUCCESS();
};

TEST_SEQUENCE("Functions")
{
    wolv::math_eval::MathEvaluator<long double> eval;
    eval.registerStandardFunctions();
    eval.registerStandardVariables();

    TEST_ASSERT(eval.evaluate("abc()") == std::nullopt);
    TEST_ASSERT(eval.evaluate("abc(1)") == std::nullopt);

    TEST_ASSERT(eval.evaluate("sin(0)") == 0.L);
    TEST_ASSERT(eval.evaluate("sin(sin(sin(sin(0))))") == 0.L);
    TEST_ASSERT(eval.evaluate("sin((0))") == 0.L);
    TEST_ASSERT(eval.evaluate("sin( 0)") == 0.L);
    TEST_ASSERT(eval.evaluate("sin(0 )") == 0.L);
    TEST_ASSERT(eval.evaluate("cos(0)") == 1.L);
    TEST_ASSERT(eval.evaluate("tan(0)") == 0.L);

    TEST_ASSERT(expect_approx_eq(eval.evaluate("sin(pi)"), 0.L));
    TEST_ASSERT(expect_approx_eq(eval.evaluate("cos(pi)"), -1.L));
    TEST_ASSERT(expect_approx_eq(eval.evaluate("tan(pi)"), 0.L));
    TEST_ASSERT(expect_approx_eq(eval.evaluate("tan(pi/4)"), 1.L));

    TEST_ASSERT(eval.evaluate("ceil(1.2)") == 2.L);
    TEST_ASSERT(eval.evaluate("floor(5.5)") == 5.L);

    TEST_ASSERT(eval.evaluate("sign(-3)") == -1.L);
    TEST_ASSERT(eval.evaluate("sign(0)") == 0.L);
    TEST_ASSERT(eval.evaluate("sign(12)") == 1.L);
    TEST_ASSERT(eval.evaluate("sign(3+4-2)") == 1.L);
    TEST_ASSERT(eval.evaluate("sign(-(20*4 + 1/(1 - 2)))") == -1.L);
    TEST_ASSERT(eval.evaluate("sign(sign(0) + sin(sign(0))/3 - 2 * (0 - 100))") == 1.L);

    TEST_ASSERT(eval.evaluate("abs(0)") == 0.L);
    TEST_ASSERT(eval.evaluate("abs(4)") == 4.L);
    TEST_ASSERT(eval.evaluate("abs(-6)") == 6.L);

    TEST_ASSERT(eval.evaluate("sqrt(0)") == 0.L);
    TEST_ASSERT(expect_approx_eq(eval.evaluate("-sqrt(4)"), -2.L));
    TEST_ASSERT(expect_nan(eval.evaluate("sqrt(-1)")));
    // TEST_ASSERT(eval.evaluate("sqrt(-1) && 1") == std::nullopt); // UB! (casting NaN to integer type)

    TEST_ASSERT(eval.evaluate("ln(1)") == 0.L);
    TEST_ASSERT(expect_nan(eval.evaluate("ln(-123)")));
    TEST_ASSERT(expect_approx_eq(eval.evaluate("ln(e)"), 1.L));
    TEST_ASSERT(eval.evaluate("lb(1)") == 0.L);
    TEST_ASSERT(expect_approx_eq(eval.evaluate("lb(2)"), 1.L));
    TEST_ASSERT(eval.evaluate("log(1)") == 0.L);
    TEST_ASSERT(expect_approx_eq(eval.evaluate("log(10)"), 1.L));
    TEST_ASSERT(expect_approx_eq(eval.evaluate("log(2, 8)"), 3.L));

    TEST_ASSERT(eval.evaluate("sqrt()") == std::nullopt);
    TEST_ASSERT(eval.evaluate("sqrt (4)") == std::nullopt);
    TEST_ASSERT(eval.evaluate("sqrt(,4)") == std::nullopt);
    TEST_ASSERT(eval.evaluate("sqrt(4))") == std::nullopt);
    TEST_ASSERT(eval.evaluate("sqrt(") == std::nullopt);
    TEST_ASSERT(eval.evaluate("sqrt)") == std::nullopt);
    TEST_ASSERT(eval.evaluate("sqrt(1, 2)") == std::nullopt);
    TEST_ASSERT(eval.evaluate("log(1, 2, 3)") == std::nullopt);

    TEST_ASSERT(eval.evaluate("5sqrt()") == 5.L); // bug
    TEST_ASSERT(eval.evaluate("sqrt()2.56") == 2.56L); // bug
    TEST_ASSERT(eval.evaluate("(1 sqrt()sqrt()  sqrt())") == 1.L); // bug
    TEST_ASSERT(eval.evaluate("1 sqrt() 2 +") == 3.L); // bug
    TEST_ASSERT(eval.evaluate("sqrt()5sqrt()") == 5.L); // bug
    TEST_ASSERT(eval.evaluate("10 +sqrt()5") == 15.L); // bug

    eval.setFunction("add", [](auto args) { return args[0] + args[1]; }, 2, 2);
    TEST_ASSERT(eval.evaluate("add(1, 2)") == 3.L);
    TEST_ASSERT(eval.evaluate("add( 1 , 2 )") == 3.L);
    TEST_ASSERT(eval.evaluate("add(add(1,2),3)") == 6.L);
    TEST_ASSERT(eval.evaluate("add(add(1,add(add(2,3),add(4,5))),add(add(6,7),8))") == 36.L);
    TEST_ASSERT(eval.evaluate("add()") == std::nullopt);
    TEST_ASSERT(eval.evaluate("add(1)") == std::nullopt);
    TEST_ASSERT(eval.evaluate("add(1, 2, 3)") == std::nullopt);

    eval.setFunction("zeroArgs", [](auto args) { return 42.L; }, 0, 0);
    TEST_ASSERT(eval.evaluate("zeroArgs()") == 42.L);
    TEST_ASSERT(eval.evaluate("zeroArgs(1)") == std::nullopt);
    TEST_ASSERT(eval.evaluate("zeroArgs(1,2,3,4,5,6,7,8,9,10)") == std::nullopt);

    eval.setFunction("maybe", [](auto args) -> std::optional<long double> {
        if (args.empty()) { return std::nullopt; }
        if (args[0] == -42.L) { return std::nullopt; }
        return args[0];
    }, 0, 1);

    TEST_ASSERT(eval.evaluate("maybe()") == std::nullopt);
    TEST_ASSERT(eval.evaluate("maybe(5)") == 5.L);
    TEST_ASSERT(eval.evaluate("maybe(-42)") == std::nullopt);
    TEST_ASSERT(eval.evaluate("maybe(12, 34)") == std::nullopt);
    TEST_ASSERT(eval.evaluate("maybe(maybe())") == std::nullopt);
    TEST_ASSERT(eval.evaluate("maybe(5, maybe())") == std::nullopt);

    TEST_SUCCESS();
};

TEST_SEQUENCE("ComplexExpressions") {
    {
        wolv::math_eval::MathEvaluator<long double> eval;
        eval.registerStandardFunctions();

        TEST_ASSERT(expect_approx_eq(eval.evaluate("-sqrt(abs(-16)) + lb(1024) * 2 ** (1 + floor(1.9)) - ceil(0.1)"), 35.L));
        TEST_ASSERT(eval.evaluate("(0xFF & 0x0F) ## (0x01 << 2) | (0x01 << 7)") == 252.L);
        TEST_ASSERT(eval.evaluate("( (5 % 2 == 1) && (3 ** 2 > 8) ) * 10 + ( !(~0 == -1) + -(-5 % 3) )") == 12.L);
        TEST_ASSERT(eval.evaluate("sqrt(1.44e2) + 0x1.8p3 * sign(-3e-100)") == 0.L);
        TEST_ASSERT(eval.evaluate("( (3 != 4) + (5 <= 5) ) ## abs(floor(-9.9)) + ( (1 << 3) | (15 ^ 10) ) * 2 ** (10 % 3) / ~(-2) - 5") == 63.L);
    }

    {
        wolv::math_eval::MathEvaluator<i128> eval;

        TEST_ASSERT(eval.evaluate("(10000000000 * 10000000000 + 123456789) * 2 - 987654321") == 199999999999259259257_i128);
        TEST_ASSERT(eval.evaluate("-( (10000000000 * 10000000000) / 3 ) + ( (10000000000 * 10000000000) % 7 )") == -33333333333333333331_i128);
        TEST_ASSERT(eval.evaluate("((10000000000 * 10000000000 > 9000000000000000000) && (5000000000 * 5000000000 < 5000000000 * 6000000000)) * 10000000000 * 10000000000") == 100000000000000000000_i128);
        TEST_ASSERT(eval.evaluate("( (0x7FFFFFFFFFFFFFFF & 0xFFFFFFFF) ## 1 ) * 10000000000") == 85899345910000000000_i128);
        TEST_ASSERT(eval.evaluate("- ( (10000000000 * 10000000000 + 5000000000 * 5000000000) / 5 ) + ( (10000000000 * 10000000000) % 13 )") == -24999999999999999991_i128);
    }
    TEST_SUCCESS();
};
