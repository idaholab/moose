//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NSFVUtils.h"
#include "FEProblemBase.h"
#include "Factory.h"
#include "MooseObject.h"
#include "InputParameters.h"
#include "MooseEnum.h"
#include "MooseError.h"
#include "MooseTypes.h"
#include "MooseUtils.h"
#include "FEProblemBase.h"
#include "ElemInfo.h"

namespace Moose
{
namespace FV
{
bool
setInterpolationMethods(const MooseObject & obj,
                        Moose::FV::InterpMethod & advected_interp_method,
                        Moose::FV::InterpMethod & velocity_interp_method)
{
  const bool need_more_ghosting =
      setInterpolationMethod(obj, advected_interp_method, "advected_interp_method");

  const auto & velocity_interp_method_in = obj.getParam<MooseEnum>("velocity_interp_method");
  if (velocity_interp_method_in == "average")
    velocity_interp_method = InterpMethod::Average;
  else if (velocity_interp_method_in == "rc")
    velocity_interp_method = InterpMethod::RhieChow;
  else
    obj.mooseError("Unrecognized interpolation type ", std::string(velocity_interp_method_in));

  return need_more_ghosting;
}

InputParameters
interpolationParameters()
{
  auto params = advectedInterpolationParameter();
  MooseEnum velocity_interp_method("average rc", "rc");
  params.addParam<MooseEnum>(
      "velocity_interp_method",
      velocity_interp_method,
      "The interpolation to use for the velocity. Options are "
      "'average' and 'rc' which stands for Rhie-Chow. The default is Rhie-Chow.");
  return params;
}
}
}

namespace NS
{
MooseEnum
fvAdvectedInterpolationMethods()
{
  return MooseEnum("average upwind vanLeer min_mod venkatakrishnan", "upwind");
}

std::string
fvAdvectedInterpolationMethodType(const MooseEnum & interpolation_method)
{
  const std::string method_name = interpolation_method;
  if (interpolation_method == "average")
    return "FVGeometricAverage";
  if (interpolation_method == "upwind")
    return "FVAdvectedUpwind";
  if (interpolation_method == "vanLeer")
    return "FVAdvectedVanLeerWeightBased";
  if (interpolation_method == "min_mod")
    return "FVAdvectedMinmodWeightBased";
  if (interpolation_method == "venkatakrishnan")
    return "FVAdvectedVenkatakrishnanDeferredCorrection";

  mooseError("Unsupported linear FV advected interpolation method '", method_name, "'.");
}

MooseEnum
fvFaceInterpolationMethods()
{
  return MooseEnum("average harmonic");
}

std::string
fvFaceInterpolationMethodType(const MooseEnum & interpolation_method)
{
  const std::string method_name = interpolation_method;
  if (interpolation_method == "average")
    return "FVGeometricAverage";
  if (interpolation_method == "harmonic")
    return "FVHarmonicAverage";

  mooseError("Unsupported linear FV face interpolation method '", method_name, "'.");
}

void
addFVFaceInterpolationMethod(FEProblemBase & problem,
                             Factory & factory,
                             const MooseEnum & interpolation_method)
{
  const std::string method_name = interpolation_method;
  if (problem.hasFVInterpolationMethod(method_name))
    return;

  const auto method_type = fvFaceInterpolationMethodType(interpolation_method);

  InputParameters params = factory.getValidParams(method_type);
  problem.addFVInterpolationMethod(method_type, method_name, params);
}

void
addLinearFVVelocityVariableParams(InputParameters & params)
{
  params.addRequiredParam<SolverVariableName>("u", "The velocity in the x direction.");
  params.addParam<SolverVariableName>("v", "The velocity in the y direction.");
  params.addParam<SolverVariableName>("w", "The velocity in the z direction.");
}

LinearFVVelocityVariableArray
getLinearFVVelocityVariables(const MooseObject & obj,
                             FEProblemBase & problem,
                             const THREAD_ID tid,
                             const unsigned int dim)
{
  LinearFVVelocityVariableArray velocity_vars{nullptr, nullptr, nullptr};

  auto get_velocity_var = [&](const std::string & param_name)
  {
    return dynamic_cast<const MooseLinearVariableFVReal *>(
        &problem.getVariable(tid, obj.getParam<SolverVariableName>(param_name)));
  };

  velocity_vars[0] = get_velocity_var("u");
  if (!velocity_vars[0])
    obj.paramError("u", "the u velocity must be a MooseLinearVariableFVReal.");

  if (dim >= 2)
  {
    if (!obj.isParamValid("v"))
      obj.paramError("v", "In two or more dimensions, the v velocity must be supplied.");
    velocity_vars[1] = get_velocity_var("v");
    if (!velocity_vars[1])
      obj.paramError("v",
                     "In two or more dimensions, the v velocity must be supplied and it must be a "
                     "MooseLinearVariableFVReal.");
  }

  if (dim >= 3)
  {
    if (!obj.isParamValid("w"))
      obj.paramError("w", "In three dimensions, the w velocity must be supplied.");
    velocity_vars[2] = get_velocity_var("w");
    if (!velocity_vars[2])
      obj.paramError("w",
                     "In three dimensions, the w velocity must be supplied and it must be a "
                     "MooseLinearVariableFVReal.");
  }

  return velocity_vars;
}

RealVectorValue
linearFVCellVelocity(const LinearFVVelocityVariableArray & velocity_vars,
                     const unsigned int dim,
                     const ElemInfo & elem_info,
                     const Moose::StateArg & state)
{
  RealVectorValue velocity;
  for (const auto dim_i : make_range(dim))
    velocity(dim_i) = velocity_vars[dim_i]->getElemValue(elem_info, state);

  return velocity;
}

const ElemInfo &
linearFVBoundaryElemInfo(const FaceInfo & fi, const FaceInfo::VarFaceNeighbors face_type)
{
  return face_type == FaceInfo::VarFaceNeighbors::NEIGHBOR ? *fi.neighborInfo() : *fi.elemInfo();
}

const ElemInfo &
linearFVFaceSideElemInfo(const FaceInfo & fi, const FaceInfo::VarFaceNeighbors face_type)
{
  return face_type == FaceInfo::VarFaceNeighbors::ELEM ? *fi.elemInfo() : *fi.neighborInfo();
}

Real
linearFVBoundaryNormalMultiplier(const FaceInfo::VarFaceNeighbors face_type)
{
  return face_type == FaceInfo::VarFaceNeighbors::NEIGHBOR ? -1.0 : 1.0;
}

RealVectorValue
linearFVOutwardUnitNormal(const FaceInfo & fi, const FaceInfo::VarFaceNeighbors face_type)
{
  auto normal = fi.normal();
  normal *= linearFVBoundaryNormalMultiplier(face_type);

  const Real normal_magnitude = normal.norm();
  if (normal_magnitude <= libMesh::TOLERANCE)
    return RealVectorValue();

  return normal / normal_magnitude;
}

template <class T>
std::tuple<bool, T, T>
isPorosityJumpFace(const Moose::FunctorBase<T> & porosity,
                   const FaceInfo & fi,
                   const Moose::StateArg & time)
{
  if (!fi.neighborPtr() || (fi.elem().subdomain_id() == fi.neighbor().subdomain_id()))
    // We've agreed to only support porosity jump treatment at subdomain boundaries
    return {false, 0, 0};

  mooseAssert(porosity.hasBlocks(fi.elem().subdomain_id()) &&
                  porosity.hasBlocks(fi.neighbor().subdomain_id()),
              "Porosity should have blocks on both elem and neighbor");

  const Moose::FaceArg face_elem{
      &fi, Moose::FV::LimiterType::CentralDifference, true, false, fi.elemPtr(), nullptr};
  const Moose::FaceArg face_neighbor{
      &fi, Moose::FV::LimiterType::CentralDifference, true, false, fi.neighborPtr(), nullptr};
  const auto eps_elem = porosity(face_elem, time), eps_neighbor = porosity(face_neighbor, time);
  return {!MooseUtils::relativeFuzzyEqual(eps_elem, eps_neighbor), eps_elem, eps_neighbor};
}

template std::tuple<bool, Real, Real> isPorosityJumpFace<Real>(
    const Moose::FunctorBase<Real> & porosity, const FaceInfo & fi, const Moose::StateArg & time);
template std::tuple<bool, ADReal, ADReal> isPorosityJumpFace<ADReal>(
    const Moose::FunctorBase<ADReal> & porosity, const FaceInfo & fi, const Moose::StateArg & time);
}
