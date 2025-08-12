//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
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
/**
 * Checks to see whether the porosity value jumps from one side to the other of the provided face
 * @param porosity the porosity
 * @param fi the face to inspect for porosity jumps
 * @param time A temporal argument indicating at what time state to evaluate the porosity
 * @return a tuple where the zeroth member indicates whether there is a jump, the first member is
 * the porosity value on the "element" side of the face, and the second member is the porosity value
 * on the "neighbor" side of the face
 */
template <class T>
std::tuple<bool, T, T> isPorosityJumpFace(const Moose::FunctorBase<T> & porosity,
                                          const FaceInfo & fi,
                                          const Moose::StateArg & time);

extern template std::tuple<bool, Real, Real> isPorosityJumpFace<Real>(
    const Moose::FunctorBase<Real> & porosity, const FaceInfo & fi, const Moose::StateArg & time);
extern template std::tuple<bool, ADReal, ADReal> isPorosityJumpFace<ADReal>(
    const Moose::FunctorBase<ADReal> & porosity, const FaceInfo & fi, const Moose::StateArg & time);
}
