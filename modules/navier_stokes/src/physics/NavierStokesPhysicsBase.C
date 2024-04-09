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

  params.addParam<PhysicsName>("coupled_flow_physics",
                               "WCNSFVFlowPhysics generating the velocities");
  params.addParam<bool>(
      "define_variables",
      true,
      "Whether to define variables if the variables with the specified names do not exist");

  params.addParam<unsigned short>(
      "ghost_layers", 2, "Number of layers of elements to ghost near process domain boundaries");
  params.addParamNamesToGroup("define_variables ghost_layers", "Advanced");

  return params;
}

NavierStokesPhysicsBase::NavierStokesPhysicsBase(const InputParameters & parameters)
  : PhysicsBase(parameters), _define_variables(getParam<bool>("define_variables"))
{
  // Annoying edge case. We cannot use ANY_BLOCK_ID for kernels and variables since errors got added
  // downstream for using it, we cannot leave it empty as that sets all objects to not live on any
  // block
  if (isParamSetByUser("block") && _blocks.empty())
    paramError("block",
               "Empty block restriction is not supported. Comment out the Physics if you are "
               "trying to disable it.");

  // Placeholder before work with components
  if (_blocks.empty())
    _blocks.push_back("ANY_BLOCK_ID");
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
