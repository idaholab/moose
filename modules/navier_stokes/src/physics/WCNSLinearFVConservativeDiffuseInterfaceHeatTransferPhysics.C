//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "WCNSLinearFVConservativeDiffuseInterfaceHeatTransferPhysics.h"
#include "WCNSLinearFVConservativeDiffuseInterfaceVOFPhysics.h"
#include "LinearFVConservativeDiffuseInterfaceEnthalpyDiffusion.h"
#include "WCNSFVFlowPhysicsBase.h"
#include "NS.h"

registerNavierStokesPhysicsBaseTasks("NavierStokesApp",
                                     WCNSLinearFVConservativeDiffuseInterfaceHeatTransferPhysics);
registerMooseAction("NavierStokesApp",
                    WCNSLinearFVConservativeDiffuseInterfaceHeatTransferPhysics,
                    "add_variables_physics");
registerMooseAction("NavierStokesApp",
                    WCNSLinearFVConservativeDiffuseInterfaceHeatTransferPhysics,
                    "add_aux_variable");
registerMooseAction("NavierStokesApp",
                    WCNSLinearFVConservativeDiffuseInterfaceHeatTransferPhysics,
                    "add_ics_physics");
registerMooseAction("NavierStokesApp",
                    WCNSLinearFVConservativeDiffuseInterfaceHeatTransferPhysics,
                    "add_user_object");
registerMooseAction("NavierStokesApp",
                    WCNSLinearFVConservativeDiffuseInterfaceHeatTransferPhysics,
                    "add_material");
registerMooseAction("NavierStokesApp",
                    WCNSLinearFVConservativeDiffuseInterfaceHeatTransferPhysics,
                    "add_fv_kernel");
registerMooseAction("NavierStokesApp",
                    WCNSLinearFVConservativeDiffuseInterfaceHeatTransferPhysics,
                    "add_fv_bc");

InputParameters
WCNSLinearFVConservativeDiffuseInterfaceHeatTransferPhysics::validParams()
{
  InputParameters params = NavierStokesPhysicsBase::validParams();
  params += WCNSFVCoupledAdvectionPhysicsHelper::validParams();
  params.addClassDescription(
      "Create the energy equation of a two-phase diffuse-interface flow, solved for the conserved "
      "mixture enthalpy density with phase enthalpies from fluid properties.");
  params.set<std::vector<SolverSystemName>>("system_names") = {"energy_system"};

  params.addRequiredParam<PhysicsName>(
      "coupled_vof_physics",
      "The ConservativeDiffuseInterfaceVOFSegregated physics transporting the volume fraction.");
  params.addParam<VariableName>(
      "conserved_enthalpy_variable", "rho_h", "Name of the solved mixture enthalpy density rho*h.");
  params.addParam<VariableName>("temperature_variable",
                                NS::T_fluid,
                                "Name of the temperature auxiliary variable recovered from rho*h.");
  params.addRequiredParam<UserObjectName>("liquid_fluid_properties",
                                          "Fluid properties giving the liquid enthalpy, heat "
                                          "capacity and thermal conductivity.");
  params.addRequiredParam<UserObjectName>(
      "gas_fluid_properties",
      "Fluid properties giving the gas enthalpy, heat capacity and thermal conductivity.");
  // One standard atmosphere
  params.addParam<Real>("thermodynamic_pressure",
                        101325,
                        "Pressure at which the phase properties are evaluated [Pa].");
  params.addParam<FunctionName>("initial_temperature", "300", "Initial temperature [K].");

  params.addParam<Real>("liquid_thermal_expansion",
                        0,
                        "Volumetric thermal expansion coefficient of the liquid in the "
                        "Boussinesq buoyancy density [1/K].");
  params.addParam<Real>("gas_thermal_expansion",
                        0,
                        "Volumetric thermal expansion coefficient of the gas in the Boussinesq "
                        "buoyancy density [1/K].");
  params.addParam<Real>(
      "reference_temperature",
      "Temperature at which the buoyancy density equals the phase densities [K].");
  params.addParam<MooseFunctorName>(
      "buoyancy_density_name",
      "rho_buoyancy",
      "Name of the generated Boussinesq density, sum_k alpha_k rho_k (1 - beta_k (T - T_ref)), "
      "to pass as the coupled flow physics' buoyancy_density.");

  params.addParam<MultiMooseEnum>("energy_inlet_types",
                                  MultiMooseEnum("fixed-temperature"),
                                  "Type of the energy condition at each inlet of the flow.");
  params.addParam<std::vector<MooseFunctorName>>(
      "energy_inlet_functors", {}, "The temperature at each inlet of the flow.");
  params.addParam<MultiMooseEnum>("energy_wall_types",
                                  MultiMooseEnum("fixed-temperature heatflux"),
                                  "Type of the energy condition at each wall of the flow.");
  params.addParam<std::vector<MooseFunctorName>>(
      "energy_wall_functors",
      {},
      "The temperature or the heat flux into the domain at each wall of the flow.");

  params.transferParam<MooseEnum>(
      LinearFVConservativeDiffuseInterfaceEnthalpyDiffusion::validParams(),
      "conductivity_interpolation");

  params.addParamNamesToGroup("coupled_vof_physics", "Coupled Physics");
  params.addParamNamesToGroup("liquid_fluid_properties gas_fluid_properties thermodynamic_pressure",
                              "Material properties");
  params.addParamNamesToGroup("liquid_thermal_expansion gas_thermal_expansion "
                              "reference_temperature buoyancy_density_name",
                              "Buoyancy");
  params.addParamNamesToGroup("energy_inlet_types energy_inlet_functors",
                              "Inlet boundary conditions");
  params.addParamNamesToGroup("energy_wall_types energy_wall_functors", "Wall boundary conditions");
  params.addParamNamesToGroup("conductivity_interpolation", "Numerical scheme");

  params.suppressParameter<MooseEnum>("preconditioning");
  return params;
}

WCNSLinearFVConservativeDiffuseInterfaceHeatTransferPhysics::
    WCNSLinearFVConservativeDiffuseInterfaceHeatTransferPhysics(const InputParameters & parameters)
  : NavierStokesPhysicsBase(parameters),
    WCNSFVCoupledAdvectionPhysicsHelper(this),
    _vof_physics(getCoupledPhysics<const WCNSLinearFVConservativeDiffuseInterfaceVOFPhysics>(
        getParam<PhysicsName>("coupled_vof_physics"))),
    _enthalpy_name(getParam<VariableName>("conserved_enthalpy_variable")),
    _temperature_name(getParam<VariableName>("temperature_variable")),
    _alpha_name(
        _vof_physics->getParam<std::vector<NonlinearVariableName>>("passive_scalar_names")[0]),
    _mixture_enthalpy_name(prefix() + "mixture_enthalpy")
{
  saveSolverVariableName(_enthalpy_name);

  if (!isParamSetByUser("block"))
    _blocks = _flow_equations_physics->blocks();

  checkVectorParamAndMultiMooseEnumLength<MooseFunctorName>("energy_inlet_functors",
                                                            "energy_inlet_types");
  checkVectorParamAndMultiMooseEnumLength<MooseFunctorName>("energy_wall_functors",
                                                            "energy_wall_types");
  if ((getParam<Real>("liquid_thermal_expansion") != 0 ||
       getParam<Real>("gas_thermal_expansion") != 0) &&
      !isParamValid("reference_temperature"))
    paramError("reference_temperature",
               "A reference temperature is required for a temperature-dependent buoyancy density.");

  addRequiredPhysicsTask("add_variables_physics");
  addRequiredPhysicsTask("add_ics_physics");
  addRequiredPhysicsTask("add_fv_kernel");
  addRequiredPhysicsTask("add_fv_bc");
}

unsigned short
WCNSLinearFVConservativeDiffuseInterfaceHeatTransferPhysics::
    getNumberAlgebraicGhostingLayersNeeded() const
{
  return _flow_equations_physics->getNumberAlgebraicGhostingLayersNeeded();
}

void
WCNSLinearFVConservativeDiffuseInterfaceHeatTransferPhysics::addSolverVariables()
{
  const auto var_type = "MooseLinearVariableFVReal";
  auto params = getFactory().getValidParams(var_type);
  assignBlocks(params, _blocks);
  params.set<SolverSystemName>("solver_sys") = getSolverSystem(_enthalpy_name);
  getProblem().addVariable(var_type, _enthalpy_name, params);
}

void
WCNSLinearFVConservativeDiffuseInterfaceHeatTransferPhysics::addAuxiliaryVariables()
{
  const auto var_type = "MooseLinearVariableFVReal";
  auto params = getFactory().getValidParams(var_type);
  assignBlocks(params, _blocks);
  getProblem().addAuxVariable(var_type, _temperature_name, params);
}

void
WCNSLinearFVConservativeDiffuseInterfaceHeatTransferPhysics::addInitialConditions()
{
  // rho*h is set from the initial temperature and volume fraction by the mixture enthalpy object
  auto params = getFactory().getValidParams("FVFunctionIC");
  assignBlocks(params, _blocks);
  params.set<VariableName>("variable") = _temperature_name;
  params.set<FunctionName>("function") = getParam<FunctionName>("initial_temperature");
  getProblem().addFVInitialCondition("FVFunctionIC", prefix() + _temperature_name + "_ic", params);
}

void
WCNSLinearFVConservativeDiffuseInterfaceHeatTransferPhysics::addMaterials()
{
  // Boussinesq: the phase densities vary with temperature in the gravity force only
  const auto & liquid_density = _vof_physics->getParam<MooseFunctorName>("liquid_density_name");
  const auto & gas_density = _vof_physics->getParam<MooseFunctorName>("gas_density_name");
  const auto beta_l = Moose::stringifyExact(getParam<Real>("liquid_thermal_expansion"));
  const auto beta_g = Moose::stringifyExact(getParam<Real>("gas_thermal_expansion"));
  const auto dT = "(" + _temperature_name + " - " +
                  (isParamValid("reference_temperature")
                       ? Moose::stringifyExact(getParam<Real>("reference_temperature"))
                       : std::string("0")) +
                  ")";

  auto params = getFactory().getValidParams("ParsedFunctorMaterial");
  assignBlocks(params, _blocks);
  params.set<std::string>("expression") = _alpha_name + " * " + liquid_density + " * (1 - " +
                                          beta_l + " * " + dT + ") + (1 - " + _alpha_name + ") * " +
                                          gas_density + " * (1 - " + beta_g + " * " + dT + ")";
  std::vector<std::string> functor_names = {_alpha_name, _temperature_name, liquid_density};
  // The phases may share a density functor, and the parser takes each name once
  if (gas_density != liquid_density)
    functor_names.push_back(gas_density);
  params.set<std::vector<std::string>>("functor_names") = functor_names;
  params.set<std::string>("property_name") = getParam<MooseFunctorName>("buoyancy_density_name");
  getProblem().addMaterial("ParsedFunctorMaterial", prefix() + "buoyancy_density", params);
}

void
WCNSLinearFVConservativeDiffuseInterfaceHeatTransferPhysics::addUserObjects()
{
  const std::string object_type = "ConservativeDiffuseInterfaceMixtureEnthalpy";
  auto params = getFactory().getValidParams(object_type);
  assignBlocks(params, _blocks);
  params.set<VariableName>("enthalpy_variable") = _enthalpy_name;
  params.set<VariableName>("temperature") = _temperature_name;
  params.set<MooseFunctorName>("volume_fraction") = _alpha_name;
  params.set<MooseFunctorName>("liquid_density") =
      _vof_physics->getParam<MooseFunctorName>("liquid_density_name");
  params.set<MooseFunctorName>("gas_density") =
      _vof_physics->getParam<MooseFunctorName>("gas_density_name");
  params.set<UserObjectName>("liquid_fluid_properties") =
      getParam<UserObjectName>("liquid_fluid_properties");
  params.set<UserObjectName>("gas_fluid_properties") =
      getParam<UserObjectName>("gas_fluid_properties");
  params.set<Real>("thermodynamic_pressure") = getParam<Real>("thermodynamic_pressure");
  getProblem().addUserObject(object_type, _mixture_enthalpy_name, params);
}

void
WCNSLinearFVConservativeDiffuseInterfaceHeatTransferPhysics::addFVKernels()
{
  {
    const std::string kernel_type = "LinearFVTimeDerivative";
    auto params = getFactory().getValidParams(kernel_type);
    assignBlocks(params, _blocks);
    params.set<LinearVariableName>("variable") = _enthalpy_name;
    getProblem().addLinearFVKernel(kernel_type, prefix() + "rho_h_time", params);
  }
  {
    const std::string kernel_type = "LinearFVConservativeDiffuseInterfaceEnthalpyAdvection";
    auto params = getFactory().getValidParams(kernel_type);
    assignBlocks(params, _blocks);
    params.set<LinearVariableName>("variable") = _enthalpy_name;
    params.set<UserObjectName>("mixture_enthalpy") = _mixture_enthalpy_name;
    getProblem().addLinearFVKernel(kernel_type, prefix() + "rho_h_advection", params);
  }
  {
    const std::string kernel_type = "LinearFVConservativeDiffuseInterfaceEnthalpyDiffusion";
    auto params = getFactory().getValidParams(kernel_type);
    assignBlocks(params, _blocks);
    params.set<LinearVariableName>("variable") = _enthalpy_name;
    params.set<UserObjectName>("mixture_enthalpy") = _mixture_enthalpy_name;
    params.set<MooseEnum>("conductivity_interpolation") =
        getParam<MooseEnum>("conductivity_interpolation");
    getProblem().addLinearFVKernel(kernel_type, prefix() + "rho_h_diffusion", params);
  }
}

void
WCNSLinearFVConservativeDiffuseInterfaceHeatTransferPhysics::addEnthalpyBC(
    const BoundaryName & boundary, const std::string & parameter, const MooseFunctorName & functor)
{
  const std::string bc_type = "LinearFVConservativeDiffuseInterfaceEnthalpyBC";
  auto params = getFactory().getValidParams(bc_type);
  params.set<LinearVariableName>("variable") = _enthalpy_name;
  params.set<std::vector<BoundaryName>>("boundary") = {boundary};
  params.set<UserObjectName>("mixture_enthalpy") = _mixture_enthalpy_name;
  params.set<MooseFunctorName>("volume_fraction") = _alpha_name;
  params.set<MooseFunctorName>(parameter) = functor;
  getProblem().addLinearFVBC(bc_type, prefix() + _enthalpy_name + "_" + boundary, params);
}

void
WCNSLinearFVConservativeDiffuseInterfaceHeatTransferPhysics::addFVBCs()
{
  const auto & inlet_types = getParam<MultiMooseEnum>("energy_inlet_types");
  const auto & inlet_functors = getParam<std::vector<MooseFunctorName>>("energy_inlet_functors");
  const auto & inlet_boundaries = _flow_equations_physics->getInletBoundaries();
  if (inlet_boundaries.size() != inlet_types.size())
    paramError("energy_inlet_types",
               "There should be one energy inlet type for each of the ",
               inlet_boundaries.size(),
               " inlet boundaries of the flow.");
  for (const auto i : index_range(inlet_boundaries))
    addEnthalpyBC(inlet_boundaries[i], "temperature", inlet_functors[i]);

  const auto & wall_types = getParam<MultiMooseEnum>("energy_wall_types");
  const auto & wall_functors = getParam<std::vector<MooseFunctorName>>("energy_wall_functors");
  const auto & wall_boundaries = _flow_equations_physics->getWallBoundaries();
  if (wall_boundaries.size() != wall_types.size())
    paramError("energy_wall_types",
               "There should be one energy wall type for each of the ",
               wall_boundaries.size(),
               " wall boundaries of the flow.");
  for (const auto i : index_range(wall_boundaries))
    addEnthalpyBC(wall_boundaries[i],
                  wall_types[i] == "fixed-temperature" ? "temperature" : "heat_flux",
                  wall_functors[i]);

  // Fluid leaving or re-entering through an outlet carries the temperature of its cell
  for (const auto & outlet_boundary : _flow_equations_physics->getOutletBoundaries())
    addEnthalpyBC(outlet_boundary, "heat_flux", "0");
}
