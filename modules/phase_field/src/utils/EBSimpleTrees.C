#include "EBSimpleTrees.h"

std::vector<std::pair<ExpressionBuilderToo::EBTermNode *, ExpressionBuilderToo::EBTermNode *>>
getPrepRules()
{
  ExpressionBuilderToo::EBTerm a(0, true), b(1, true), c(2, true), d(3, true), e(4, true),
      f(5, true), g(6, true), h(7, true), i(8, true), j(9, true), k(10, true), l(11, true),
      m(12, true), n(13, true), o(14, true), p(15, true), q(16, true), r(17, true), s(18, true),
      t(19, true), u(20, true), v(21, true), w(22, true), x(23, true), y(24, true), z(25, true);

  return SimplificationRules(
      {{a - b, a + (-1 * b)}, {a / b, a * pow(b, -1)}, {a * (b + c), a * b + a * c}});
}

std::vector<std::pair<ExpressionBuilderToo::EBTermNode *, ExpressionBuilderToo::EBTermNode *>>
getRules()
{
  ExpressionBuilderToo::EBTerm a(0, true), b(1, true), c(2, true), d(3, true), e(4, true),
      f(5, true), g(6, true), h(7, true), i(8, true), j(9, true), k(10, true), l(11, true),
      m(12, true), n(13, true), o(14, true), p(15, true), q(16, true), r(17, true), s(18, true),
      t(19, true), u(20, true), v(21, true), w(22, true), x(23, true), y(24, true), z(25, true);

  ExpressionBuilderToo::EBTerm rest(true);

  return SimplificationRules({{pow(cos(a), 2) + pow(sin(a), 2), 1},
                              {a + b * a, (b + 1) * a},
                              {b * a + c * a, (b + c) * a},
                              {a + a, 2 * a},
                              {0 + a, a},
                              {0 * a, 0},
                              {1 * a, a},
                              {a * a, pow(a, 2)},
                              {a * pow(a, b), pow(a, b + 1)},
                              {pow(pow(a, b), c), pow(a, b * c)},
                              {pow(a, b) * pow(a, c), pow(a, b + c)},
                              {pow(b, 0), 1},
                              {pow(a, 1), a},
                              {pow(a, b) * pow(a, c), pow(a, b + c)},
                              {log10(a), log(a) / M_LN10},
                              {log2(a), log(a) / M_LN2},
                              {log(pow(x, y)), y * log(x)},
                              {log(a * rest), log(a) + log(rest)},
                              {exp(log(x)), x},
                              {exp(log(x) * y), pow(x, y)},
                              {exp(a), pow(M_E, a)},
                              {a - b, a + (-1 * b)},
                              {a / b, a * pow(b, -1)},
                              {pow(a * rest, b), pow(a, b) * pow(rest, b)}});
}
