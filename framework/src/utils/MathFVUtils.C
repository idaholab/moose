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

InterpMethod
selectInterpolationMethod(const std::string & interp_method)
{
  if (interp_method == "average")
    return Moose::FV::InterpMethod::Average;
  else if (interp_method == "harmonic")
    return Moose::FV::InterpMethod::HarmonicAverage;
  else if (interp_method == "skewness-corrected")
    return Moose::FV::InterpMethod::SkewCorrectedAverage;
  else if (interp_method == "upwind")
    return Moose::FV::InterpMethod::Upwind;
  else if (interp_method == "rc")
    return Moose::FV::InterpMethod::RhieChow;
  else if (interp_method == "vanLeer")
    return Moose::FV::InterpMethod::VanLeer;
  else if (interp_method == "min_mod")
    return Moose::FV::InterpMethod::MinMod;
  else if (interp_method == "sou")
    return Moose::FV::InterpMethod::SOU;
  else if (interp_method == "quick")
    return Moose::FV::InterpMethod::QUICK;
  else
    mooseError("Interpolation method ",
               interp_method,
               " is not currently an option in Moose::FV::selectInterpolationMethod");
}
}
}
