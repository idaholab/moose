#include "EBSimpleTrees.h"

ExpressionBuilderToo::SimpleRules
getPrepRules()
{
  unsigned int temp_index = _star_index;
  _star_index = 0;
  ExpressionBuilderToo::EBTerm a('s'), b('s'), c('s'), d('s'), e('s'), f('s'), g('s'), h('s'),
      i('s'), j('s'), k('s'), l('s'), m('s'), n('s'), o('s'), p('s'), q('s'), r('s'), s('s'),
      t('s'), u('s'), v('s'), w('s'), x('s'), y('s'), z('s');

  ExpressionBuilderToo::EBTerm rest('r');

  if (_star_index > temp_index)
    _star_index = _star_index;
  else
    _star_index = temp_index;
  return SimplificationRules(
      {{a - b, a + (-1 * b)}, {a / b, a * pow(b, -1), b != 0}, {a * (b + rest), a * b + a * rest}});
}

ExpressionBuilderToo::SimpleRules
getRules()
{
  unsigned int temp_index = _star_index;
  _star_index = 0;
  ExpressionBuilderToo::EBTerm a('s'), b('s'), c('s'), d('s'), e('s'), f('s'), g('s'), h('s'),
      i('s'), j('s'), k('s'), l('s'), m('s'), n('s'), o('s'), p('s'), q('s'), r('s'), s('s'),
      t('s'), u('s'), v('s'), w('s'), x('s'), y('s'), z('s');

  ExpressionBuilderToo::EBTerm rest('r');
  ExpressionBuilderToo::EBTerm rest_two('r');
  ExpressionBuilderToo::EBTerm none('n');

  if (_star_index > temp_index)
    _star_index = _star_index;
  else
    _star_index = temp_index;
  return SimplificationRules({{pow(cos(a), 2) + pow(sin(a), 2), 1},
                              {-a, (-1) * a, a != 1},
                              {a + b * a, (b + 1) * a},
                              {a + a, 2 * a},
                              {(a == a) == rest, a == rest},
                              {(a == a) == none, true},
                              {0 + a, a},
                              {0 * a, 0},
                              {1 * a, a},
                              {a * a, pow(a, 2)},
                              {a * pow(a, b), pow(a, b + 1)},
                              {pow(pow(a, b), c), pow(a, b * c)},
                              {pow(a, b) * pow(a, c), pow(a, b + c)},
                              {pow(b, 0), 1},
                              {pow(a, 1), a},
                              {log10(a), log(a) / M_LN10},
                              {log2(a), log(a) / M_LN2},
                              {log(pow(x, y)), y * log(x)},
                              {log(a * rest), log(a) + log(rest)},
                              {exp(log(x)), x},
                              {exp(log(x) * y), pow(x, y)},
                              {exp(a), pow(M_E, a)},
                              {b * rest + c * rest, (b + c) * rest},
                              {a - b, a + (-1 * b)},
                              {a / b, a * pow(b, -1)},
                              {pow(a * rest, b), pow(a, b) * pow(rest, b)}});
}
