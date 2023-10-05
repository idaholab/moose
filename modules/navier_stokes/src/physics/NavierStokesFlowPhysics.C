//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NavierStokesFlowPhysics.h"
#include "NSFVAction.h"

InputParameters
NavierStokesFlowPhysics::validParams()
{
  InputParameters params = PhysicsBase::validParams();
  params.addClassDescription("Define the Navier Stokes equation");

  // We pull in parameters from various flow objects. This helps make sure the parameters are
  // spelled the same way and match the evolution of other objects.
  // If we remove these objects, or change their parameters, these parameters should be updated
  // Downstream actions must either implement all these options, or redefine the parameter with
  // a restricted MooseEnum, or place an error in the constructor for unsupported configurations

  params.transferParam<RealVectorValue>(NSFVAction::validParams(), "gravity");

  // Pull the variables from the NSFV Action
  params.transferParam<std::vector<std::string>>(NSFVAction::validParams(), "velocity_variable");
  params.transferParam<NonlinearVariableName>(NSFVAction::validParams(), "pressure_variable");
  params.transferParam<NonlinearVariableName>(NSFVAction::validParams(),
                                              "fluid_temperature_variable");

  // Pull the boundary parameters from NSFV Action
  params.transferParam<std::vector<BoundaryName>>(NSFVAction::validParams(), "inlet_boundaries");
  params.transferParam<std::vector<BoundaryName>>(NSFVAction::validParams(), "outlet_boundaries");
  params.transferParam<std::vector<BoundaryName>>(NSFVAction::validParams(), "wall_boundaries");
  params.transferParam<std::vector<MooseEnum>>(NSFVAction::validParams(), "momentum_inlet_types");
  params.transferParam<std::vector<MooseEnum>>(NSFVAction::validParams(), "momentum_outlet_types");
  params.transferParam<std::vector<MooseEnum>>(NSFVAction::validParams(), "momentum_wall_types");

  // Material properties
  params.transferParam<MooseFunctorName>(NSFVAction::validParams(), "dynamic_viscosity");
  params.transferParam<MooseFunctorName>(NSFVAction::validParams(), "density");

  return params;
}

NavierStokesFlowPhysics::NavierStokesFlowPhysics(const InputParameters & parameters)
  : PhysicsBase(parameters)
{
  // Parameter checking
  checkVectorParamsSameLength<BoundaryName, MooseEnum>("inlet_boundaries", "momentum_inlet_types");
  checkVectorParamsSameLength<BoundaryName, MooseEnum>("outlet_boundaries",
                                                       "momentum_outlet_types");
  checkVectorParamsSameLength<BoundaryName, MooseEnum>("wall_boundaries", "momentum_wall_types");
  checkVectorParamsNoOverlap<BoundaryName>(
      {"inlet_boundaries", "outlet_boundaries", "wall_boundaries"});
}
