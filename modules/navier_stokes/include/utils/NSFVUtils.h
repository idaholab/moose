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
#include "MooseEnum.h"
#include "MooseLinearVariableFV.h"
#include "FaceInfo.h"

#include <array>

class MooseObject;
class InputParameters;
class FEProblemBase;
class Factory;
class ElemInfo;

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
using LinearFVVelocityVariableArray = std::array<const MooseLinearVariableFVReal *, 3>;

/**
 * Adds the standard velocity-component parameters used by linear FV Navier-Stokes objects.
 */
void addLinearFVVelocityVariableParams(InputParameters & params);

/**
 * Gets and validates linear FV velocity variables from the standard u/v/w parameters.
 */
LinearFVVelocityVariableArray getLinearFVVelocityVariables(const MooseObject & obj,
                                                           FEProblemBase & problem,
                                                           THREAD_ID tid,
                                                           unsigned int dim);

/**
 * Reconstructs a velocity vector from linear FV cell-centered velocity component variables.
 */
RealVectorValue linearFVCellVelocity(const LinearFVVelocityVariableArray & velocity_vars,
                                     unsigned int dim,
                                     const ElemInfo & elem_info,
                                     const Moose::StateArg & state);

const ElemInfo & linearFVBoundaryElemInfo(const FaceInfo & fi,
                                          FaceInfo::VarFaceNeighbors face_type);
const ElemInfo & linearFVFaceSideElemInfo(const FaceInfo & fi,
                                          FaceInfo::VarFaceNeighbors face_type);
Real linearFVBoundaryNormalMultiplier(FaceInfo::VarFaceNeighbors face_type);
RealVectorValue linearFVOutwardUnitNormal(const FaceInfo & fi,
                                          FaceInfo::VarFaceNeighbors face_type);

/**
 * Enum of the advected interpolation methods supported by FVInterpolationMethod objects.
 */
MooseEnum fvAdvectedInterpolationMethods();

/**
 * Gets the FVInterpolationMethod object type for an advected interpolation method.
 * @param interpolation_method The interpolation method enum to query
 *
 * Errors when no FVInterpolationMethod equivalent is supported.
 */
std::string fvAdvectedInterpolationMethodType(const MooseEnum & interpolation_method);

/**
 * Enum of the interpolation methods supported by FVInterpolationMethod objects.
 */
MooseEnum fvFaceInterpolationMethods();

/**
 * Gets the FVInterpolationMethod object type for a face interpolation method name.
 * @param interpolation_method The interpolation method enum to query
 *
 * Errors when no FVInterpolationMethod equivalent is supported.
 */
std::string fvFaceInterpolationMethodType(const MooseEnum & interpolation_method);

/**
 * Add the FVInterpolationMethod object for a face interpolation method name, if absent.
 * @param problem The problem to which the interpolation method should be added
 * @param factory The factory used to build interpolation method parameters
 * @param interpolation_method The interpolation method enum to add
 *
 * The method name must be supported by fvFaceInterpolationMethodType().
 */
void addFVFaceInterpolationMethod(FEProblemBase & problem,
                                  Factory & factory,
                                  const MooseEnum & interpolation_method);

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
