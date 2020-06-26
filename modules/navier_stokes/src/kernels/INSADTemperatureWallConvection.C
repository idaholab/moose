//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSADTemperatureWallConvection.h"
#include "INSADObjectTracker.h"
#include "FEProblemBase.h"

registerMooseObject("NavierStokesApp", INSADTemperatureWallConvection);

InputParameters
INSADTemperatureWallConvection::validParams()
{
  InputParameters params = ADKernelValue::validParams();
  params.addClassDescription(
      "Computes a heat source/sink due to convection from ambient walls/surroundings.");
  params.addRequiredParam<Real>(
      "alpha", "The heat transfer coefficient from the ambient walls/surroundings");
  params.addRequiredParam<Real>("T_wall", "The wall/ambient temperature");
  return params;
}

INSADTemperatureWallConvection::INSADTemperatureWallConvection(const InputParameters & parameters)
  : ADKernelValue(parameters),
    _temperature_wall_convection_strong_residual(
        getADMaterialProperty<Real>("temperature_wall_convection_strong_residual"))
{
  // Bypass the UserObjectInterface method because it requires a UserObjectName param which we
  // don't need
  auto & obj_tracker = const_cast<INSADObjectTracker &>(
      _fe_problem.getUserObject<INSADObjectTracker>("ins_ad_object_tracker"));
  obj_tracker.set("has_wall_convection", true);
  obj_tracker.set("wall_convection_alpha", getParam<Real>("alpha"));
  obj_tracker.set("wall_temperature", getParam<Real>("T_wall"));
}

ADReal
INSADTemperatureWallConvection::precomputeQpResidual()
{
  return _temperature_wall_convection_strong_residual[_qp];
}
