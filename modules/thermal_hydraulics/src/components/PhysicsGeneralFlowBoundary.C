//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PhysicsGeneralFlowBoundary.h"
#include "ThermalHydraulicsFlowPhysics.h"
#include "MapConversionUtils.h"

registerMooseObject("ThermalHydraulicsApp", PhysicsGeneralFlowBoundary);

InputParameters
PhysicsGeneralFlowBoundary::validParams()
{
  InputParameters params = PhysicsFlowBoundary::validParams();

  // Specified variable values
  params.addParam<std::vector<VariableName>>(
      "fixed_values_variables", {}, "Variables to set the boundary value of");
  params.addParam<std::vector<MooseFunctorName>>(
      "fixed_values_functors", {}, "Functors to set the variables with");

  // Specified fluxes
  params.addParam<std::vector<VariableName>>(
      "fixed_equation_flux_variables",
      {},
      "Variable of the equation for which to specify a boundary flux");
  params.addParam<std::vector<MooseFunctorName>>(
      "fixed_equation_flux_functors", {}, "Functor computing the boundary flux for the equations");

  // TODO: Add proper support for NS and this
  params.addParam<bool>("reversible", true, "True for reversible, false for pure inlet");
  params.addClassDescription("Boundary condition with user-defined flux and variable values.");
  return params;
}

PhysicsGeneralFlowBoundary::PhysicsGeneralFlowBoundary(const InputParameters & params)
  : PhysicsFlowBoundary(params),
    _specified_values_variables(getParam<std::vector<VariableName>>("fixed_values_variables")),
    _specified_values_functors(getParam<std::vector<MooseFunctorName>>("fixed_values_functors")),
    _specified_fluxes_variables(
        getParam<std::vector<VariableName>>("fixed_equation_flux_variables")),
    _specified_fluxes_functors(
        getParam<std::vector<MooseFunctorName>>("fixed_equation_flux_functors")),
    _reversible(getParam<bool>("reversible"))
{
  if (_specified_values_variables.size() != _specified_values_functors.size())
    paramError("fixed_values_variables", "Should be the same size as 'fixed_values_functors'");
  if (_specified_fluxes_variables.size() != _specified_fluxes_functors.size())
    paramError("fixed_equation_flux_variables",
               "Should be the same size as 'fixed_equation_flux_functors'");

  _specified_values = Moose::createMapFromVectors<VariableName, MooseFunctorName>(
      _specified_values_variables, _specified_values_functors);
  _specified_fluxes = Moose::createMapFromVectors<VariableName, MooseFunctorName>(
      _specified_fluxes_variables, _specified_fluxes_functors);
}

void
PhysicsGeneralFlowBoundary::check() const
{
  PhysicsFlowBoundary::check();
}

void
PhysicsGeneralFlowBoundary::init()
{
  PhysicsFlowBoundary::init();

  // For now, we do this here. We could consider doing it elsewhere
  for (auto th_phys : _th_physics)
    th_phys->setInlet(name(), ThermalHydraulicsFlowPhysics::GeneralBoundary);
}
