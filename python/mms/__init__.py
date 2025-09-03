# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html
import os
import matplotlib

if not os.getenv("DISPLAY", False):
    matplotlib.use("Agg")

from .fparser import (
    FParserPrinter as FParserPrinter,
    fparser as fparser,
    print_fparser as print_fparser,
    build_hit as build_hit,
    print_hit as print_hit,
)

from .moosefunction import (
    MooseFunctionPrinter as MooseFunctionPrinter,
    moosefunction as moosefunction,
    print_moose as print_moose,
)

from .evaluate import evaluate as evaluate

from .runner import (
    run_spatial as run_spatial,
    run_temporal as run_temporal,
)

from .ConvergencePlot import ConvergencePlot as ConvergencePlot
