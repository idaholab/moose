//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "WCNSFVPhysicsBase.h"
#include "NSFVAction.h"

InputParameters
WCNSFVPhysicsBase::validParams()
{
  InputParameters params = NavierStokesFlowPhysics::validParams();
  params.addClassDescription(
      "Base class to define the Navier Stokes incompressible and weakly-compressible equation");

  // We pull in parameters from various flow objects. This helps make sure the parameters are
  // spelled the same way and match the evolution of other objects.
  // If we remove these objects, or change their parameters, these parameters should be updated

  // Specify the weakly compressible boundary types
  params.transferParam<MultiMooseEnum>(NSFVAction::validParams(), "momentum_inlet_function");
  params.transferParam<std::vector<PostprocessorName>>(NSFVAction::validParams(), "flux_inlet_pps");
  params.transferParam<std::vector<Point>>(NSFVAction::validParams(), "flux_inlet_directions");
  params.transferParam<std::vector<FunctionName>>(NSFVAction::validParams(), "pressure_function");

  // Specify the numerical schemes for interpolations of velocity and pressure
  params.transferParam<MooseEnum>(NSFVAction::validParams(), "velocity_interpolation");
  params.transferParam<MooseEnum>(NSFVAction::validParams(), "pressure_face_interpolation");
  params.transferParam<MooseEnum>(NSFVAction::validParams(), "velocity_face_interpolation");

  return params;
}

WCNSFVPhysicsBase::WCNSFVPhysicsBase(const InputParameters & parameters)
  : NavierStokesFlowPhysics(parameters),
    _velocity_interpolation(getParam<MooseEnum>("velocity_interpolation"))
{
  // Parameter checking
  // checkVectorParamsSameLengthOrZero<BoundaryName, MooseEnum>("inlet_boundaries",
  // "flux_inlet_pps");
  // checkVectorParamsSameLengthOrZero<BoundaryName,
  // MooseEnum>("inlet_boundaries",
  //                                                            "flux_inlet_directions");
}
