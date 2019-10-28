#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html
try:
    import sympy

except ImportError:
    print("The 'mms' package requires sympy for symbolic function evaluation, it can be installed " \
          "by running `pip install sympy --user`.")
else:
    from .fparser import FParserPrinter, fparser, print_fparser, build_hit, print_hit
    from .moosefunction import MooseFunctionPrinter, moosefunction, print_moose
    from .evaluate import evaluate

from .runner import run_spatial, run_temporal

try:
    import os
    import matplotlib
    if not os.getenv('DISPLAY', False):
        matplotlib.use('Agg')

except ImportError:
    print("The 'mms' package requires matplotlib for the ConvergencePlot object, matplotlib can " \
          "be installed by running `pip install matplotlib --user`.")
else:
    from .ConvergencePlot import ConvergencePlot
