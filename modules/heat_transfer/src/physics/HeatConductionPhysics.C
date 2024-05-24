//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "HeatConductionPhysics.h"

InputParameters
HeatConductionPhysics::validParams()
{
  InputParameters params = PhysicsBase::validParams();
  params.addClassDescription("Add the heat conduction physics");

  params.addParam<VariableName>("temperature_name", "T", "Variable name for the temperature");
  params.addParam<VariableName>("heat_source_var", "Variable providing the heat source");
  params.addParam<std::vector<SubdomainName>>("heat_source_blocks",
                                              "Block restriction of the heat source");
  params.addParam<MooseFunctorName>("heat_source_functor", "Functor providing the heat source");

  params.addParam<FunctionName>(
      "initial_temperature", 300, "Initial value of the temperature variable");

  // Boundary conditions
  params.addParam<std::vector<BoundaryName>>(
      "heat_flux_boundaries", {}, "Boundaries on which to apply a heat flux");
  params.addParam<std::vector<MooseFunctorName>>(
      "boundary_heat_fluxes",
      {},
      "Functors to compute the heat flux on each boundary in 'heat_flux_boundaries'");
  params.addParam<std::vector<BoundaryName>>(
      "insulated_boundaries", {}, "Boundaries on which to apply a zero heat flux");
  params.addParam<std::vector<BoundaryName>>(
      "fixed_temperature_boundaries", {}, "Boundaries on which to apply a fixed temperature");
  params.addParam<std::vector<MooseFunctorName>>(
      "boundary_temperatures",
      {},
      "Functors to compute the heat flux on each boundary in 'fixed_temperature_boundaries'");
  params.addParamNamesToGroup(
      "heat_flux_boundaries insulated_boundaries fixed_temperature_boundaries boundary_heat_fluxes "
      "boundary_temperatures",
      "Thermal boundaries");

  // Preconditioning is implemented so let's use it by default
  MooseEnum pc_options("default none", "default");
  params.addParam<MooseEnum>(
      "preconditioning", pc_options, "Which preconditioning to use for this Physics");

  return params;
}

HeatConductionPhysics::HeatConductionPhysics(const InputParameters & parameters)
  : PhysicsBase(parameters), _temperature_name(getParam<VariableName>("temperature_name"))
{
  // Parameter checking
  checkVectorParamsSameLength<BoundaryName, MooseFunctorName>("heat_flux_boundaries",
                                                              "boundary_heat_fluxes");
  checkVectorParamsSameLength<BoundaryName, MooseFunctorName>("fixed_temperature_boundaries",
                                                              "boundary_temperatures");
  checkVectorParamsNoOverlap<BoundaryName>(
      {"heat_flux_boundaries", "insulated_boundaries", "fixed_temperature_boundaries"});

  addRequiredPhysicsTask("add_preconditioning");
}

void
HeatConductionPhysics::addInitialConditions()
{
  // Always obey the user, but dont set a hidden default when restarting
  if (!_app.isRestarting() || parameters().isParamSetByUser("initial_temperature"))
  {
    InputParameters params = getFactory().getValidParams("FunctionIC");
    params.set<VariableName>("variable") = _temperature_name;
    params.set<FunctionName>("function") = getParam<FunctionName>("initial_temperature");
    getProblem().addInitialCondition("FunctionIC", _temperature_name + "_ic", params);
  }
}

void
HeatConductionPhysics::addPreconditioning()
{
  // Use a multigrid method, known to work for elliptic problems such as diffusion
  if (_preconditioning == "default")
  {
    // We only pass petsc options as that's all that's needed to set up the preconditioner
    auto & po = _problem->getPetscOptions();
    const auto option_pair1 =
        std::make_pair<MooseEnumItem, std::string>(MooseEnumItem("-pc_type"), "hypre");
    const auto option_pair2 =
        std::make_pair<MooseEnumItem, std::string>(MooseEnumItem("-pc_hypre_type"), "boomeramg");
    processPetscPairs({option_pair1, option_pair2}, _problem->mesh().dimension(), po);
  }
}
