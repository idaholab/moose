//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "WCNSFVFlowPhysics.h"
#include "NSFVAction.h"

InputParameters
WCNSFVFlowPhysics::validParams()
{
  InputParameters params = WCNSFVPhysicsBase::validParams();
  params.addClassDescription(
      "Define the Navier Stokes weakly-compressible mass and momentum equations");

  // Pressure pin parameters
  params.transferParam<bool>(NSFVAction::validParams(), "pin_pressure");
  params.transferParam<MooseEnum>(NSFVAction::validParams(), "pinned_pressure_type");
  params.transferParam<Point>(NSFVAction::validParams(), "pinned_pressure_point");
  params.transferParam<PostprocessorName>(NSFVAction::validParams(), "pinned_pressure_value");

  // Initialization parameters
  params.transferParam<std::vector<FunctionName>>(NSFVAction::validParams(), "initial_velocity");
  params.transferParam<FunctionName>(NSFVAction::validParams(), "initial_pressure");

  // Friction correction, a technique to limit oscillations at friction interfaces
  params.transferParam<bool>(NSFVAction::validParams(), "use_friction_correction");
  params.transferParam<Real>(NSFVAction::validParams(), "consistent_scaling");

  // Boussinesq approximation
  params.transferParam<bool>(NSFVAction::validParams(), "boussinesq_approximation");
  params.transferParam<Real>(NSFVAction::validParams(), "ref_temperature");
  params.transferParam<MooseFunctorName>(NSFVAction::validParams(), "thermal_expansion");

  // Volumetric friction terms, mostly used for porous media modeling
  params.transferParam<std::vector<std::vector<SubdomainName>>>(NSFVAction::validParams(),
                                                                "friction_blocks");
  params.transferParam<std::vector<std::vector<std::string>>>(NSFVAction::validParams(),
                                                              "friction_types");
  params.transferParam<std::vector<std::vector<std::string>>>(NSFVAction::validParams(),
                                                              "friction_coeffs");

  // Spatial discretization scheme
  params.transferParam<MooseEnum>(NSFVAction::validParams(), "mass_advection_interpolation");
  params.transferParam<MooseEnum>(NSFVAction::validParams(), "momentum_advection_interpolation");

  // Nonlinear solver parameters
  params.transferParam<Real>(NSFVAction::validParams(), "mass_scaling");
  params.transferParam<Real>(NSFVAction::validParams(), "momentum_scaling");

  return params;
}

WCNSFVFlowPhysics::WCNSFVFlowPhysics(const InputParameters & parameters)
  : WCNSFVPhysicsBase(parameters)
{
  // checkTwoDVectorParamsSameLength<BoundaryName, MooseEnum>("friction_blocks", "friction_types");
  // checkTwoDVectorParamsSameLength<BoundaryName, MooseEnum>("friction_blocks", "friction_coeffs");
}
