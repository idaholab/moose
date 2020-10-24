//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSADEnergySource.h"
#include "INSADObjectTracker.h"
#include "FEProblemBase.h"

registerMooseObject("NavierStokesApp", INSADEnergySource);

InputParameters
INSADEnergySource::validParams()
{
  InputParameters params = ADKernelValue::validParams();
  params.addClassDescription("Computes an arbitrary volumetric heat source (or sink).");
  params.addCoupledVar(
      "source_variable",
      "Variable describing the volumetric heat source. Note that if this variable evaluates to a "
      "negative number, then this object will be an energy sink");
  params.addParam<FunctionName>(
      "source_function",
      "Function describing the volumetric heat source. Note that if this function evaluates to a "
      "negative number, then this object will be an energy sink");
  return params;
}

INSADEnergySource::INSADEnergySource(const InputParameters & parameters)
  : ADKernelValue(parameters),
    _temperature_source_strong_residual(
        getADMaterialProperty<Real>("temperature_source_strong_residual"))
{
  bool has_coupled = isCoupled("source_variable");
  bool has_function = isParamValid("source_function");
  if (!has_coupled && !has_function)
    mooseError("Either the 'source_variable' or 'source_function' param must be set for the "
               "'INSADMomentumCoupledForce' object");
  else if (has_coupled && has_function)
    mooseError("Both the 'source_variable' or 'source_function' param are set for the "
               "'INSADMomentumCoupledForce' object. Please use one or the other.");

  if (has_coupled && coupledComponents("source_variable") != 1)
    paramError("source_variable", "Only expect one variable for the 'source_variable' parameter");

  // Bypass the UserObjectInterface method because it requires a UserObjectName param which we
  // don't need
  auto & obj_tracker = const_cast<INSADObjectTracker &>(
      _fe_problem.getUserObject<INSADObjectTracker>("ins_ad_object_tracker"));
  for (const auto block_id : blockIDs())
  {
    obj_tracker.set("has_heat_source", true, block_id);
    if (has_coupled)
      obj_tracker.set("heat_source_var", getVar("source_variable", 0)->name(), block_id);
    else if (has_function)
      obj_tracker.set("heat_source_function", getParam<FunctionName>("source_function"), block_id);
  }
}

ADReal
INSADEnergySource::precomputeQpResidual()
{
  return _temperature_source_strong_residual[_qp];
}
