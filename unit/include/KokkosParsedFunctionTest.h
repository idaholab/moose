//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosParsedFunction.h"

#include "gtest_include.h"

class KokkosParsedFunctionTestObject
{
public:
  KokkosParsedFunctionTestObject(const std::map<std::string, Real> & constants,
                                 const std::string & expression)
    : _builder(expression)
  {
    _result.create(1);

    for (const auto & it : constants)
    {
      _builder.addVariable(it.first);
      _builder.associateValue(it.first, &it.second);
    }

    _builder.build();

    _builder.finalize();

    _evaluator.init(_builder);
  }

  KOKKOS_FUNCTION void operator()(const int) const
  {
    _result[0] = _evaluator.eval(0, Moose::Kokkos::Real3(0));
  }

  Real result()
  {
    _result.copyToHost();
    return _result[0];
  }

private:
  Moose::Kokkos::Array<Real> _result;
  Moose::Kokkos::RPNBuilder _builder;
  Moose::Kokkos::RPNEvaluator _evaluator;
};

class KokkosParsedFunctionTest : public ::testing::Test
{
public:
  struct Expected
  {
    std::string canonical;
    double value;
  };

  // clang-format off

  // ---- Constants for testing ----
  const std::map<std::string, Real> constants = {
    {"a",  2},
    {"b",  3},
    {"c",  4},
    {"d",  5},
    {"e",  6},
    {"f",  7},
    {"x",  0.5},
    {"y",  -1.5},
    {"z",  0},
    {"u",  1},
    {"v",  -2},
    {"PI", 3.141592653589793}
  };

  // ---- Valid expressions: canonical + evaluated value ----
  const std::map<std::string, Expected> valid_cases = {
    // --- Numbers (all forms) ---
    {"42",                    {"42",                              42}},
    {".5",                    {".5",                              0.5}},
    {".5e-2",                 {".5e-2",                           0.005}},
    {"2.",                    {"2.",                              2.0}},
    {"2.0e+10",               {"2.0e+10",                         20000000000.0}},
    {".0E+0",                 {".0E+0",                           0.0}},
    {"00",                    {"00",                              0}},

    // --- Identifiers (assuming bindings: a=2, x=0.5) ---
    {"x",                     {"x",                               0.5}},
    {"a",                     {"a",                               2}},

    // --- Add/Mul precedence & left-associativity ---
    {"a + b * c",             {"a + (b * c)",                     14}},
    {"a * b + c",             {"(a * b) + c",                     10}},
    {"a - b - c",             {"(a - b) - c",                     -5}},
    {"2 * 3 / 4",             {"(2 * 3) / 4",                     1.5}},
    {"16 / 4 / 2",            {"(16 / 4) / 2",                    2}},

    // --- Unary ---
    {"-x",                    {"-(x)",                            -0.5}},
    {"!x",                    {"!(x)",                            0}},
    {"!z",                    {"!(z)",                            1}},
    {"- ! - x",               {"-(!(-(x)))",                      0}},
    {"-(a+b)",                {"-((a + b))",                      -5}},

    // --- Power (right-associative) ---
    {"2 ^ 3 ^ 2",             {"2 ^ (3 ^ 2)",                     512}},
    {"-x ^ 2",                {"-(x ^ 2)",                        -0.25}},
    {"pow(2, 3)",             {"pow(2, 3)",                       8}},
    {"pow(2,3)^2",            {"(pow(2, 3)) ^ 2",                 64}},
    {"2 ^ 3 * 4",             {"(2 ^ 3) * 4",                     32}},
    {"2 ^ (3 * 4)",           {"2 ^ (3 * 4)",                     4096}},

    // --- Logical AND/OR precedence & left-assoc ---
    {"(a | b) & z",           {"(a | b) & z",                     0}},
    {"a | b & z",             {"a | (b & z)",                     1}},
    {"z | z",                 {"z | z",                           0}},
    {"z & a",                 {"z & a",                           0}},
    {"a + b & c",             {"(a + b) & c",                     1}},

    // --- Comparisons (non-associative) ---
    {"a > b",                 {"a > b",                           0}},
    {"a < b",                 {"a < b",                           1}},
    {"c >= d",                {"c >= d",                          0}},
    {"c <= d",                {"c <= d",                          1}},
    {"a + d != b + c",        {"(a + d) != (b + c)",              0}},
    {"a + d = b + c",         {"(a + d) = (b + c)",               1}},
    {"a + d <= b + c",        {"(a + d) <= (b + c)",              1}},
    {"a + d >= b + c",        {"(a + d) >= (b + c)",              1}},
    {"a + d < b + c",         {"(a + d) < (b + c)",               0}},
    {"a + d > b + c",         {"(a + d) > (b + c)",               0}},

    // --- Mixed Cmp with And/Or (Cmp > And > Or) ---
    {"a < b & c < d",         {"(a < b) & (c < d)",               1}},
    {"a < b | c < z",         {"(a < b) | (c < z)",               1}},
    {"b & c < d",             {"b & (c < d)",                     1}},

    // --- Parentheses & whitespace ---
    {"(1 + 2) * 3",           {"(1 + 2) * 3",                     9}},
    {"((a))",                 {"(((a)))",                         2}},
    {"  \t\n  sin (  x  )  ", {"sin(x)",                          0.479425538604203}},

    // --- Trig & reciprocals (radians), PI=3.141592653589793 ---
    {"sin(0)",                {"sin(0)",                          0.0}},
    {"cos(0)",                {"cos(0)",                          1.0}},
    {"tan(PI / 4)",           {"tan((PI / 4))",                   1.0}},
    {"sec(0)",                {"sec(0)",                          1.0}},
    {"csc(PI / 2)",           {"csc((PI / 2))",                   1.0}},
    {"cot(PI / 4)",           {"cot((PI / 4))",                   1.0}},
    {"sin(x)",                {"sin(x)",                          0.479425538604203}},
    {"cos(x)",                {"cos(x)",                          0.8775825618903728}},
    {"tan(x)",                {"tan(x)",                          0.5463024898437905}},

    // --- Hyperbolic ---
    {"sinh(1)",               {"sinh(1)",                         1.1752011936438014}},
    {"cosh(0)",               {"cosh(0)",                         1.0}},
    {"tanh(1)",               {"tanh(1)",                         0.7615941559557649}},

    // --- Logs & exponents ---
    {"exp(1)",                {"exp(1)",                          2.718281828459045}},
    {"exp2(10)",              {"exp2(10)",                        1024.0}},
    {"log(1)",                {"log(1)",                          0.0}},
    {"log10(100)",            {"log10(100)",                      2.0}},
    {"log2(8)",               {"log2(8)",                         3.0}},

    // --- Roots ---
    {"sqrt(9)",               {"sqrt(9)",                         3.0}},
    {"cbrt(27)",              {"cbrt(27)",                        3.0}},

    // --- Rounding / integer ops ---
    {"abs(y)",                {"abs(y)",                          1.5}},
    {"ceil(y)",               {"ceil(y)",                         -1.0}},
    {"floor(y)",              {"floor(y)",                        -2.0}},
    {"trunc(y)",              {"trunc(y)",                        -1.0}},
    {"int(3.7)",              {"round(3.7)",                      4.0}},
    {"int(y)",                {"round(y)",                        -2.0}},

    // --- Inverse trig (safe domains) ---
    {"acos(1)",               {"acos(1)",                         0.0}},
    {"asin(0)",               {"asin(0)",                         0.0}},
    {"atan(1)",               {"atan(1)",                         0.7853981633974483}},
    {"atan2(0, -1)",          {"atan2(0, -1)",                    3.141592653589793}},
    {"acosh(1)",              {"acosh(1)",                        0.0}},
    {"asinh(0)",              {"asinh(0)",                        0.0}},
    {"atanh(0.5)",            {"atanh(0.5)",                      0.5493061443340548}},

    // --- Hypotenuse / min/max ---
    {"hypot(3, 4)",           {"hypot(3, 4)",                     5.0}},
    {"max(a, b)",             {"max(a, b)",                       3.0}},
    {"min(a, b)",             {"min(a, b)",                       2.0}},

    // --- Conditional ---
    {"if(a < b, 10, 20)",     {"if((a < b), 10, 20)",             10}},
    {"if(z, 10, 20)",         {"if(z, 10, 20)",                   20}},
    {"if(!(a<b), 10, 20)",    {"if(!(a < b), 10, 20)",            20}},

    // --- Big mixed precedence check ---
    {"a | b & c ^ d + e * f", {"a | (b & ((c ^ d) + (e * f)))",   1}},

    // --- Nested function arguments ---
    {"abs(-pow(2,3))",        {"abs(-(pow(2, 3)))",               8.0}},
    {"max(sin(0), cos(0))",   {"max(sin(0), cos(0))",             1.0}},

    // --- More precedence mixes ---
    {"a + (b & (c < d))",     {"a + (b & (c < d))",               3}},
    {"2 * (3 + 4)",           {"2 * (3 + 4)",                     14}},
    {"(2 + 3) * 4",           {"(2 + 3) * 4",                     20}}
  };

  // ---- Invalid parse cases: expression -> reason ----
  const std::map<std::string, std::string> invalid_cases = {
    {".",            "Isolated dot is not a Number"},
    {"1abc",         "Identifier must start with letter/underscore; '1abc' doesn't"},
    {"a < b < c",    "Comparisons are non-associative (at most one comparison)"},
    {"1 = 2 != 3",   "Two comparisons in one expression are not allowed"},
    {"f(,a)",        "ArgList requires Expr before comma"},
    {"f(a,)",        "Trailing comma not allowed"},
    {"(1 + 2",       "Unmatched '('"},
    {"1 + 2)",       "Unmatched ')'"},
    {"a && b",       "Only single '&' supported"},
    {"a || b",       "Only single '|' supported"},
    {"a % b",        "Modulus not in grammar (MulOp is '*' or '/')"},
    {"a +",          "Incomplete expression"},
    {"!",            "Unary requires operand"}
  };

  // ---- Semantic errors: parse OK but evaluation should fail ----
  const std::map<std::string, std::string> semantic_error_cases = {
    {"if(a, b)",     "if() expects 3 argument(s)."},
    {"exp()",        "argument(s) missing for function 'exp()'."},
    {"max",          "variable name 'max' is reserved for function."},
    {"foo()",        "unsupported function 'foo()'."}
  };

  // clang-format on
};
