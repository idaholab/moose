//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MathFVUtils.h"
#include "MooseVariableFV.h"

namespace Moose
{
namespace FV
{
ADReal
gradUDotNormal(const FaceInfo & face_info,
               const MooseVariableFV<Real> & fv_var,
               const Moose::StateArg & time,
               bool correct_skewness)

{
  return fv_var.adGradSln(face_info, time, correct_skewness) * face_info.normal();
}

bool
onBoundary(const std::set<SubdomainID> & subs, const FaceInfo & fi)
{
  if (!fi.neighborPtr())
    // We're on the exterior boundary
    return true;

  if (subs.empty())
    // The face is internal and our functor lives on all subdomains
    return false;

  const auto sub_count =
      subs.count(fi.elem().subdomain_id()) + subs.count(fi.neighbor().subdomain_id());

  switch (sub_count)
  {
    case 0:
      mooseError("We should not be calling isExtrapolatedBoundaryFace on a functor that doesn't "
                 "live on either of the face information's neighboring elements");

    case 1:
      // We only live on one of the subs
      return true;

    case 2:
      // We live on both of the subs
      return false;

    default:
      mooseError("There should be no other sub_count options");
  }
}

MooseEnum
interpolationMethods()
{
  return MooseEnum("average upwind sou min_mod vanLeer quick venkatakrishnan skewness-corrected",
                   "upwind");
}

InputParameters
advectedInterpolationParameter()
{
  auto params = emptyInputParameters();
  params.addParam<MooseEnum>("advected_interp_method",
                             interpolationMethods(),
                             "The interpolation to use for the advected quantity. Options are "
                             "'upwind', 'average', 'sou' (for second-order upwind), 'min_mod', "
                             "'vanLeer', 'quick', 'venkatakrishnan', and "
                             "'skewness-corrected' with the default being 'upwind'.");
  return params;
}

InterpMethod
selectInterpolationMethod(const std::string & interp_method)
{
  if (interp_method == "average")
    return InterpMethod::Average;
  else if (interp_method == "harmonic")
    return InterpMethod::HarmonicAverage;
  else if (interp_method == "skewness-corrected")
    return InterpMethod::SkewCorrectedAverage;
  else if (interp_method == "upwind")
    return InterpMethod::Upwind;
  else if (interp_method == "rc")
    return InterpMethod::RhieChow;
  else if (interp_method == "vanLeer")
    return InterpMethod::VanLeer;
  else if (interp_method == "min_mod")
    return InterpMethod::MinMod;
  else if (interp_method == "sou")
    return InterpMethod::SOU;
  else if (interp_method == "quick")
    return InterpMethod::QUICK;
  else if (interp_method == "venkatakrishnan")
    return InterpMethod::Venkatakrishnan;
  else
    mooseError("Interpolation method ",
               interp_method,
               " is not currently an option in Moose::FV::selectInterpolationMethod");
}

bool
setInterpolationMethod(const MooseObject & obj,
                       Moose::FV::InterpMethod & interp_method,
                       const std::string & param_name)
{
  bool need_more_ghosting = false;

  const auto & interp_method_in = obj.getParam<MooseEnum>(param_name);
  interp_method = selectInterpolationMethod(interp_method_in);

  if (interp_method == InterpMethod::SOU || interp_method == InterpMethod::MinMod ||
      interp_method == InterpMethod::VanLeer || interp_method == InterpMethod::QUICK ||
      interp_method == InterpMethod::Venkatakrishnan)
    need_more_ghosting = true;

  return need_more_ghosting;
}
}
}
