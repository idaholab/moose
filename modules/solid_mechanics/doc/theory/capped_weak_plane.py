#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import os
import time
import sys
import matplotlib
import re
from optparse import OptionParser, OptionValueError

import numpy as np
import matplotlib.pyplot as plt
import scipy.linalg

def sw(x, eps):
    if (x < - eps):
        return 0
    return 0.5 * (x + eps) - eps / np.pi * np.cos(0.5 * np.pi * x / eps)

def yield_function(x, y, coh, tanphi, tensile, compressive, tip_s, s_tol):
    yf_raw = [0, 0, 0]
    yf_raw[0] = np.sqrt(y * y + tip_s * tip_s) + x * tanphi - coh
    yf_raw[1] = x - tensile
    yf_raw[2] = - x - compressive

    # sort in ascending order
    yf = sorted(yf_raw)

    # perform the smoothing
    if yf[2] > yf[1] + s_tol:
        return yf[2]

    yfnew = yf[2] + sw(yf[1] - yf[2], s_tol)

    if yfnew > yf[0] + s_tol:
        return yfnew

    return yfnew + sw(yf[0] - yfnew, s_tol)


def mat_yf(x, y, coh, tanphi, tensile, compressive, tip_s, s_tol):
    return [[yield_function(i, j, coh, tanphi, tensile, compressive, tip_s, s_tol) for i in x] for j in y]


# parse command line
p = OptionParser(usage="""usage: %prog [options] <cohesion> <tan_phi> <tensile> <compressive>
Produce a picture of the capped weak-plane yield surface in p-q space\n""")
p.add_option("--tip_smoother", action="store", type="float", dest="tip_smoother", help="Tip smoother.  Default is 0.1 * cohesion.  This parameter should be non-negative")
p.add_option("--stol", action="store", type="float", dest="smoothing_tol", help="Smoothing tolerance at the corners.  Default is 0.1 * cohesion.  This parameter should be non-negative")
p.add_option("-o", action="store", type="str", dest="output", default="cwp.png", help="Output filename (default: %default)")
p.add_option("--pmin", action="store", type="float", dest="pmin", default=-1.0, help="Horizontal axis will run between pmin and pmax (default: %default)");
p.add_option("--pmax", action="store", type="float", dest="pmax", default=1.0, help="Horizontal axis will run between pmin and pmax (default: %default)");
p.add_option("--qmax", action="store", type="float", dest="qmax", default=1.0, help="Vertical axis will run between 0 and qmax (default: %default)");
p.add_option("--np", action="store", type="int", dest="np", default=100, help="Horizontal discretisation (default: %default)");
p.add_option("--nq", action="store", type="int", dest="nq", default=100, help="Vertical discretisation (default: %default)");
p.add_option("--fmin", action="store", type="float", dest="fmin", default=-1.0, help="Yield-function contours will be plotted for yield-function values between fmin and fmax (default: %default)")
p.add_option("--fmax", action="store", type="float", dest="fmax", default=1.0, help="Yield-function contours will be plotted for yield-function values between fmin and fmax (default: %default)")
p.add_option("--nf", action="store", type="int", dest="nf", default=3, help="This many yield-function contours will be plotted.  If this is set to 1 or 0 then only the fmax contour will be plotted.  If this is set to a negative number, then negative-nf default contours will be shown (default: %default)")
p.add_option("-l", action="store_true", dest="labels", help="Include labels on the contours")
(opts, args) = p.parse_args()
if len(args) != 4:
    p.print_help()
    sys.exit(1)
(coh, tanphi, tensile, compressive) = map(float, args)
tip_s = (opts.tip_smoother if opts.tip_smoother >= 0 else 0.1 * coh)
s_tol = (opts.smoothing_tol if opts.smoothing_tol >= 0 else 0.1 * coh)

plt.figure()
x = np.arange(opts.pmin, opts.pmax, (opts.pmax - opts.pmin) / opts.np)
y = np.arange(0, opts.qmax, opts.qmax / opts.nq)
levels = [opts.fmax]
if opts.nf < 0:
    levels = - opts.nf
elif opts.nf > 1:
    delf = (opts.fmax - opts.fmin) / (opts.nf - 1)
    levels = np.arange(opts.fmin, opts.fmax + delf, delf)
CS = plt.contour(x, y, mat_yf(x, y, coh, tanphi, tensile, compressive, tip_s, s_tol), levels)
if opts.labels:
    plt.clabel(CS, inline=1, fontsize=10)
plt.savefig(opts.output)


sys.exit(0)

