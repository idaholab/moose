#include "EBSimpleTrees.h"

PrepEBSimpleTrees::PrepEBSimpleTrees()
{
  ExpressionBuilder::EBTerm a(0, true), b(1, true), c(2, true), d(3, true), e(4, true), f(5, true),
      g(6, true), h(7, true), i(8, true), j(9, true), k(10, true), l(11, true), m(12, true),
      n(13, true), o(14, true), p(15, true), q(16, true), r(17, true), s(18, true), t(19, true),
      u(20, true), v(21, true), w(22, true), x(23, true), y(24, true), z(25, true);

  _prep_simplification_trees = SimplificationRules({{a - b, a + (-b)}, {a / b, a * pow(b, -1)}});
}

EBSimpleTrees::EBSimpleTrees()
{
  ExpressionBuilder::EBTerm a(0, true), b(1, true), c(2, true), d(3, true), e(4, true), f(5, true),
      g(6, true), h(7, true), i(8, true), j(9, true), k(10, true), l(11, true), m(12, true),
      n(13, true), o(14, true), p(15, true), q(16, true), r(17, true), s(18, true), t(19, true),
      u(20, true), v(21, true), w(22, true), x(23, true), y(24, true), z(25, true);

  _simplification_trees = SimplificationRules({{pow(cos(a), 2) + pow(sin(b), 2), 1},
                                               {a + (-a), 0},
                                               {a + 0, a},
                                               {a + a, 2 * a},
                                               {a * a, pow(a, 2)},
                                               {a * 0, 0}});
}
