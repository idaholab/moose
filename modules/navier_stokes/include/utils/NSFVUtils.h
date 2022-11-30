//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MathFVUtils.h"

class MooseObject;
class InputParameters;

namespace Moose
{
namespace FV
{
/**
 * Sets the advection and velocity interpolation methods
 * @param obj The \p MooseObject with input parameters to query
 * @param advected_interp_method The advected interpolation method we will set
 * @param velocity_interp_method The velocity interpolation method we will set
 * @return Whether the interpolation methods have indicated that we will need more than the
 * default level of ghosting
 */
bool setInterpolationMethods(const MooseObject & obj,
                             Moose::FV::InterpMethod & advected_interp_method,
                             Moose::FV::InterpMethod & velocity_interp_method);

/**
 * @return interpolation parameters for use in advection object input parameters
 */
InputParameters interpolationParameters();
}
}

namespace NS
{
std::tuple<bool, ADReal, ADReal> isPorosityJumpFace(const Moose::Functor<ADReal> & porosity,
                                                    const FaceInfo & fi);
}
