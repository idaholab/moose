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
#include "MooseBase.h"
#include "MooseObject.h"
#include "InputParameters.h"
#include "MooseEnum.h"
#include "MooseError.h"
#include "MooseTypes.h"
#include "MooseUtils.h"

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

void
addFVAdvectedInterpolationMethod(FEProblemBase & problem,
                                 Factory & factory,
                                 const MooseEnum & interpolation_method)
{
  const std::string method_name = interpolation_method;
  if (problem.hasFVInterpolationMethod(method_name))
    return;

  const auto method_type = fvAdvectedInterpolationMethodType(interpolation_method);

  InputParameters params = factory.getValidParams(method_type);
  problem.addFVInterpolationMethod(method_type, method_name, params);
}

std::string
fvAdvectedInterpolationMethodName(const MooseBase & obj,
                                  FEProblemBase & problem,
                                  Factory & factory,
                                  const std::string & interpolation_param,
                                  const std::string & interpolation_name_param)
{
  if (obj.isParamValid(interpolation_name_param))
    return obj.getParam<InterpolationMethodName>(interpolation_name_param);

  const auto & interpolation_method = obj.getParam<MooseEnum>(interpolation_param);
  addFVAdvectedInterpolationMethod(problem, factory, interpolation_method);
  return interpolation_method;
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
