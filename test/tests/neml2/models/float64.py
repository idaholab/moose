# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details

"""Set the embedded eager runtime's tensor default to MOOSE's Float64 precision."""

import torch

# Parameter-less eager models infer their dtype from Python's torch default.
torch.set_default_dtype(torch.float64)
