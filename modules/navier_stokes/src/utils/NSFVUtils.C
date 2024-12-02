//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NSFVUtils.h"
#include "MooseObject.h"
#include "InputParameters.h"
#include "MooseEnum.h"
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
    obj.mooseError("Unrecognized interpolation type ",
                   static_cast<std::string>(velocity_interp_method_in));

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
std::tuple<bool, ADReal, ADReal>
isPorosityJumpFace(const Moose::Functor<ADReal> & porosity,
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
}
