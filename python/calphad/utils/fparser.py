"""
FParser printer

The FParserPrinter converts single sympy expressions into a single FParser.
"""

from __future__ import print_function, division

from sympy import S, C
from sympy.core.compatibility import string_types
from sympy.printing.codeprinter import CodePrinter
from sympy.printing.precedence import precedence

# dictionary mapping sympy function to (argument_conditions, fparser function).
# Used in FParserPrinter._print_Function(self)
known_functions = {
  "Abs": "abs",
  "sin": "sin",
  "cos": "cos",
  "tan": "tan",
  "asin": "asin",
  "acos": "acos",
  "atan": "atan",
  "atan2": "atan2",
  "exp": "exp",
  "log": "log",
  "erf": "erf",
  "sinh": "sinh",
  "cosh": "cosh",
  "tanh": "tanh",
  "asinh": "asinh",
  "acosh": "acosh",
  "atanh": "atanh",
  "floor": "floor",
  "ceiling": "ceil",
}


class FParserPrinter(CodePrinter):
    """A printer to convert python expressions to FParser expressions"""
    printmethod = "_fparser"

    _default_settings = {
      'order': None,
      'human': False,
      'full_prec': 'auto',
      'precision': 15,
    }

    # ovewrite some operators (FParser uses single char and/or)
    _operators = {
      'and': '&',
      'or': '|',
      'not': '!',
    }

    def __init__(self, settings={}):
        """Register function mappings supplied by user"""
        CodePrinter.__init__(self, settings)
        self.known_functions = dict(known_functions)

    def _rate_index_position(self, p):
        """function to calculate score based on position among indices

        This method is used to sort loops in an optimized order, see
        CodePrinter._sort_optimized()
        """
        return p*5

    def _format_code(self, lines):
        return lines

    def _get_statement(self, codestring):
        return "%s;" % codestring

    def _get_loop_opening_ending(self, indices):
        return '',''
        #raise TypeError("FParserPrinter does not support loops")

    def _print_Pow(self, expr):
        PREC = precedence(expr)
        if expr.exp == -1:
            return '1/%s' % (self.parenthesize(expr.base, PREC))
        elif expr.exp == 0.5:
            return 'sqrt(%s)' % self._print(expr.base)
        elif expr.base == 2:
            return 'exp2(%s)' % self._print(expr.exp)
        else:
            return '%s^%s' % (self.parenthesize(expr.base, PREC),
                     self.parenthesize(expr.exp, PREC))

    def _print_Rational(self, expr):
        p, q = int(expr.p), int(expr.q)
        return '%d/%d' % (p, q)

    def _print_Indexed(self, expr):
        raise TypeError("FParserPrinter does not support array indices")

    def _print_Idx(self, expr):
        raise TypeError("FParserPrinter does not support array indices")

    def _print_Exp1(self, expr):
        return 'exp(1)'

    def _print_Pi(self, expr):
        return '3.14159265359'

    # TODO: we need a more elegant way to deal with infinity in FParser
    def _print_Float(self, expr):
        if expr == float("inf"):
            return "1e200"
        elif expr == float("-inf"):
            return "-1e200"
        else:
            return CodePrinter._print_Float(self, expr)

    def _print_Infinity(self, expr):
        return '1e200'

    def _print_NegativeInfinity(self, expr):
        return '-1e200'

    def _print_Piecewise(self, expr):
        ecpairs = ["if(%s,%s" % (self._print(c), self._print(e))
              for e, c in expr.args[:-1]]

        if expr.args[-1].cond == True:
            ecpairs.append("%s" % self._print(expr.args[-1].expr))
        else:
            # there is no default value, so we generate an invalid expression
            # that will fail at runtime
            ecpairs.append("if(%s,%s,0/0)" %
                    (self._print(expr.args[-1].cond),
                    self._print(expr.args[-1].expr)))
        return ",".join(ecpairs) + ")" * (len(ecpairs)-1)


def fparser(expr, assign_to=None, **settings):
    r"""Converts an expr to an FParser expression

      Parameters
      ==========

      expr : sympy.core.Expr
        a sympy expression to be converted
      precision : optional
        the precision for numbers such as pi [default=15]


      Examples
      ========

      >>> from sympy import ccode, symbols, Rational, sin, ceiling, Abs
      >>> x, tau = symbols(["x", "tau"])
      >>> ccode((2*tau)**Rational(7,2))
      '8*sqrt(2)*pow(tau, 7.0L/2.0L)'
      >>> fparser(sin(x), assign_to="s")
      's = sin(x);'
    """
    return FParserPrinter(settings).doprint(expr, assign_to)


def print_fparser(expr, **settings):
    """Prints an FParser representation of the given expression."""
    print(ccode(expr, **settings))
