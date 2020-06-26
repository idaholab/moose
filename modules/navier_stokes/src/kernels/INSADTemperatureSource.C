//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSADTemperatureSource.h"
#include "INSADObjectTracker.h"
#include "FEProblemBase.h"

registerMooseObject("NavierStokesApp", INSADTemperatureSource);

InputParameters
INSADTemperatureSource::validParams()
{
  InputParameters params = ADKernelValue::validParams();
  params.addClassDescription("Computes an arbitrary volumetric heat source (or sink).");
  params.addRequiredParam<FunctionName>(
      "source_function",
      "Function describing the volumetric heat source. Note that if this function evaluates to a "
      "negative number, then this object will be an energy sink");
  return params;
}

INSADTemperatureSource::INSADTemperatureSource(const InputParameters & parameters)
  : ADKernelValue(parameters),
    _temperature_source_strong_residual(
        getADMaterialProperty<Real>("temperature_source_strong_residual"))
{
  // Bypass the UserObjectInterface method because it requires a UserObjectName param which we
  // don't need
  auto & obj_tracker = const_cast<INSADObjectTracker &>(
      _fe_problem.getUserObject<INSADObjectTracker>("ins_ad_object_tracker"));
  obj_tracker.set("has_heat_source", true);
  obj_tracker.set("heat_source_function", &getFunction("source_function"));
}

ADReal
INSADTemperatureSource::precomputeQpResidual()
{
  return _temperature_source_strong_residual[_qp];
}
