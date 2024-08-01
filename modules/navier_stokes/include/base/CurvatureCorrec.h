//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Moose.h"
#include "MooseUtils.h"
#include "ADReal.h"
#include "metaphysicl/raw_type.h"
#include "FEProblemBase.h"
#include "SubProblem.h"

namespace CurvatureCorrec

{

/// Function that find the friction velocity in the swirling direction due to curvature
ADReal findWStar(const ADReal mu,
                 const ADReal rho,
                 const ADReal & w,
                 const Real dist,
                 const ADReal curv_R,
                 const bool convex);
}
