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
#include "INSFVRhieChowInterpolator.h"
#include "RelationshipManager.h"
#include "WCNSFVFlowPhysics.h"

InputParameters
WCNSFVPhysicsBase::validParams()
{
  InputParameters params = NavierStokesPhysicsBase::validParams();
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

WCNSFVPhysicsBase::WCNSFVPhysicsBase(const InputParameters & parameters)
  : NavierStokesPhysicsBase(parameters),
    _define_variables(getParam<bool>("define_variables")),
    _flow_equations_physics(nullptr)
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

  // Search for a physics defining the flow equations
  findCoupledFlowPhysics();
}

void
WCNSFVPhysicsBase::initializePhysicsAdditional()
{
  getProblem().needFV();
}

void
WCNSFVPhysicsBase::findCoupledFlowPhysics()
{
  // User passed it, just use that
  if (isParamValid("coupled_flow_physics"))
    _flow_equations_physics =
        getCoupledPhysics<WCNSFVFlowPhysics>(getParam<PhysicsName>("coupled_flow_physics"));
  // Look for any physics of the right type, and check the block restriction
  else
  {
    // We probably dont want this routine to be used for WCNSFVFlowPhysics
    // Maybe WCNSFVPhysicsBase should WCNSFVCoupledAdvectionPhysicsBase
    if (this->type() == "WCNSFVFlowPhysics")
      return;
    const auto all_flow_physics = getCoupledPhysics<const WCNSFVFlowPhysics>();
    for (const auto physics : all_flow_physics)
      if (checkBlockRestrictionIdentical(
              physics->name(), physics->blocks(), /*error_if_not_identical=*/false))
      {
        // We could check even further, and check that the block restrictions of WCNSFVFlowPhysics
        // do not overlap.
        if (_flow_equations_physics)
          mooseError("Only one WCNSFV flow physics may be defined for a given block restriction");
        _flow_equations_physics = physics;
      }
  }
}

InputParameters
WCNSFVPhysicsBase::getAdditionalRMParams() const
{
  unsigned short necessary_layers = getParam<unsigned short>("ghost_layers");
  necessary_layers = std::max(necessary_layers, getNumberAlgebraicGhostingLayersNeeded());
  if (_porous_medium_treatment && isParamValid("porosity_smoothing_layers"))
    necessary_layers =
        std::max(getParam<unsigned short>("porosity_smoothing_layers"), necessary_layers);

  // Just an object that has a ghost_layers parameter
  const std::string kernel_type = "INSFVMixingLengthReynoldsStress";
  InputParameters params = getFactory().getValidParams(kernel_type);
  params.template set<unsigned short>("ghost_layers") = necessary_layers;

  return params;
}
