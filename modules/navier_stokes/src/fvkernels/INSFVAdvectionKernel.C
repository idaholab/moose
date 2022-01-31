//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSFVAdvectionKernel.h"
#include "NS.h"
#include "MooseVariableFV.h"

InputParameters
INSFVAdvectionKernel::validParams()
{
  InputParameters params = FVFluxKernel::validParams();
  MooseEnum advected_interp_method("average upwind skewness-corrected", "upwind");
  params.addParam<MooseEnum>(
      "advected_interp_method",
      advected_interp_method,
      "The interpolation to use for the advected quantity. Options are "
      "'upwind', 'average', and 'skewness-corrected' with the default being 'upwind'.");
  MooseEnum velocity_interp_method("average rc", "rc");
  params.addParam<MooseEnum>(
      "velocity_interp_method",
      velocity_interp_method,
      "The interpolation to use for the velocity. Options are "
      "'average' and 'rc' which stands for Rhie-Chow. The default is Rhie-Chow.");
  params.addRequiredParam<UserObjectName>("rhie_chow_user_object", "The rhie-chow user-object");
  // We need 2 ghost layers for the Rhie-Chow interpolation
  params.set<unsigned short>("ghost_layers") = 2;
  params.addRequiredParam<UserObjectName>("rhie_chow_user_object", "The rhie-chow user-object");
  return params;
}

INSFVAdvectionKernel::INSFVAdvectionKernel(const InputParameters & params)
  : FVFluxKernel(params),
    _rc_vel_provider(getUserObject<INSFVRhieChowInterpolator>("rhie_chow_user_object"))
{
#ifndef MOOSE_GLOBAL_AD_INDEXING
  mooseError("INSFV is not supported by local AD indexing. In order to use INSFV, please run the "
             "configure script in the root MOOSE directory with the configure option "
             "'--with-ad-indexing-type=global'");
#endif
  using namespace Moose::FV;

  const auto & advected_interp_method = getParam<MooseEnum>("advected_interp_method");
  if (advected_interp_method == "average")
    _advected_interp_method = InterpMethod::Average;
  else if (advected_interp_method == "skewness-corrected")
    _advected_interp_method = Moose::FV::InterpMethod::SkewCorrectedAverage;
  else if (advected_interp_method == "upwind")
    _advected_interp_method = InterpMethod::Upwind;
  else
    mooseError("Unrecognized interpolation type ",
               static_cast<std::string>(advected_interp_method));

  const auto & velocity_interp_method = getParam<MooseEnum>("velocity_interp_method");
  if (velocity_interp_method == "average")
    _velocity_interp_method = InterpMethod::Average;
  else if (velocity_interp_method == "rc")
    _velocity_interp_method = InterpMethod::RhieChow;
  else
    mooseError("Unrecognized interpolation type ",
               static_cast<std::string>(velocity_interp_method));
}

void
INSFVAdvectionKernel::initialSetup()
{
  INSFVBCInterface::initialSetup(*this);
}

bool
INSFVAdvectionKernel::skipForBoundary(const FaceInfo & fi) const
{
  if (!onBoundary(fi))
    return false;

  // If we have flux bcs then we do skip
  const auto & flux_pr = _var.getFluxBCs(fi);
  if (flux_pr.first)
    return true;

  // If we have a flow boundary without a replacement flux BC, then we must not skip. Mass and
  // momentum are transported via advection across boundaries
  for (const auto bc_id : fi.boundaryIDs())
    if (_flow_boundaries.find(bc_id) != _flow_boundaries.end())
      return false;

  // If not a flow boundary, then there should be no advection/flow in the normal direction, e.g. we
  // should not contribute any advective flux
  return true;
}
