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
#include "NSFVUtils.h"

InputParameters
INSFVAdvectionKernel::validParams()
{
  InputParameters params = FVFluxKernel::validParams();
  params += Moose::FV::interpolationParameters();
  params.addRequiredParam<UserObjectName>("rhie_chow_user_object", "The rhie-chow user-object");
  // We need 2 ghost layers for the Rhie-Chow interpolation
  params.set<unsigned short>("ghost_layers") = 2;
  params.addRequiredParam<UserObjectName>("rhie_chow_user_object", "The rhie-chow user-object");

  // We currently do not have a need for this, boundary conditions tell us where to execute
  // advection kernels
  params.suppressParameter<bool>("force_boundary_execution");
  params.suppressParameter<std::vector<BoundaryName>>("boundaries_to_force");

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
  const bool need_more_ghosting =
      Moose::FV::setInterpolationMethods(*this, _advected_interp_method, _velocity_interp_method);
  if (need_more_ghosting && _tid == 0)
  {
    auto & factory = _app.getFactory();

    auto rm_params = factory.getValidParams("ElementSideNeighborLayers");

    rm_params.set<std::string>("for_whom") = name();
    rm_params.set<MooseMesh *>("mesh") = &const_cast<MooseMesh &>(_mesh);
    rm_params.set<Moose::RelationshipManagerType>("rm_type") =
        Moose::RelationshipManagerType::GEOMETRIC | Moose::RelationshipManagerType::ALGEBRAIC |
        Moose::RelationshipManagerType::COUPLING;
    FVKernel::setRMParams(
        _pars, rm_params, std::max((unsigned short)(3), _pars.get<unsigned short>("ghost_layers")));
    mooseAssert(rm_params.areAllRequiredParamsValid(),
                "All relationship manager parameters should be valid.");

    auto rm_obj = factory.create<RelationshipManager>(
        "ElementSideNeighborLayers", name() + "_skew_correction", rm_params);

    // Delete the resources created on behalf of the RM if it ends up not being added to the
    // App.
    if (!_app.addRelationshipManager(rm_obj))
      factory.releaseSharedObjects(*rm_obj);

    // If we need more ghosting, then we are a second-order nonlinear limiting scheme whose stencil
    // is liable to change upon wind-direction change. Consequently we need to tell our problem that
    // it's ok to have new nonzeros which may crop-up after PETSc has shrunk the matrix memory
    getCheckedPointerParam<FEProblemBase *>("_fe_problem_base")
        ->setErrorOnJacobianNonzeroReallocation(false);
  }

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
  // Boundaries to avoid come first, since they are always obeyed
  if (avoidBoundary(fi))
    return true;

  // We're not on a boundary, so technically we're not skipping a boundary
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
