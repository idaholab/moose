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
#include "RelationshipManager.h"

InputParameters
INSFVAdvectionKernel::validParams()
{
  InputParameters params = FVFluxKernel::validParams();
  MooseEnum advected_interp_method("average upwind sou min_mod vanLeer quick skewness-corrected",
                                   "upwind");
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
  {
    if (advected_interp_method == "sou")
      _advected_interp_method = InterpMethod::SOU;
    else if (advected_interp_method == "min_mod")
      _advected_interp_method = InterpMethod::MinMod;
    else if (advected_interp_method == "vanLeer")
      _advected_interp_method = InterpMethod::VanLeer;
    else if (advected_interp_method == "quick")
      _advected_interp_method = InterpMethod::QUICK;
    else
      mooseError("Unrecognized interpolation type ",
                 static_cast<std::string>(advected_interp_method));

    if (_tid == 0)
    {
      auto & factory = _app.getFactory();

      auto rm_params = factory.getValidParams("ElementSideNeighborLayers");

      rm_params.set<std::string>("for_whom") = name();
      rm_params.set<MooseMesh *>("mesh") = &const_cast<MooseMesh &>(_mesh);
      rm_params.set<Moose::RelationshipManagerType>("rm_type") =
          Moose::RelationshipManagerType::GEOMETRIC | Moose::RelationshipManagerType::ALGEBRAIC |
          Moose::RelationshipManagerType::COUPLING;
      FVKernel::setRMParams(
          _pars,
          rm_params,
          std::max((unsigned short)(3), _pars.get<unsigned short>("ghost_layers")));
      mooseAssert(rm_params.areAllRequiredParamsValid(),
                  "All relationship manager parameters should be valid.");

      auto rm_obj = factory.create<RelationshipManager>(
          "ElementSideNeighborLayers", name() + "_skew_correction", rm_params);

      // Delete the resources created on behalf of the RM if it ends up not being added to the
      // App.
      if (!_app.addRelationshipManager(rm_obj))
        factory.releaseSharedObjects(*rm_obj);
    }
  }

  const auto & velocity_interp_method = getParam<MooseEnum>("velocity_interp_method");
  if (velocity_interp_method == "average")
    _velocity_interp_method = InterpMethod::Average;
  else if (velocity_interp_method == "rc")
    _velocity_interp_method = InterpMethod::RhieChow;
  else
    mooseError("Unrecognized interpolation type ",
               static_cast<std::string>(velocity_interp_method));

  auto param_check = [&params, this](const auto & param_name)
  {
    if (params.isParamSetByUser(param_name))
      paramError(
          param_name, "This parameter is not honored by INSFVAdvectionKernels like '", name(), "'");
  };

  param_check("force_boundary_execution");
  param_check("boundaries_to_force");
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
