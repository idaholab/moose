//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NavierStokesPhysicsBase.h"
#include "INSFVRhieChowInterpolator.h"
#include "RelationshipManager.h"
#include "WCNSFVFlowPhysics.h"

InputParameters
NavierStokesPhysicsBase::validParams()
{
  InputParameters params = PhysicsBase::validParams();
  params.addClassDescription(
      "Base class to define the Navier Stokes incompressible and weakly-compressible equation");

  params.addParam<bool>(
      "define_variables",
      true,
      "Whether to define variables if the variables with the specified names do not exist. Note "
      "that if the variables are defined externally from the Physics, the initial conditions will "
      "not be created in the Physics either.");

  params.addParam<unsigned short>(
      "ghost_layers", 2, "Number of layers of elements to ghost near process domain boundaries");
  params.addParamNamesToGroup("define_variables ghost_layers", "Advanced");

  return params;
}

NavierStokesPhysicsBase::NavierStokesPhysicsBase(const InputParameters & parameters)
  : PhysicsBase(parameters), _define_variables(getParam<bool>("define_variables"))
{
}

InputParameters
NavierStokesPhysicsBase::getAdditionalRMParams() const
{
  unsigned short necessary_layers = getParam<unsigned short>("ghost_layers");
  necessary_layers = std::max(necessary_layers, getNumberAlgebraicGhostingLayersNeeded());

  // Just an object that has a ghost_layers parameter
  const std::string kernel_type = "INSFVMixingLengthReynoldsStress";
  InputParameters params = getFactory().getValidParams(kernel_type);
  params.template set<unsigned short>("ghost_layers") = necessary_layers;

  return params;
}
