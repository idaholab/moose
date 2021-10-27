#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

try:
    from sympy.printing.cxx import CXX11CodePrinter
except:
    from sympy.printing.cxxcode import CXX11CodePrinter

class MooseFunctionPrinter(CXX11CodePrinter):
    """sympy printer for MOOSE C++ Function objects."""
    def _print_BaseScalar(self, expr):
        """
        Print p(0), p(1), p(2) instead of R.x, R.y, or R.z for inserting into value method

        see sympy/sympy/vector/scalar.py
        """
        return 'p({})'.format(expr._id[0])

    def _print_Symbol(self, expr):
        """
        Print _u instead of u, following the MOOSE member variable convention.
        """
        s = str(expr)
        if s.endswith(('_x', '_y', '_z')):
            return '_{}({})'.format(s[:-2], 'xyz'.index(s[-1]))

        elif s != 't':
            return '_{}'.format(expr)

        else:
            return str(expr)

def moosefunction(expr, assign_to=None, **kwargs):
    """
    Converts an expr to an MOOSE C++ expression for Function objects.

    Inputs:
      expr[sympy.core.Expr]: a sympy expression to be converted
    """
    return MooseFunctionPrinter(**kwargs).doprint(expr, assign_to)

def print_moose(expr, **kwargs):
    """
    Prints a C++ expression for a Function object.
    """
    print(moosefunction(expr, **kwargs))
