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
#include "FVBoundaryScalarLagrangeMultiplierConstraint.h"

InputParameters
INSFVAdvectionKernel::validParams()
{
  InputParameters params = FVFluxKernel::validParams();
  params += Moose::FV::interpolationParameters();
  params.addRequiredParam<UserObjectName>("rhie_chow_user_object", "The rhie-chow user-object");
  // We need 2 ghost layers for the Rhie-Chow interpolation
  params.set<unsigned short>("ghost_layers") = 2;

  // We currently do not have a need for this, boundary conditions tell us where to execute
  // advection kernels
  params.suppressParameter<bool>("force_boundary_execution");

  return params;
}

INSFVAdvectionKernel::INSFVAdvectionKernel(const InputParameters & params)
  : FVFluxKernel(params),
    _rc_vel_provider(getUserObject<INSFVRhieChowInterpolator>("rhie_chow_user_object"))
{
  const bool need_more_ghosting =
      Moose::FV::setInterpolationMethods(*this, _advected_interp_method, _velocity_interp_method);
  if (need_more_ghosting && _tid == 0)
  {
    adjustRMGhostLayers(std::max((unsigned short)(3), _pars.get<unsigned short>("ghost_layers")));

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

  // Selected boundaries to force
  for (const auto bnd_to_force : _boundaries_to_force)
    if (fi.boundaryIDs().count(bnd_to_force))
      return false;

  // If we have flux bcs then we do skip
  const auto & [have_flux_bcs, flux_bcs] = _var.getFluxBCs(fi);
  libmesh_ignore(have_flux_bcs);
  for (const auto * const flux_bc : flux_bcs)
    // If we have something like an average-value pressure constraint on a flow boundary, then we
    // still want to execute this advection kernel on the boundary to ensure we're enforcing local
    // conservation (mass in this example)
    if (!dynamic_cast<const FVBoundaryScalarLagrangeMultiplierConstraint *>(flux_bc))
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
