#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

"""
FParser printer

The FParserPrinter converts single sympy expressions into a single FParser.
"""

from sympy.printing.codeprinter import CodePrinter
from sympy.printing.precedence import precedence
from sympy import ccode

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

    def __init__(self, **kwargs):
        """Register function mappings supplied by user"""
        CodePrinter.__init__(self, kwargs)
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

    def _print_BaseScalar(self, expr):
        """
        Print simple variable names instead of R.variable_name

        see sympy/sympy/vector/scalar.py
        """
        index, system = expr._id
        return system._variable_names[index]

    def _print_Rational(self, expr):
        p, q = int(expr.p), int(expr.q)
        return '%d/%d' % (p, q)

    def _print_Indexed(self, expr):
        raise TypeError("FParserPrinter does not support array indices")

    def _print_Idx(self, expr):
        raise TypeError("FParserPrinter does not support array indices")

    def _print_Exp1(self, expr):
        return 'exp(1)'

    #def _print_Pi(self, expr):
    #    return '3.14159265359'

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


def fparser(expr, assign_to=None, **kwargs):
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
    return FParserPrinter(**kwargs).doprint(expr, assign_to)[-1]

def print_fparser(expr, **kwargs):
    """Prints an FParser representation of the given expression."""
    print(str(fparser(expr, **kwargs)))

def build_hit(expr, name, **kwargs):
    """
    Create a hit node containing a ParsedFunction of the given expression

    Inputs:
        expr[sympy.core.Expr]: The sympy expression to convert
        name[str]: The name of the input file block to create
        kwargs: Key, value pairs for val, vals input parameters (defaults to 1.0) if not provided
    """
    import pyhit

    if hasattr(expr, 'free_symbols'):
        symbols = set([str(s) for s in expr.free_symbols]).difference(set(['R.x', 'R.y', 'R.z', 't']))
    else:
        symbols = set()
    for symbol in symbols:
        kwargs.setdefault(symbol, 1.)

    root = pyhit.Node(None, name)
    root['type'] = 'ParsedFunction'
    root['value'] = "'{}'".format(str(fparser(expr)))

    if kwargs:
        pvars = ' '.join(kwargs.keys())
        pvals = ' '.join([str(v) for v in kwargs.values()])
        root['vars'] = "'{}'".format(pvars)
        root['vals'] = "'{}'".format(pvals)

    return root

def print_hit(*args, **kwargs):
    """Prints a hit block containing a ParsedFunction of the given expression"""
    root = build_hit(*args, **kwargs)
    print(root.render())
