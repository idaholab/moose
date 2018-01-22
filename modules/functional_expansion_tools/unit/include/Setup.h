// This file is part of the MOOSE framework
// https://www.mooseframework.org
//
// All rights reserved, see COPYRIGHT for full restrictions
// https://github.com/idaholab/moose/blob/master/COPYRIGHT
//
// Licensed under LGPL 2.1, please see LICENSE for details
// https://www.gnu.org/licenses/lgpl-2.1.html

#include "Cartesian.h"
#include "CylindricalDuo.h"
#include "Legendre.h"
#include "Zernike.h"

#ifndef SETUP_H
#define SETUP_H

const double tol = 1e-13;

extern MooseEnum SingleSeriesTypes1D;
extern MooseEnum SingleSeriesTypes2D;

#endif // SETUP_H
