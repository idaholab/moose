//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSADHeatConductionTimeDerivative.h"
#include "INSADObjectTracker.h"
#include "FEProblemBase.h"

registerMooseObject("NavierStokesApp", INSADHeatConductionTimeDerivative);

InputParameters
INSADHeatConductionTimeDerivative::validParams()
{
  InputParameters params = ADTimeDerivative::validParams();
  params.addClassDescription(
      "AD Time derivative term $\\rho c_p \\frac{\\partial T}{\\partial t}$ of "
      "the heat equation for quasi-constant specific heat $c_p$ and the density $\\rho$.");
  return params;
}

INSADHeatConductionTimeDerivative::INSADHeatConductionTimeDerivative(
    const InputParameters & parameters)
  : ADTimeDerivative(parameters),
    _temperature_td_strong_residual(getADMaterialProperty<Real>("temperature_td_strong_residual"))
{
  // Bypass the UserObjectInterface method because it requires a UserObjectName param which we
  // don't need
  auto & obj_tracker = const_cast<INSADObjectTracker &>(
      _fe_problem.getUserObject<INSADObjectTracker>("ins_ad_object_tracker"));
  for (const auto block_id : blockIDs())
    obj_tracker.set("has_energy_transient", true, block_id);
}

ADReal
INSADHeatConductionTimeDerivative::precomputeQpResidual()
{
  return _temperature_td_strong_residual[_qp];
}
