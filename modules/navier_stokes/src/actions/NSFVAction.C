//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// Navier-Stokes includes
#include "NSFVAction.h"

#include "NS.h"
#include "AddVariableAction.h"
#include "MooseObject.h"
#include "INSADObjectTracker.h"
#include "NonlinearSystemBase.h"
#include "RelationshipManager.h"

// MOOSE includes
#include "FEProblem.h"

#include "libmesh/fe.h"
#include "libmesh/vector_value.h"
#include "libmesh/string_to_enum.h"

registerMooseAction("NavierStokesApp", NSFVAction, "add_navier_stokes_variables");
registerMooseAction("NavierStokesApp", NSFVAction, "add_navier_stokes_user_objects");
registerMooseAction("NavierStokesApp", NSFVAction, "add_navier_stokes_ics");
registerMooseAction("NavierStokesApp", NSFVAction, "add_navier_stokes_kernels");
registerMooseAction("NavierStokesApp", NSFVAction, "add_navier_stokes_bcs");
registerMooseAction("NavierStokesApp", NSFVAction, "add_material");

InputParameters
NSFVAction::validParams()
{
  InputParameters params = Action::validParams();
  params.addClassDescription("This class allows us to set up Navier-Stokes equations for porous "
                             "medium or clean fluid flows using finite volume discretization.");

  // General simulation control parameters
  MooseEnum sim_type("steady-state transient", "steady-state");
  params.addParam<MooseEnum>("simulation_type", sim_type, "Navier-Stokes equation type");

  MooseEnum comp_type("incompressible weakly-compressible compressible", "incompressible");
  params.addParam<MooseEnum>(
      "compressibility", comp_type, "Compressibility constraint for the Navier-Stokes equations.");

  params.addParam<bool>(
      "porous_medium_treatment", false, "Whether to use porous medium solvers or not.");

  params.addParam<MooseFunctorName>(
      "porosity", NS::porosity, "The name of the auxiliary variable for the porosity field.");

  MooseEnum turbulence_type("mixing-length none", "none");
  params.addParam<MooseEnum>(
      "turbulence_handling",
      turbulence_type,
      "The way additional diffusivities are determined in the turbulent regime.");

  params.addParam<std::vector<SubdomainName>>(
      "block", "The list of block ids (SubdomainID) on which NS equation is defined on");

  // Parameters used for defining boundaries

  params.addParam<std::vector<BoundaryName>>(
      "inlet_boundaries", std::vector<BoundaryName>(), "Names of inlet boundaries");
  params.addParam<std::vector<BoundaryName>>(
      "outlet_boundaries", std::vector<BoundaryName>(), "Names of outlet boundaries");
  params.addParam<std::vector<BoundaryName>>(
      "wall_boundaries", std::vector<BoundaryName>(), "Names of wall boundaries");

  // Momentum and Mass equation related parameters

  params.addParam<RealVectorValue>("initial_velocity",
                                   RealVectorValue(1e-15, 1e-15, 1e-15),
                                   "The initial velocity, assumed constant everywhere");
  params.addParam<RealVectorValue>("initial_momentum",
                                   RealVectorValue(1e-15, 1e-15, 1e-15),
                                   "The initial momentum, assumed constant everywhere");

  params.addParam<MaterialPropertyName>(
      "dynamic_viscosity", NS::mu, "The name of the dynamic viscosity");
  params.addParam<MaterialPropertyName>("density", NS::density, "The name of the density");

  params.addParam<bool>(
      "has_coupled_force",
      false,
      "Whether the simulation has a force due to a coupled vector variable/vector function");
  params.addCoupledVar("coupled_force_var", "The variable(s) providing the coupled force(s)");
  params.addParam<std::vector<FunctionName>>("coupled_force_vector_function",
                                             "The function(s) standing in as a coupled force");

  params.addParam<RealVectorValue>(
      "gravity", RealVectorValue(0, 0, 0), "The gravitational acceleration vector.");

  MultiMooseEnum mom_inlet_type("fixed-velocity fixed-massflow", "fixed-velocity");
  params.addParam<MultiMooseEnum>("momentum_inlet_types",
                                  mom_inlet_type,
                                  "Types of inlet boundaries for them omentum equation");

  params.addParam<std::vector<FunctionName>>("momentum_inlet_function",
                                             std::vector<FunctionName>(),
                                             "Functions for inlet boundary velocities");

  MultiMooseEnum mom_outlet_type("fixed-pressure zero-gradient fixed-pressure-zero-gradient",
                                 "fixed-pressure");
  params.addParam<MultiMooseEnum>("momentum_outlet_types",
                                  mom_outlet_type,
                                  "Types of outlet boundaries for them momentum equation");
  params.addParam<std::vector<FunctionName>>("pressure_function",
                                             std::vector<FunctionName>(),
                                             "Functions for boundary pressures at outlets.");

  MultiMooseEnum mom_wall_type("symmetry noslip slip wallfunction", "noslip");
  params.addParam<MultiMooseEnum>(
      "momentum_wall_types", mom_wall_type, "Types of wall boundaries for them momentum equation");

  params.addParam<bool>(
      "pin_pressure", false, "Switch to enable pressure shifting for incompressible simulations.");

  MooseEnum s_type("average point-value", "average");
  params.addParam<MooseEnum>(
      "pinned_pressure_type",
      s_type,
      "Types for shifting (pinning) the pressure in case of incompressible simulations.");

  params.addParam<Point>(
      "pinned_pressure_point",
      Point(),
      "The XYZ coordinates where pressure needs to be pinned for incompressible simulations.");

  params.addParam<Real>("pinned_pressure_value",
                        0.0,
                        "The value used for pinning the pressure (point value/average).");

  params.addParam<Real>("initial_pressure", 0, "The initial pressure, assumed constant everywhere");

  // Energy equation related parameters

  params.addParam<bool>("boussinesq_approximation", false, "True to have Boussinesq approximation");
  params.addParam<Real>("ref_temperature",
                        273.15,
                        "Value for reference temperature in case of Boussinesq approximation");
  params.addParam<MaterialPropertyName>(
      "thermal_expansion", NS::alpha, "The name of the thermal expansion");
  params.addParam<bool>("add_energy_equation", false, "True to add energy equation");

  params.addParam<VariableName>("solid_temperature_variable",
                                NS::T_solid,
                                "Solid subscale structure temperature variable name");
  params.addParam<Real>(
      "initial_temperature", 0, "The initial temperature, assumed constant everywhere");
  params.addParam<MaterialPropertyName>(
      "thermal_conductivity", NS::k, "The name of the thermal conductivity");
  params.addParam<MaterialPropertyName>("specific_heat", NS::cp, "The name of the specific heat");

  params.addParam<bool>(
      "has_heat_source", false, "Whether there is a heat source function object in the simulation");
  params.addParam<FunctionName>("heat_source_function", "The function describing the heat source");
  params.addCoupledVar("heat_source_var", "The coupled variable describing the heat source");

  MultiMooseEnum en_inlet_type("fixed-temperature fixed-enthalpy fixed-totalenergy heatflux",
                               "fixed-temperature");
  params.addParam<MultiMooseEnum>("energy_inlet_types",
                                  en_inlet_type,
                                  "Types for the inlet boundaries for the energy equation.");

  params.addParam<std::vector<FunctionName>>(
      "energy_inlet_function",
      std::vector<FunctionName>(),
      "Functions for fixed-value boundaries in the energy equation.");

  MultiMooseEnum en_wall_type("fixed-temperature heatflux symmetry wallfunction", "symmetry");
  params.addParam<MultiMooseEnum>(
      "energy_wall_types", en_wall_type, "Types for the wall boundaries for the energy equation.");

  params.addParam<std::vector<FunctionName>>(
      "energy_wall_function",
      std::vector<FunctionName>(),
      "Functions for Dirichlet/Neumann boundaries in the energy equation.");

  params.addParam<std::vector<SubdomainName>>(
      "ambient_convection_blocks",
      std::vector<SubdomainName>(),
      "The blocks where the ambient convection is present.");

  params.addParam<std::vector<MaterialPropertyName>>(
      "ambient_convection_alpha",
      std::vector<MaterialPropertyName>(),
      "The heat exchange coefficients for each block in 'ambient_convection_blocks'.");

  params.addParam<std::vector<MooseFunctorName>>(
      "ambient_temperature",
      std::vector<MooseFunctorName>(),
      "The ambient temperature for each block in 'ambient_convection_blocks'.");

  // Friction term-related

  params.addParam<std::vector<SubdomainName>>(
      "friction_blocks",
      std::vector<SubdomainName>(),
      "The blocks where the friciton factors are applied to emulate flow resistances.");

  params.addParam<std::vector<std::vector<std::string>>>(
      "friction_types",
      std::vector<std::vector<std::string>>(),
      "The types of friction forces for every block in 'friction_blocks'.");

  params.addParam<std::vector<std::vector<std::string>>>(
      "friction_coeffs",
      std::vector<std::vector<std::string>>(),
      "The firction coefficients for every item in 'friction_types'.");

  // Control of numerical schemes
  params.addParam<std::string>("momentum_advection_interpolation",
                               "average",
                               "The numerical scheme to use for interpolating momentum/velocity, "
                               "as an advected quantity, to the face.");
  params.addParam<std::string>("energy_advection_interpolation",
                               "average",
                               "The numerical scheme to use for interpolating energy/temperature, "
                               "as an advected quantity, to the face.");
  params.addParam<std::string>("mass_advection_interpolation",
                               "average",
                               "The numerical scheme to use for interpolating density, "
                               "as an advected quantity, to the face.");

  params.addParam<Real>("momentum_scaling", 1.0, "The scaling factor for the momentum variables.");
  params.addParam<Real>("energy_scaling", 1.0, "The scaling factor for the energy variables.");
  params.addParam<Real>("mass_scaling",
                        1.0,
                        "The scaling factor for the mass variables (for incompressible simulation "
                        "this is pressure scaling).");

  // Turbulence-related parameters
  params.addParam<std::vector<BoundaryName>>(
      "mixing_length_walls",
      std::vector<BoundaryName>(),
      "Walls where the mixing length model should be utilized.");

  ExecFlagEnum exec_enum = MooseUtils::getDefaultExecFlagEnum();
  params.addParam<ExecFlagEnum>("mixing_length_aux_execute_on",
                                exec_enum,
                                "When the mixing length aux kernels should be executed.");

  params.addParam<Real>(
      "von_karman_const", 0.41, "Von Karman parameter for the mixing length model");
  params.addParam<Real>("von_karman_const_0", 0.09, "Escudier' model parameter");
  params.addParam<Real>(
      "mixing_length_delta",
      1e9,
      "Tunable parameter related to the thickness of the boundary layer."
      "When it is not specified, Prandtl's original mixing length model is retrieved.");

  // Create input parameter groups

  params.addParamNamesToGroup(
      "simulation_type compressibility porous_medium_treatment turbulence_handling", "Base");
  params.addParamNamesToGroup(
      "dynamic_viscosity density thermal_expansion thermal_conductivity specific_heat",
      "Materials");
  params.addParamNamesToGroup(
      "inlet_boundaries momentum_inlet_types momentum_inlet_function energy_inlet_types "
      "energy_inlet_function wall_boundaries momentum_wall_types energy_wall_types "
      "energy_wall_function outlet_boundaries momentum_outlet_types pressure_function",
      "BoundaryCondition");
  params.addParamNamesToGroup("initial_pressure initial_velocity initial_temperature", "Variable");

  return params;
}

NSFVAction::NSFVAction(InputParameters parameters)
  : Action(parameters),
    _type(getParam<MooseEnum>("simulation_type")),
    _compressibility(getParam<MooseEnum>("compressibility")),
    _porous_medium_treatment(getParam<bool>("porous_medium_treatment")),
    _has_energy_equation(getParam<bool>("add_energy_equation")),
    _boussinesq_approximation(getParam<bool>("boussinesq_approximation")),
    _porosity_name(getParam<MooseFunctorName>("porosity")),
    _turbulence_handling(getParam<MooseEnum>("turbulence_handling")),
    _blocks(getParam<std::vector<SubdomainName>>("block")),
    _inlet_boundaries(getParam<std::vector<BoundaryName>>("inlet_boundaries")),
    _outlet_boundaries(getParam<std::vector<BoundaryName>>("outlet_boundaries")),
    _wall_boundaries(getParam<std::vector<BoundaryName>>("wall_boundaries")),
    _momentum_inlet_types(getParam<MultiMooseEnum>("momentum_inlet_types")),
    _momentum_inlet_function(getParam<std::vector<FunctionName>>("momentum_inlet_function")),
    _momentum_outlet_types(getParam<MultiMooseEnum>("momentum_outlet_types")),
    _momentum_wall_types(getParam<MultiMooseEnum>("momentum_wall_types")),
    _energy_inlet_types(getParam<MultiMooseEnum>("energy_inlet_types")),
    _energy_inlet_function(getParam<std::vector<FunctionName>>("energy_inlet_function")),
    _energy_wall_types(getParam<MultiMooseEnum>("energy_wall_types")),
    _energy_wall_function(getParam<std::vector<FunctionName>>("energy_wall_function")),
    _pressure_function(getParam<std::vector<FunctionName>>("pressure_function")),
    _ambient_convection_blocks(getParam<std::vector<SubdomainName>>("ambient_convection_blocks")),
    _ambient_convection_alpha(
        getParam<std::vector<MaterialPropertyName>>("ambient_convection_alpha")),
    _ambient_temperature(getParam<std::vector<MooseFunctorName>>("ambient_temperature")),
    _friction_blocks(getParam<std::vector<SubdomainName>>("friction_blocks")),
    _friction_types(getParam<std::vector<std::vector<std::string>>>("friction_types")),
    _friction_coeffs(getParam<std::vector<std::vector<std::string>>>("friction_coeffs")),
    _solid_temperature_variable_name(getParam<VariableName>("solid_temperature_variable")),
    _density_name(getParam<MaterialPropertyName>("density")),
    _dynamic_viscosity_name(getParam<MaterialPropertyName>("dynamic_viscosity")),
    _specific_heat_name(getParam<MaterialPropertyName>("specific_heat")),
    _thermal_conductivity_name(getParam<MaterialPropertyName>("thermal_conductivity")),
    _thermal_expansion_name(getParam<MaterialPropertyName>("thermal_expansion")),
    _momentum_advection_interpolation(getParam<std::string>("momentum_advection_interpolation")),
    _energy_advection_interpolation(getParam<std::string>("energy_advection_interpolation")),
    _mass_advection_interpolation(getParam<std::string>("mass_advection_interpolation")),
    _momentum_scaling(getParam<Real>("momentum_scaling")),
    _energy_scaling(getParam<Real>("energy_scaling")),
    _mass_scaling(getParam<Real>("mass_scaling"))
{
  checkGeneralControErrors();

  checkBoundaryParameterErrors();

  checkAmbientConvectionParameterErrors();

  checkFrictionParameterErrors();

  if (!_ambient_convection_blocks.size() && _blocks.size())
    _ambient_convection_blocks = _blocks;
}

void
NSFVAction::act()
{
  if (_current_task == "add_navier_stokes_variables")
  {
    processBlocks();

    if (_compressibility == "compressible")
      addCNSVariables();
    else if (_compressibility == "weakly-compressible" || _compressibility == "incompressible")
      addINSVariables();
  }

  if (_current_task == "add_navier_stokes_user_objects")
  {
    if (_compressibility == "incompressible" || _compressibility == "weakly-compressible")
      addRhieChowUserObjects();
  }

  if (_current_task == "add_navier_stokes_ics")
  {
    if (_compressibility == "compressible")
      addCNSInitialConditions();
    else if (_compressibility == "weakly-compressible" || _compressibility == "incompressible")
      addINSInitialConditions();
  }

  if (_current_task == "add_navier_stokes_kernels")
  {
    if (_compressibility == "compressible")
    {
      mooseError("compressible flows are not supported yet.");

      if (_type == "transient")
        addCNSTimeKernels();

      addCNSMass();
      addCNSMomentum();
      addCNSEnergy();
    }
    else if (_compressibility == "weakly-compressible")
    {
      mooseError("weakly-compressible flows are not supported yet.");

      if (_type == "transient")
        addWCNSTimeKernels();

      addINSMassKernels();
      addINSMomentumAdvectionKernels();
      addINSMomentumDiffusionKernels();
      addINSMomentumPressureKernels();
      addINSMomentumGravityKernels();
      addINSMomentumFrictionKernels();

      if (_has_energy_equation)
        addWCNSEnergy();
    }
    else if (_compressibility == "incompressible")
    {
      if (_type == "transient")
      {
        addINSMomentumTimeKernels();
        if (_has_energy_equation)
          addINSEnergyTimeKernels();
      }

      addINSMassKernels();
      addINSMomentumAdvectionKernels();
      addINSMomentumDiffusionKernels();
      addINSMomentumPressureKernels();
      addINSMomentumGravityKernels();
      addINSMomentumFrictionKernels();

      if (_has_energy_equation)
        addINSEnergy();
    }
  }

  if (_current_task == "add_navier_stokes_bcs")
  {
    if (_compressibility == "incompressible")
    {
      addINSInletBC();
      addINSOutletBC();
      addINSWallBC();
    }
    else
    {
      mooseError("Weakly-compressible and compressible simulations are not supported yet.");
    }
  }

  if (_current_task == "add_material")
  {
    if (_compressibility == "incompressible" || _compressibility == "weakly-compressible")
    {
      if (_has_energy_equation)
        addEnthalpyMaterial();
      if (_turbulence_handling == "mixing-length")
        addMixingLengthMaterial();
    }
    else
      mooseError("Compressible simulations are not supported yet.");
  }
}

void
NSFVAction::addINSVariables()
{

  if (_porous_medium_treatment)
  {
    auto params = _factory.getValidParams("PINSFVSuperficialVelocityVariable");
    params.set<std::vector<SubdomainName>>("block") = _blocks;

    params.set<std::vector<Real>>("scaling") = {_momentum_scaling};
    for (unsigned int d = 0; d < _dim; ++d)
      _problem->addVariable(
          "PINSFVSuperficialVelocityVariable", NS::superficial_velocity_vector[d], params);

    // params = _factory.getValidParams("INSFVVelocityVariable");
    // params.set<std::vector<SubdomainName>>("block") = _blocks;
    // for (unsigned int d = 0; d < _dim; ++d)
    //   _problem->addAuxVariable("INSFVVelocityVariable", NS::velocity_vector[d], params);
  }
  else
  {
    auto params = _factory.getValidParams("INSFVVelocityVariable");
    params.set<std::vector<SubdomainName>>("block") = _blocks;

    for (unsigned int d = 0; d < _dim; ++d)
      _problem->addVariable("INSFVVelocityVariable", NS::velocity_vector[d], params);
  }

  auto params = _factory.getValidParams("INSFVPressureVariable");
  params.set<std::vector<SubdomainName>>("block") = _blocks;
  params.set<std::vector<Real>>("scaling") = {_mass_scaling};
  _problem->addVariable("INSFVPressureVariable", NS::pressure, params);

  if (getParam<bool>("pin_pressure"))
  {
    auto lm_params = _factory.getValidParams("MooseVariableScalar");
    lm_params.set<MooseEnum>("family") = "scalar";
    lm_params.set<MooseEnum>("order") = "first";
    _problem->addVariable("MooseVariableScalar", "lambda", lm_params);
  }

  if (_turbulence_handling == "mixing-length")
  {
    auto params = _factory.getValidParams("MooseVariableFVReal");
    params.set<std::vector<SubdomainName>>("block") = _blocks;
    _problem->addAuxVariable("MooseVariableFVReal", NS::mixing_length, params);
  }

  if (_has_energy_equation)
  {
    auto params = _factory.getValidParams("INSFVEnergyVariable");
    params.set<std::vector<SubdomainName>>("block") = _blocks;
    params.set<std::vector<Real>>("scaling") = {_energy_scaling};
    _problem->addVariable("INSFVEnergyVariable", NS::T_fluid, params);
  }
}

void
NSFVAction::addCNSVariables()
{
  auto params = _factory.getValidParams("MooseVariableFVReal");
  params.set<std::vector<SubdomainName>>("block") = _blocks;

  params.set<std::vector<Real>>("scaling") = {_momentum_scaling};
  if (_porous_medium_treatment)
    for (unsigned int d = 0; d < _dim; ++d)
      _problem->addVariable("MooseVariableFVReal", NS::superficial_momentum_vector[d], params);
  else
    for (unsigned int d = 0; d < _dim; ++d)
      _problem->addVariable("MooseVariableFVReal", NS::momentum_vector[d], params);

  params.set<std::vector<Real>>("scaling") = {_mass_scaling};
  _problem->addVariable("MooseVariableFVReal", NS::pressure, params);

  params.set<std::vector<Real>>("scaling") = {_energy_scaling};
  _problem->addVariable("MooseVariableFVReal", NS::T_fluid, params);

  for (unsigned int d = 0; d < _dim; ++d)
    _problem->addAuxVariable("MooseVariableFVReal", NS::velocity_vector[d], params);
}

void
NSFVAction::addRhieChowUserObjects()
{
  const std::string u_names[3] = {"u", "v", "w"};
  if (_porous_medium_treatment)
  {
    auto params = _factory.getValidParams("PINSFVRhieChowInterpolator");
    for (unsigned int d = 0; d < _dim; ++d)
      params.set<VariableName>(u_names[d]) = NS::superficial_velocity_vector[d];

    params.set<VariableName>("pressure") = NS::pressure;
    params.set<MooseFunctorName>(NS::porosity) = _porosity_name;

    _problem->addUserObject("PINSFVRhieChowInterpolator", "pins_rhie_chow_interpolator", params);
  }
  else
  {
    auto params = _factory.getValidParams("INSFVRhieChowInterpolator");
    for (unsigned int d = 0; d < _dim; ++d)
      params.set<VariableName>(u_names[d]) = NS::velocity_vector[d];

    params.set<VariableName>("pressure") = NS::pressure;

    _problem->addUserObject("INSFVRhieChowInterpolator", "ins_rhie_chow_interpolator", params);
  }
}

void
NSFVAction::addINSInitialConditions()
{
  InputParameters params = _factory.getValidParams("ConstantIC");
  auto vvalue = getParam<RealVectorValue>("initial_velocity");

  if (_porous_medium_treatment)
    for (unsigned int d = 0; d < _dim; ++d)
    {
      params.set<VariableName>("variable") = NS::superficial_velocity_vector[d];
      params.set<Real>("value") = vvalue(d);
      _problem->addInitialCondition(
          "ConstantIC", NS::superficial_velocity_vector[d] + "_ic", params);
    }
  else
    for (unsigned int d = 0; d < _dim; ++d)
    {
      params.set<VariableName>("variable") = NS::velocity_vector[d];
      params.set<Real>("value") = vvalue(d);
      _problem->addInitialCondition("ConstantIC", NS::velocity_vector[d] + "_ic", params);
    }

  params.set<VariableName>("variable") = NS::pressure;
  params.set<Real>("value") = getParam<Real>("initial_pressure");
  _problem->addInitialCondition("ConstantIC", "pressure_ic", params);

  if (_has_energy_equation)
  {
    params.set<VariableName>("variable") = NS::T_fluid;
    params.set<Real>("value") = getParam<Real>("initial_temperature");
    _problem->addInitialCondition("ConstantIC", NS::T_fluid + "_ic", params);
  }
}

void
NSFVAction::addCNSInitialConditions()
{
  InputParameters params = _factory.getValidParams("ConstantIC");
  auto mvalue = getParam<RealVectorValue>("initial_momentum");

  if (_porous_medium_treatment)
    for (unsigned int d = 0; d < _dim; ++d)
    {
      params.set<VariableName>("variable") = NS::superficial_momentum_vector[d];
      params.set<Real>("value") = mvalue(d);
      _problem->addInitialCondition(
          "ConstantIC", NS::superficial_momentum_vector[d] + "_ic", params);
    }
  else
    for (unsigned int d = 0; d < _dim; ++d)
    {
      params.set<VariableName>("variable") = NS::momentum_vector[d];
      params.set<Real>("value") = mvalue(d);
      _problem->addInitialCondition("ConstantIC", NS::momentum_vector[d] + "_ic", params);
    }

  params.set<VariableName>("variable") = NS::T_fluid;
  params.set<Real>("value") = getParam<Real>("initial_temperature");
  ;
  _problem->addInitialCondition("ConstantIC", NS::T_fluid + "_ic", params);
}

void
NSFVAction::addINSMomentumTimeKernels()
{
  if (_porous_medium_treatment)
  {
    const std::string mom_kernel_type = "PINSFVMomentumTimeDerivative";
    InputParameters params = _factory.getValidParams(mom_kernel_type);
    params.set<std::vector<SubdomainName>>("block") = _blocks;
    params.set<MaterialPropertyName>(NS::density) = _density_name;

    for (unsigned int d = 0; d < _dim; ++d)
    {
      params.set<NonlinearVariableName>("variable") = NS::superficial_velocity_vector[d];
      _problem->addKernel(mom_kernel_type, "pins_momentum_" + NS::directions[d] + "_time", params);
    }
  }
  else
  {
    const std::string mom_kernel_type = "INSFVMomentumTimeDerivative";
    InputParameters params = _factory.getValidParams(mom_kernel_type);
    params.set<std::vector<SubdomainName>>("block") = _blocks;
    params.set<MaterialPropertyName>(NS::density) = _density_name;

    for (unsigned int d = 0; d < _dim; ++d)
    {
      params.set<NonlinearVariableName>("variable") = NS::velocity_vector[d];
      _problem->addKernel(mom_kernel_type, "ins_momentum_" + NS::directions[d] + "_time", params);
    }
  }
}

void
NSFVAction::addINSEnergyTimeKernels()
{
  if (_porous_medium_treatment)
  {
    const std::string en_kernel_type = "PINSFVEnergyTimeDerivative";
    InputParameters params = _factory.getValidParams(en_kernel_type);
    params.set<std::vector<SubdomainName>>("block") = _blocks;

    params.set<NonlinearVariableName>("variable") = NS::T_fluid;
    params.set<MooseFunctorName>(NS::porosity) = _porosity_name;
    params.set<MooseFunctorName>(NS::density) = _density_name;
    params.set<MooseFunctorName>(NS::time_deriv(NS::density)) = NS::time_deriv(_density_name);
    params.set<MooseFunctorName>(NS::cp) = _specific_heat_name;
    params.set<MooseFunctorName>(NS::time_deriv(NS::cp)) = NS::time_deriv(_specific_heat_name);
    _problem->addKernel(en_kernel_type, "pins_energy_time", params);
  }
  else
  {
    const std::string en_kernel_type = "INSFVEnergyTimeDerivative";
    InputParameters params = _factory.getValidParams(en_kernel_type);
    params.set<std::vector<SubdomainName>>("block") = _blocks;

    params.set<NonlinearVariableName>("variable") = NS::T_fluid;
    params.set<MooseFunctorName>(NS::density) = _density_name;
    params.set<MooseFunctorName>(NS::time_deriv(NS::density)) = NS::time_deriv(_density_name);
    params.set<MooseFunctorName>(NS::cp) = _specific_heat_name;
    params.set<MooseFunctorName>(NS::time_deriv(NS::cp)) = NS::time_deriv(_specific_heat_name);
    _problem->addKernel(en_kernel_type, "ins_energy_time", params);
  }
}

void
NSFVAction::addWCNSTimeKernels()
{
  if (_porous_medium_treatment)
  {
    {
      const std::string mass_kernel_type = "PWCNSFVMassTimeDerivative";
      InputParameters params = _factory.getValidParams(mass_kernel_type);
      if (_blocks.size() > 0)
        params.set<std::vector<SubdomainName>>("block") = _blocks;

      params.set<NonlinearVariableName>("variable") = NS::pressure;
      params.set<MooseFunctorName>(NS::porosity) = _porosity_name;
      params.set<MaterialPropertyName>(NS::time_deriv(NS::density)) = NS::time_deriv(_density_name);

      _problem->addKernel(mass_kernel_type, "pwcns_mass_time", params);
    }
    {
      const std::string mom_kernel_type = "WCNSFVMomentumTimeDerivative";
      InputParameters params = _factory.getValidParams(mom_kernel_type);
      if (_blocks.size() > 0)
        params.set<std::vector<SubdomainName>>("block") = _blocks;

      params.set<MaterialPropertyName>(NS::density) = _density_name;
      params.set<MaterialPropertyName>(NS::time_deriv(NS::density)) = NS::time_deriv(_density_name);

      for (unsigned int d = 0; d < _dim; ++d)
      {
        params.set<NonlinearVariableName>("variable") = NS::superficial_velocity_vector[d];
        _problem->addKernel(
            mom_kernel_type, "pwcns_momentum_" + NS::directions[d] + "_time", params);
      }
    }
    if (_has_energy_equation)
    {
      const std::string en_kernel_type = "PINSFVEnergyTimeDerivative";
      InputParameters params = _factory.getValidParams(en_kernel_type);
      if (_blocks.size() > 0)
        params.set<std::vector<SubdomainName>>("block") = _blocks;

      params.set<NonlinearVariableName>("variable") = NS::T_fluid;
      params.set<MooseFunctorName>(NS::porosity) = _porosity_name;
      params.set<MooseFunctorName>(NS::density) = _density_name;
      params.set<MooseFunctorName>(NS::time_deriv(NS::density)) = NS::time_deriv(_density_name);
      params.set<MooseFunctorName>(NS::cp) = _specific_heat_name;
      params.set<MooseFunctorName>(NS::time_deriv(NS::cp)) = NS::time_deriv(_specific_heat_name);
      _problem->addKernel(en_kernel_type, "pwcns_energy_time", params);
    }
  }
  else
  {
    {
      const std::string mass_kernel_type = "WCNSFVMassTimeDerivative";
      InputParameters params = _factory.getValidParams(mass_kernel_type);
      if (_blocks.size() > 0)
        params.set<std::vector<SubdomainName>>("block") = _blocks;
      params.set<NonlinearVariableName>("variable") = NS::pressure;
      params.set<MaterialPropertyName>(NS::time_deriv(NS::density)) = NS::time_deriv(_density_name);
      _problem->addKernel(mass_kernel_type, "wcns_mass_time", params);
    }
    {
      const std::string mom_kernel_type = "WCNSFVMomentumTimeDerivative";
      InputParameters params = _factory.getValidParams(mom_kernel_type);
      if (_blocks.size() > 0)
        params.set<std::vector<SubdomainName>>("block") = _blocks;

      params.set<MaterialPropertyName>(NS::density) = _density_name;
      params.set<MaterialPropertyName>(NS::time_deriv(NS::density)) = NS::time_deriv(_density_name);

      for (unsigned int d = 0; d < _dim; ++d)
      {
        params.set<NonlinearVariableName>("variable") = NS::velocity_vector[d];
        _problem->addKernel(
            mom_kernel_type, "wcns_momentum_" + NS::directions[d] + "_time", params);
      }
    }
    if (_has_energy_equation)
    {
      const std::string en_kernel_type = "INSFVEnergyTimeDerivative";
      InputParameters params = _factory.getValidParams(en_kernel_type);
      if (_blocks.size() > 0)
        params.set<std::vector<SubdomainName>>("block") = _blocks;

      params.set<NonlinearVariableName>("variable") = NS::T_fluid;
      params.set<MooseFunctorName>(NS::density) = _density_name;
      params.set<MooseFunctorName>(NS::time_deriv(NS::density)) = NS::time_deriv(_density_name);
      params.set<MooseFunctorName>(NS::cp) = _specific_heat_name;
      params.set<MooseFunctorName>(NS::time_deriv(NS::cp)) = NS::time_deriv(_specific_heat_name);
      _problem->addKernel(en_kernel_type, "wcns_energy_time", params);
    }
  }
}

void
NSFVAction::addCNSTimeKernels()
{
  mooseError("Compressible simulations are not supported yet!");
}

void
NSFVAction::addINSMassKernels()
{
  if (_porous_medium_treatment)
  {
    const std::string kernel_type = "PINSFVMassAdvection";
    InputParameters params = _factory.getValidParams(kernel_type);
    params.set<std::vector<SubdomainName>>("block") = _blocks;

    params.set<NonlinearVariableName>("variable") = NS::pressure;
    params.set<MooseFunctorName>(NS::density) = _density_name;
    params.set<MooseEnum>("velocity_interp_method") = "rc";
    params.set<UserObjectName>("rhie_chow_user_object") = "pins_rhie_chow_interpolator";
    params.set<MooseEnum>("advected_interp_method") = _mass_advection_interpolation;

    _problem->addFVKernel(kernel_type, "pins_mass_advection", params);
  }
  else
  {
    const std::string kernel_type = "INSFVMassAdvection";
    InputParameters params = _factory.getValidParams(kernel_type);
    params.set<std::vector<SubdomainName>>("block") = _blocks;

    params.set<NonlinearVariableName>("variable") = NS::pressure;
    params.set<MooseFunctorName>(NS::density) = _density_name;
    params.set<MooseEnum>("velocity_interp_method") = "rc";
    params.set<UserObjectName>("rhie_chow_user_object") = "ins_rhie_chow_interpolator";
    params.set<MooseEnum>("advected_interp_method") = _mass_advection_interpolation;

    _problem->addFVKernel(kernel_type, "ins_mass_advection", params);
  }

  if (getParam<bool>("pin_pressure"))
  {
    const std::string kernel_type = "FVScalarLagrangeMultiplier";
    InputParameters params = _factory.getValidParams(kernel_type);
    params.set<CoupledName>("lambda") = {"lambda"};
    params.set<Real>("phi0") = getParam<Real>("pinned_pressure_value");
    params.set<NonlinearVariableName>("variable") = NS::pressure;
    MooseEnum pin_type = getParam<MooseEnum>("pinned_pressure_type");
    params.set<MooseEnum>("constraint_type") = pin_type;
    if (pin_type == "point-value")
      params.set<Point>("point") = getParam<Point>("pinned_pressure_point");
    _problem->addFVKernel(kernel_type, "ins_mass_pressure_constraint", params);
  }
}

void
NSFVAction::addCNSMass()
{
  _console << "something here" << std::endl;
}

void
NSFVAction::addINSMomentumAdvectionKernels()
{
  if (_porous_medium_treatment)
  {
    const std::string adv_kernel_type = "PINSFVMomentumAdvection";
    InputParameters params = _factory.getValidParams(adv_kernel_type);
    params.set<std::vector<SubdomainName>>("block") = _blocks;

    params.set<MooseFunctorName>(NS::density) = _density_name;
    params.set<MooseFunctorName>(NS::porosity) = _porosity_name;
    params.set<MooseEnum>("velocity_interp_method") = "rc";
    params.set<UserObjectName>("rhie_chow_user_object") = "pins_rhie_chow_interpolator";
    params.set<MooseEnum>("advected_interp_method") = _momentum_advection_interpolation;

    for (unsigned int d = 0; d < _dim; ++d)
    {
      params.set<NonlinearVariableName>("variable") = NS::superficial_velocity_vector[d];
      params.set<MooseEnum>("momentum_component") = NS::directions[d];
      _problem->addFVKernel(
          adv_kernel_type, "pins_momentum_" + NS::directions[d] + "_advection", params);
    }
  }
  else
  {
    const std::string adv_kernel_type = "INSFVMomentumAdvection";
    InputParameters params = _factory.getValidParams(adv_kernel_type);
    params.set<std::vector<SubdomainName>>("block") = _blocks;

    params.set<MooseFunctorName>(NS::density) = _density_name;
    params.set<MooseEnum>("velocity_interp_method") = "rc";
    params.set<UserObjectName>("rhie_chow_user_object") = "ins_rhie_chow_interpolator";
    params.set<MooseEnum>("advected_interp_method") = _momentum_advection_interpolation;

    for (unsigned int d = 0; d < _dim; ++d)
    {
      params.set<NonlinearVariableName>("variable") = NS::velocity_vector[d];
      params.set<MooseEnum>("momentum_component") = NS::directions[d];
      _problem->addFVKernel(
          adv_kernel_type, "ins_momentum_" + NS::directions[d] + "_advection", params);
    }
  }
}

void
NSFVAction::addINSMomentumDiffusionKernels()
{
  if (_porous_medium_treatment)
  {
    const std::string diff_kernel_type = "PINSFVMomentumDiffusion";
    InputParameters params = _factory.getValidParams(diff_kernel_type);
    params.set<std::vector<SubdomainName>>("block") = _blocks;

    params.set<UserObjectName>("rhie_chow_user_object") = "pins_rhie_chow_interpolator";

    params.set<MooseFunctorName>(NS::mu) = _dynamic_viscosity_name;
    params.set<MooseFunctorName>(NS::porosity) = _porosity_name;

    for (unsigned int d = 0; d < _dim; ++d)
    {
      params.set<NonlinearVariableName>("variable") = NS::superficial_velocity_vector[d];
      params.set<MooseEnum>("momentum_component") = NS::directions[d];
      addRelationshipManager("pins_momentum_" + NS::directions[d] + "_diffusion", 2, params);
      _problem->addFVKernel(
          diff_kernel_type, "pins_momentum_" + NS::directions[d] + "_diffusion", params);
    }
  }
  else
  {
    const std::string diff_kernel_type = "INSFVMomentumDiffusion";
    InputParameters params = _factory.getValidParams(diff_kernel_type);
    params.set<std::vector<SubdomainName>>("block") = _blocks;

    params.set<UserObjectName>("rhie_chow_user_object") = "ins_rhie_chow_interpolator";
    params.set<MooseFunctorName>(NS::mu) = _dynamic_viscosity_name;

    for (unsigned int d = 0; d < _dim; ++d)
    {
      params.set<NonlinearVariableName>("variable") = NS::velocity_vector[d];
      params.set<MooseEnum>("momentum_component") = NS::directions[d];
      addRelationshipManager("ins_momentum_" + NS::directions[d] + "_diffusion", 2, params);
      _problem->addFVKernel(
          diff_kernel_type, "ins_momentum_" + NS::directions[d] + "_diffusion", params);
    }

    if (_turbulence_handling == "mixing-length")
    {
      const std::string u_names[3] = {"u", "v", "w"};

      const std::string kernel_type = "INSFVMixingLengthReynoldsStress";
      InputParameters params = _factory.getValidParams(kernel_type);
      params.set<std::vector<SubdomainName>>("block") = _blocks;

      params.set<UserObjectName>("rhie_chow_user_object") = "ins_rhie_chow_interpolator";
      params.set<MooseFunctorName>(NS::density) = NS::density;
      params.set<MooseFunctorName>(NS::mixing_length) = NS::mixing_length;
      for (unsigned int dim_i = 0; dim_i < _dim; ++dim_i)
        params.set<CoupledName>(u_names[dim_i]) = {NS::velocity_vector[dim_i]};

      for (unsigned int d = 0; d < _dim; ++d)
      {
        params.set<NonlinearVariableName>("variable") = NS::velocity_vector[d];
        params.set<MooseEnum>("momentum_component") = NS::directions[d];
        _problem->addFVKernel(kernel_type,
                              "ins_momentum_" + NS::directions[d] +
                                  "_mixing_length_reynolds_stress",
                              params);
        addRelationshipManager(
            "ins_momentum_" + NS::directions[d] + "_mixing_length_reynolds_stress", 3, params);
      }

      const std::string ml_kernel_type = "WallDistanceMixingLengthAux";
      InputParameters ml_params = _factory.getValidParams(ml_kernel_type);
      ml_params.set<std::vector<SubdomainName>>("block") = _blocks;
      ml_params.set<AuxVariableName>("variable") = NS::mixing_length;
      ml_params.set<std::vector<BoundaryName>>("walls") =
          getParam<std::vector<BoundaryName>>("mixing_length_walls");
      if (isParamValid("mixing_length_aux_execute_on"))
        ml_params.set<ExecFlagEnum>("execute_on") =
            getParam<ExecFlagEnum>("mixing_length_aux_execute_on");
      ml_params.set<Real>("von_karman_const") = getParam<Real>("von_karman_const");
      ml_params.set<Real>("von_karman_const_0") = getParam<Real>("von_karman_const_0");
      ml_params.set<Real>("delta") = getParam<Real>("mixing_length_delta");

      _problem->addAuxKernel(ml_kernel_type, "mixing_length_aux ", ml_params);
    }
  }
}

void
NSFVAction::addINSMomentumPressureKernels()
{
  if (_porous_medium_treatment)
  {
    const std::string press_kernel_type = "PINSFVMomentumPressure";
    InputParameters params = _factory.getValidParams(press_kernel_type);
    params.set<std::vector<SubdomainName>>("block") = _blocks;

    params.set<UserObjectName>("rhie_chow_user_object") = "pins_rhie_chow_interpolator";

    params.set<MooseFunctorName>(NS::porosity) = _porosity_name;
    params.set<CoupledName>("pressure") = {NS::pressure};

    for (unsigned int d = 0; d < _dim; ++d)
    {
      params.set<NonlinearVariableName>("variable") = NS::superficial_velocity_vector[d];
      params.set<MooseEnum>("momentum_component") = NS::directions[d];
      _problem->addFVKernel(
          press_kernel_type, "pins_momentum_" + NS::directions[d] + "_pressure", params);
    }
  }
  else
  {
    const std::string press_kernel_type = "INSFVMomentumPressure";
    InputParameters params = _factory.getValidParams(press_kernel_type);
    params.set<std::vector<SubdomainName>>("block") = _blocks;

    params.set<UserObjectName>("rhie_chow_user_object") = "ins_rhie_chow_interpolator";

    params.set<CoupledName>("pressure") = {NS::pressure};

    for (unsigned int d = 0; d < _dim; ++d)
    {
      params.set<NonlinearVariableName>("variable") = NS::velocity_vector[d];
      params.set<MooseEnum>("momentum_component") = NS::directions[d];
      _problem->addFVKernel(
          press_kernel_type, "ins_momentum_" + NS::directions[d] + "_pressure", params);
    }
  }
}

void
NSFVAction::addINSMomentumGravityKernels()
{
  if (isParamValid("gravity"))
  {
    if (_porous_medium_treatment)
    {
      const std::string grav_kernel_type = "PINSFVMomentumGravity";
      InputParameters params = _factory.getValidParams(grav_kernel_type);
      params.set<std::vector<SubdomainName>>("block") = _blocks;

      params.set<UserObjectName>("rhie_chow_user_object") = "pins_rhie_chow_interpolator";

      params.set<MooseFunctorName>(NS::porosity) = _porosity_name;
      params.set<MooseFunctorName>(NS::density) = _density_name;
      params.set<RealVectorValue>("gravity") = getParam<RealVectorValue>("gravity");

      for (unsigned int d = 0; d < _dim; ++d)
      {
        params.set<MooseEnum>("momentum_component") = NS::directions[d];
        params.set<NonlinearVariableName>("variable") = NS::superficial_velocity_vector[d];
        _problem->addFVKernel(
            grav_kernel_type, "pins_momentum_" + NS::directions[d] + "_gravity", params);
      }

      if (_boussinesq_approximation && !(_compressibility == "weakly-compressible"))
      {
        const std::string boussinesq_kernel_type = "PINSFVMomentumBoussinesq";
        InputParameters params = _factory.getValidParams(boussinesq_kernel_type);
        params.set<std::vector<SubdomainName>>("block") = _blocks;

        params.set<UserObjectName>("rhie_chow_user_object") = "pins_rhie_chow_interpolator";

        params.set<MooseFunctorName>(NS::T_fluid) = NS::T_fluid;
        params.set<MooseFunctorName>(NS::porosity) = _porosity_name;
        params.set<MooseFunctorName>(NS::density) = _density_name;
        params.set<RealVectorValue>("gravity") = getParam<RealVectorValue>("gravity");
        params.set<Real>("ref_temperature") = getParam<Real>("ref_temperature");
        params.set<MooseFunctorName>("alpha_name") = _thermal_expansion_name;

        for (unsigned int d = 0; d < _dim; ++d)
        {
          params.set<MooseEnum>("momentum_component") = NS::directions[d];
          params.set<NonlinearVariableName>("variable") = NS::superficial_velocity_vector[d];
          _problem->addKernel(
              boussinesq_kernel_type, "pins_momentum_" + NS::directions[d] + "_boussinesq", params);
        }
      }
    }
    else
    {
      const std::string grav_kernel_type = "INSFVMomentumGravity";
      InputParameters params = _factory.getValidParams(grav_kernel_type);
      params.set<std::vector<SubdomainName>>("block") = _blocks;

      params.set<UserObjectName>("rhie_chow_user_object") = "ins_rhie_chow_interpolator";

      params.set<MooseFunctorName>(NS::density) = _density_name;
      params.set<RealVectorValue>("gravity") = getParam<RealVectorValue>("gravity");

      for (unsigned int d = 0; d < _dim; ++d)
      {
        params.set<MooseEnum>("momentum_component") = NS::directions[d];
        params.set<NonlinearVariableName>("variable") = NS::velocity_vector[d];
        _problem->addFVKernel(
            grav_kernel_type, "ins_momentum_" + NS::directions[d] + "_gravity", params);
      }

      if (_boussinesq_approximation && !(_compressibility == "weakly-compressible"))
      {
        const std::string boussinesq_kernel_type = "INSFVMomentumBoussinesq";
        InputParameters params = _factory.getValidParams(boussinesq_kernel_type);
        if (_blocks.size() > 0)
          params.set<std::vector<SubdomainName>>("block") = _blocks;

        params.set<UserObjectName>("rhie_chow_user_object") = "ins_rhie_chow_interpolator";

        params.set<MooseFunctorName>(NS::density) = _density_name;
        params.set<RealVectorValue>("gravity") = getParam<RealVectorValue>("gravity");
        params.set<Real>("ref_temperature") = getParam<Real>("ref_temperature");
        params.set<MooseFunctorName>(NS::T_fluid) = NS::T_fluid;
        params.set<MooseFunctorName>("alpha_name") = _thermal_expansion_name;

        for (unsigned int d = 0; d < _dim; ++d)
        {
          params.set<MooseEnum>("momentum_component") = NS::directions[d];
          params.set<NonlinearVariableName>("variable") = NS::velocity_vector[d];
          _problem->addFVKernel(
              boussinesq_kernel_type, "ins_momentum_" + NS::directions[d] + "_boussinesq", params);
        }
      }
    }
  }
}

void
NSFVAction::addINSMomentumFrictionKernels()
{

  if (_friction_blocks.size())
  {
    const std::string kernel_type = "PINSFVMomentumFriction";
    InputParameters params = _factory.getValidParams(kernel_type);

    for (unsigned int block_i = 0; block_i < _friction_blocks.size(); ++block_i)
    {
      params.set<std::vector<SubdomainName>>("block") = {_friction_blocks[block_i]};
      params.set<MooseFunctorName>(NS::density) = _density_name;

      for (unsigned int d = 0; d < _dim; ++d)
      {
        NonlinearVariableName vname;
        if (_porous_medium_treatment)
        {
          vname = NS::superficial_velocity_vector[d];
          params.set<UserObjectName>("rhie_chow_user_object") = "pins_rhie_chow_interpolator";
          params.set<MooseFunctorName>(NS::porosity) = _porosity_name;
        }
        else
        {
          vname = NS::velocity_vector[d];
          params.set<UserObjectName>("rhie_chow_user_object") = "ins_rhie_chow_interpolator";
          params.set<MooseFunctorName>(NS::porosity) = "1";
        }

        params.set<NonlinearVariableName>("variable") = vname;
        params.set<MooseEnum>("momentum_component") = NS::directions[d];
        for (unsigned int type_i = 0; type_i < _friction_types[block_i].size(); ++type_i)
        {
          const auto upper_name = MooseUtils::toUpper(_friction_types[block_i][type_i]);
          if (upper_name == "DARCY")
            params.set<MooseFunctorName>("Darcy_name") = _friction_coeffs[block_i][type_i];
          else if (upper_name == "FORCHHEIMER")
            params.set<MooseFunctorName>("Forchheimer_name") = _friction_coeffs[block_i][type_i];
        }

        _problem->addFVKernel(
            kernel_type, "momentum_friction_" + _friction_blocks[block_i] + "_" + vname, params);
      }
    }
  }
  else
  {
    if (_friction_types.size())
    {
      const std::string kernel_type = "PINSFVMomentumFriction";
      InputParameters params = _factory.getValidParams(kernel_type);
      params.set<std::vector<SubdomainName>>("block") = _blocks;
      params.set<MooseFunctorName>(NS::density) = _density_name;

      for (unsigned int d = 0; d < _dim; ++d)
      {
        NonlinearVariableName vname;
        if (_porous_medium_treatment)
        {
          vname = NS::superficial_velocity_vector[d];
          params.set<UserObjectName>("rhie_chow_user_object") = "pins_rhie_chow_interpolator";
          params.set<MooseFunctorName>(NS::porosity) = _porosity_name;
        }
        else
        {
          vname = NS::velocity_vector[d];
          params.set<UserObjectName>("rhie_chow_user_object") = "ins_rhie_chow_interpolator";
          params.set<MooseFunctorName>(NS::porosity) = "1";
        }

        params.set<NonlinearVariableName>("variable") = vname;
        params.set<MooseEnum>("momentum_component") = NS::directions[d];
        for (unsigned int type_i = 0; type_i < _friction_types[0].size(); ++type_i)
        {
          const auto upper_name = MooseUtils::toUpper(_friction_types[0][type_i]);
          if (upper_name == "DARCY")
            params.set<MooseFunctorName>("Darcy_name") = _friction_coeffs[0][type_i];
          else if (upper_name == "FORCHHEIMER")
            params.set<MooseFunctorName>("Forchheimer_name") = _friction_coeffs[0][type_i];
        }

        _problem->addFVKernel(kernel_type, "momentum_friction_" + vname, params);
      }
    }
  }
}

void
NSFVAction::addCNSMomentum()
{
  _console << "something here" << std::endl;
}

void
NSFVAction::addINSEnergy()
{
  if (_porous_medium_treatment)
  {
    {
      const std::string kernel_type = "PINSFVEnergyAdvection";
      InputParameters params = _factory.getValidParams(kernel_type);
      params.set<NonlinearVariableName>("variable") = NS::T_fluid;
      if (_blocks.size() > 0)
        params.set<std::vector<SubdomainName>>("block") = _blocks;

      params.set<MooseEnum>("velocity_interp_method") = "rc";
      params.set<UserObjectName>("rhie_chow_user_object") = "pins_rhie_chow_interpolator";
      params.set<MooseEnum>("advected_interp_method") = _energy_advection_interpolation;

      _problem->addFVKernel(kernel_type, "pins_energy_advection", params);
    }
    {
      const std::string kernel_type = "PINSFVEnergyDiffusion";
      InputParameters params = _factory.getValidParams(kernel_type);
      params.set<NonlinearVariableName>("variable") = NS::T_fluid;
      if (_blocks.size() > 0)
        params.set<std::vector<SubdomainName>>("block") = _blocks;

      params.set<MooseFunctorName>(NS::k) = _thermal_conductivity_name;
      params.set<MooseFunctorName>(NS::porosity) = _porosity_name;

      _problem->addFVKernel(kernel_type, "pins_energy_diffusion", params);
    }
    if (_turbulence_handling == "mixing-length")
    {
      mooseError("Turbulence handling is not implemented yet!");
    }
  }
  else
  {
    {
      const std::string kernel_type = "INSFVEnergyAdvection";
      InputParameters params = _factory.getValidParams(kernel_type);
      params.set<NonlinearVariableName>("variable") = NS::T_fluid;
      if (_blocks.size() > 0)
        params.set<std::vector<SubdomainName>>("block") = _blocks;

      params.set<MooseEnum>("velocity_interp_method") = "rc";
      params.set<UserObjectName>("rhie_chow_user_object") = "ins_rhie_chow_interpolator";
      params.set<MooseEnum>("advected_interp_method") = _momentum_advection_interpolation;

      _problem->addFVKernel(kernel_type, "ins_energy_convection", params);
    }
    {
      const std::string kernel_type = "FVDiffusion";
      InputParameters params = _factory.getValidParams(kernel_type);
      params.set<NonlinearVariableName>("variable") = NS::T_fluid;
      if (_blocks.size() > 0)
        params.set<std::vector<SubdomainName>>("block") = _blocks;

      params.set<MooseFunctorName>("coeff") = _thermal_conductivity_name;

      _problem->addFVKernel(kernel_type, "ins_energy_diffusion", params);
    }

    if (_turbulence_handling == "mixing-length")
    {
      mooseError("Turbulence handling is not implemented yet!");
    }
  }

  if (_ambient_convection_blocks.size())
  {
    for (unsigned int block_i = 0; block_i < _ambient_convection_blocks.size(); ++block_i)
    {
      const std::string kernel_type = "PINSFVEnergyAmbientConvection";
      InputParameters params = _factory.getValidParams(kernel_type);
      params.set<NonlinearVariableName>("variable") = NS::T_fluid;
      params.set<std::vector<SubdomainName>>("block") = {_ambient_convection_blocks[block_i]};
      params.set<MooseFunctorName>("h_solid_fluid") = _ambient_convection_alpha[block_i];
      params.set<MooseFunctorName>(NS::T_solid) = _ambient_temperature[block_i];
      params.set<MooseFunctorName>(NS::T_fluid) = NS::T_fluid;
      params.set<bool>("is_solid") = false;

      _problem->addFVKernel(
          kernel_type, "ambient_convection_" + _ambient_convection_blocks[block_i], params);
    }
  }
  else
  {
    if (_ambient_convection_alpha.size())
    {
      const std::string kernel_type = "PINSFVEnergyAmbientConvection";
      InputParameters params = _factory.getValidParams(kernel_type);
      params.set<NonlinearVariableName>("variable") = NS::T_fluid;
      params.set<std::vector<SubdomainName>>("block") = _ambient_convection_blocks;
      params.set<MooseFunctorName>("h_solid_fluid") = _ambient_convection_alpha[0];
      params.set<MooseFunctorName>(NS::T_solid) = _ambient_temperature[0];
      params.set<MooseFunctorName>(NS::T_fluid) = NS::T_fluid;
      params.set<bool>("is_solid") = false;

      _problem->addFVKernel(kernel_type, "ambient_convection", params);
    }
  }
}

void
NSFVAction::addWCNSEnergy()
{
  _console << "something here" << std::endl;
}

void
NSFVAction::addCNSEnergy()
{
  _console << "something here" << std::endl;
}

void
NSFVAction::addINSInletBC()
{
  for (unsigned int bc_ind = 0; bc_ind < _inlet_boundaries.size(); ++bc_ind)
  {
    if (_momentum_inlet_types[bc_ind] == "fixed-velocity")
    {
      const std::string bc_type = "INSFVInletVelocityBC";
      InputParameters params = _factory.getValidParams(bc_type);
      params.set<std::vector<BoundaryName>>("boundary") = {_inlet_boundaries[bc_ind]};

      for (unsigned int d = 0; d < _dim; ++d)
      {
        NonlinearVariableName vname;
        if (_porous_medium_treatment)
          vname = NS::superficial_velocity_vector[d];
        else
          vname = NS::velocity_vector[d];

        params.set<NonlinearVariableName>("variable") = vname;
        params.set<FunctionName>("function") = _momentum_inlet_function[bc_ind * _dim + d];

        _problem->addFVBC(bc_type, vname + "_" + _inlet_boundaries[bc_ind], params);
      }
    }
    else
      mooseError(_momentum_inlet_types[bc_ind] +
                 " inlet BC is not supported for INS simulations at the moment!");

    if (_has_energy_equation)
    {
      if (_energy_inlet_types[bc_ind] == "fixed-temperature")
      {
        const std::string bc_type = "FVFunctionDirichletBC";
        InputParameters params = _factory.getValidParams(bc_type);
        params.set<NonlinearVariableName>("variable") = NS::T_fluid;
        params.set<FunctionName>("function") = _energy_inlet_function[bc_ind];
        params.set<std::vector<BoundaryName>>("boundary") = {_inlet_boundaries[bc_ind]};
        _problem->addFVBC(bc_type, NS::T_fluid + "_" + _inlet_boundaries[bc_ind], params);
      }
      else if (_energy_inlet_types[bc_ind] == "heatflux")
      {
        const std::string bc_type = "FVFunctionNeumannBC";
        InputParameters params = _factory.getValidParams(bc_type);
        params.set<NonlinearVariableName>("variable") = NS::T_fluid;
        params.set<FunctionName>("function") = _energy_inlet_function[bc_ind];
        params.set<std::vector<BoundaryName>>("boundary") = {_inlet_boundaries[bc_ind]};
        _problem->addFVBC(bc_type, NS::T_fluid + "_" + _inlet_boundaries[bc_ind], params);
      }
      else
        mooseError(_energy_inlet_types[bc_ind] +
                   " inlet BC is not supported for INS simulations at the moment!");
    }
  }
}

void
NSFVAction::addINSOutletBC()
{
  const std::string u_names[3] = {"u", "v", "w"};
  for (unsigned int bc_ind = 0; bc_ind < _outlet_boundaries.size(); ++bc_ind)
  {
    if (_momentum_outlet_types[bc_ind] == "fixed-pressure")
    {
      const std::string bc_type = "INSFVOutletPressureBC";
      InputParameters params = _factory.getValidParams(bc_type);
      params.set<NonlinearVariableName>("variable") = NS::pressure;
      params.set<FunctionName>("function") = _pressure_function[bc_ind];
      params.set<std::vector<BoundaryName>>("boundary") = {_outlet_boundaries[bc_ind]};

      _problem->addFVBC(bc_type, NS::pressure + "_" + _outlet_boundaries[bc_ind], params);
      addRelationshipManager(NS::pressure + "_" + _outlet_boundaries[bc_ind], 2, params);
    }
    else if (_momentum_outlet_types[bc_ind] == "zero-gradient")
    {
      {
        const std::string bc_type = "INSFVMassAdvectionOutflowBC";
        InputParameters params = _factory.getValidParams(bc_type);
        params.set<NonlinearVariableName>("variable") = NS::pressure;
        params.set<MooseFunctorName>(NS::density) = _density_name;
        params.set<std::vector<BoundaryName>>("boundary") = {_outlet_boundaries[bc_ind]};

        if (_porous_medium_treatment)
          for (unsigned int d = 0; d < _dim; ++d)
            params.set<CoupledName>(u_names[d]) = {NS::superficial_velocity_vector[d]};
        else
          for (unsigned int d = 0; d < _dim; ++d)
            params.set<CoupledName>(u_names[d]) = {NS::velocity_vector[d]};

        _problem->addFVBC(bc_type, NS::pressure + "_" + _outlet_boundaries[bc_ind], params);
      }

      {
        if (_porous_medium_treatment)
        {
          const std::string bc_type = "PINSFVMomentumAdvectionOutflowBC";
          InputParameters params = _factory.getValidParams(bc_type);
          params.set<std::vector<BoundaryName>>("boundary") = {_outlet_boundaries[bc_ind]};
          params.set<MooseFunctorName>(NS::porosity) = _porosity_name;
          params.set<UserObjectName>("rhie_chow_user_object") = "pins_rhie_chow_interpolator";
          params.set<MooseFunctorName>(NS::density) = _density_name;

          for (unsigned int i = 0; i < _dim; ++i)
            params.set<CoupledName>(u_names[i]) = {NS::superficial_velocity_vector[i]};

          for (unsigned int d = 0; d < _dim; ++d)
          {
            params.set<NonlinearVariableName>("variable") = NS::superficial_velocity_vector[d];
            params.set<MooseEnum>("momentum_component") = NS::directions[d];

            _problem->addFVBC(bc_type,
                              NS::superficial_velocity_vector[d] + "_" + _outlet_boundaries[bc_ind],
                              params);
          }
        }
        else
        {
          const std::string bc_type = "PINSFVMomentumAdvectionOutflowBC";
          InputParameters params = _factory.getValidParams(bc_type);
          params.set<std::vector<BoundaryName>>("boundary") = {_outlet_boundaries[bc_ind]};
          params.set<UserObjectName>("rhie_chow_user_object") = "ins_rhie_chow_interpolator";
          params.set<MooseFunctorName>(NS::density) = _density_name;

          for (unsigned int i = 0; i < _dim; ++i)
            params.set<CoupledName>(u_names[i]) = {NS::velocity_vector[i]};

          for (unsigned int d = 0; d < _dim; ++d)
          {
            params.set<NonlinearVariableName>("variable") = NS::velocity_vector[d];
            params.set<MooseEnum>("momentum_component") = NS::directions[d];

            _problem->addFVBC(
                bc_type, NS::velocity_vector[d] + "_" + _outlet_boundaries[bc_ind], params);
          }
        }
      }
    }
    else if (_momentum_outlet_types[bc_ind] == "fixed-pressure-zero-gradient")
    {
      {
        const std::string bc_type = "INSFVMomentumAdvectionOutflowBC";
        InputParameters params = _factory.getValidParams(bc_type);
        params.set<std::vector<BoundaryName>>("boundary") = {_outlet_boundaries[bc_ind]};

        for (unsigned int d = 0; d < _dim; ++d)
        {
          NonlinearVariableName vname;
          if (_porous_medium_treatment)
          {
            vname = NS::superficial_velocity_vector[d];
            for (unsigned int d = 0; d < _dim; ++d)
              params.set<CoupledName>(u_names[d]) = {NS::superficial_velocity_vector[d]};
            params.set<UserObjectName>("rhie_chow_user_object") = "pins_rhie_chow_interpolator";
          }
          else
          {
            vname = NS::velocity_vector[d];
            for (unsigned int d = 0; d < _dim; ++d)
              params.set<CoupledName>(u_names[d]) = {NS::velocity_vector[d]};
            params.set<UserObjectName>("rhie_chow_user_object") = "ins_rhie_chow_interpolator";
          }

          params.set<NonlinearVariableName>("variable") = vname;
          params.set<MooseEnum>("momentum_component") = NS::directions[d];
          params.set<MooseFunctorName>(NS::density) = _density_name;

          _problem->addFVBC(bc_type, vname + "_" + _outlet_boundaries[bc_ind], params);
        }
      }
      {
        const std::string bc_type = "INSFVOutletPressureBC";
        InputParameters params = _factory.getValidParams(bc_type);
        params.set<NonlinearVariableName>("variable") = NS::pressure;
        params.set<FunctionName>("function") = _pressure_function[bc_ind];
        params.set<std::vector<BoundaryName>>("boundary") = {_outlet_boundaries[bc_ind]};
        _problem->addFVBC(bc_type, NS::pressure + "_" + _outlet_boundaries[bc_ind], params);
      }
    }
    else
      mooseError(_momentum_outlet_types[bc_ind] +
                 " outlet BC is not supported for INS simulations at the moment!");
  }
}

void
NSFVAction::addINSWallBC()
{
  const std::string u_names[3] = {"u", "v", "w"};
  for (unsigned int bc_ind = 0; bc_ind < _wall_boundaries.size(); ++bc_ind)
  {
    if (_momentum_wall_types[bc_ind] == "noslip")
    {
      const std::string bc_type = "INSFVNoSlipWallBC";
      InputParameters params = _factory.getValidParams(bc_type);
      params.set<std::vector<BoundaryName>>("boundary") = {_wall_boundaries[bc_ind]};

      for (unsigned int d = 0; d < _dim; ++d)
      {
        NonlinearVariableName vname;
        if (_porous_medium_treatment)
          vname = NS::superficial_velocity_vector[d];
        else
          vname = NS::velocity_vector[d];

        params.set<NonlinearVariableName>("variable") = vname;
        params.set<FunctionName>("function") = "0";

        _problem->addFVBC(bc_type, vname + "_" + _wall_boundaries[bc_ind], params);
      }
    }
    else if (_momentum_wall_types[bc_ind] == "wallfunction")
    {
      const std::string bc_type = "INSFVWallFunctionBC";
      InputParameters params = _factory.getValidParams(bc_type);
      params.set<MaterialPropertyName>(NS::mu) = _dynamic_viscosity_name;
      params.set<MaterialPropertyName>(NS::density) = _density_name;
      params.set<std::vector<BoundaryName>>("boundary") = {_wall_boundaries[bc_ind]};

      if (_porous_medium_treatment)
      {
        params.set<UserObjectName>("rhie_chow_user_object") = "pins_rhie_chow_interpolator";
        for (unsigned int d = 0; d < _dim; ++d)
          params.set<CoupledName>(u_names[d]) = {NS::superficial_velocity_vector[d]};
      }
      else
      {
        params.set<UserObjectName>("rhie_chow_user_object") = "ins_rhie_chow_interpolator";
        for (unsigned int d = 0; d < _dim; ++d)
          params.set<CoupledName>(u_names[d]) = {NS::velocity_vector[d]};
      }

      for (unsigned int d = 0; d < _dim; ++d)
      {
        NonlinearVariableName vname;
        if (_porous_medium_treatment)
          vname = NS::superficial_velocity_vector[d];
        else
          vname = NS::velocity_vector[d];

        params.set<NonlinearVariableName>("variable") = vname;
        params.set<MooseEnum>("momentum_component") = NS::directions[d];

        _problem->addFVBC(bc_type, vname + "_" + _wall_boundaries[bc_ind], params);
      }
    }
    else if (_momentum_wall_types[bc_ind] == "slip")
    {
      const std::string bc_type = "INSFVNaturalFreeSlipBC";
      InputParameters params = _factory.getValidParams(bc_type);
      params.set<std::vector<BoundaryName>>("boundary") = {_wall_boundaries[bc_ind]};

      for (unsigned int d = 0; d < _dim; ++d)
      {
        NonlinearVariableName vname;
        if (_porous_medium_treatment)
        {
          vname = NS::superficial_velocity_vector[d];
          params.set<UserObjectName>("rhie_chow_user_object") = "pins_rhie_chow_interpolator";
        }
        else
        {
          vname = NS::velocity_vector[d];
          params.set<UserObjectName>("rhie_chow_user_object") = "ins_rhie_chow_interpolator";
        }

        params.set<NonlinearVariableName>("variable") = vname;
        params.set<MooseEnum>("momentum_component") = NS::directions[d];

        _problem->addFVBC(bc_type, vname + "_" + _wall_boundaries[bc_ind], params);
      }
    }
    else if (_momentum_wall_types[bc_ind] == "symmetry")
    {
      {
        std::string bc_type;
        if (_porous_medium_treatment)
          bc_type = "PINSFVSymmetryVelocityBC";
        else
          bc_type = "INSFVSymmetryVelocityBC";

        InputParameters params = _factory.getValidParams(bc_type);
        params.set<std::vector<BoundaryName>>("boundary") = {_wall_boundaries[bc_ind]};

        MaterialPropertyName viscosity_name = _dynamic_viscosity_name;
        if (_turbulence_handling != "none")
          viscosity_name = NS::total_viscosity;
        params.set<MaterialPropertyName>(NS::mu) = viscosity_name;

        for (unsigned int d = 0; d < _dim; ++d)
        {
          NonlinearVariableName vname;
          if (_porous_medium_treatment)
          {
            vname = NS::superficial_velocity_vector[d];
            for (unsigned int d = 0; d < _dim; ++d)
              params.set<MooseFunctorName>(u_names[d]) = NS::superficial_velocity_vector[d];
            params.set<UserObjectName>("rhie_chow_user_object") = "pins_rhie_chow_interpolator";
          }
          else
          {
            vname = NS::velocity_vector[d];
            for (unsigned int d = 0; d < _dim; ++d)
              params.set<MooseFunctorName>(u_names[d]) = NS::velocity_vector[d];
            params.set<UserObjectName>("rhie_chow_user_object") = "ins_rhie_chow_interpolator";
          }

          params.set<NonlinearVariableName>("variable") = vname;
          params.set<MooseEnum>("momentum_component") = NS::directions[d];

          _problem->addFVBC(bc_type, vname + "_" + _wall_boundaries[bc_ind], params);
        }
      }
      {
        const std::string bc_type = "INSFVSymmetryPressureBC";
        InputParameters params = _factory.getValidParams(bc_type);
        params.set<NonlinearVariableName>("variable") = NS::pressure;
        params.set<std::vector<BoundaryName>>("boundary") = {_wall_boundaries[bc_ind]};
        _problem->addFVBC(bc_type, NS::pressure + "_" + _wall_boundaries[bc_ind], params);
      }
    }
    else
      mooseError(_momentum_wall_types[bc_ind] +
                 " wall BC is not supported for INS simulations at the moment!");

    if (_has_energy_equation)
    {
      if (_energy_wall_types[bc_ind] == "fixed-temperature")
      {
        const std::string bc_type = "FVFunctionDirichletBC";
        InputParameters params = _factory.getValidParams(bc_type);
        params.set<NonlinearVariableName>("variable") = NS::T_fluid;
        params.set<FunctionName>("function") = _energy_wall_function[bc_ind];
        params.set<std::vector<BoundaryName>>("boundary") = {_wall_boundaries[bc_ind]};
        _problem->addFVBC(bc_type, NS::T_fluid + "_" + _wall_boundaries[bc_ind], params);
      }
      else if (_energy_wall_types[bc_ind] == "heatflux")
      {
        const std::string bc_type = "FVFunctionNeumannBC";
        InputParameters params = _factory.getValidParams(bc_type);
        params.set<NonlinearVariableName>("variable") = NS::T_fluid;
        params.set<FunctionName>("function") = _energy_wall_function[bc_ind];
        params.set<std::vector<BoundaryName>>("boundary") = {_wall_boundaries[bc_ind]};
        _problem->addFVBC(bc_type, NS::T_fluid + "_" + _wall_boundaries[bc_ind], params);
      }
      else if (_energy_wall_types[bc_ind] == "symmetry")
      {
        const std::string bc_type = "INSFVSymmetryBC";
        InputParameters params = _factory.getValidParams(bc_type);
        params.set<NonlinearVariableName>("variable") = NS::T_fluid;
        params.set<std::vector<BoundaryName>>("boundary") = {_wall_boundaries[bc_ind]};
        _problem->addFVBC(bc_type, NS::T_fluid + "_" + _wall_boundaries[bc_ind], params);
      }
      else
        mooseError(
            _energy_wall_types[bc_ind] +
            " wall BC is not supported for for energy equation in INS simulations at the moment!");
    }
  }
}

void
NSFVAction::addEnthalpyMaterial()
{
  InputParameters params = _factory.getValidParams("INSFVEnthalpyMaterial");
  params.set<std::vector<SubdomainName>>("block") = _blocks;

  params.set<MooseFunctorName>(NS::density) = _density_name;
  params.set<MooseFunctorName>("temperature") = NS::T_fluid;
  _problem->addMaterial("INSFVEnthalpyMaterial", "ins_enthalpy_material", params);
}

void
NSFVAction::addMixingLengthMaterial()
{
  const std::string u_names[3] = {"u", "v", "w"};
  InputParameters params = _factory.getValidParams("MixingLengthTurbulentViscosityMaterial");
  params.set<std::vector<SubdomainName>>("block") = _blocks;

  if (_porous_medium_treatment)
    for (unsigned int d = 0; d < _dim; ++d)
      params.set<CoupledName>(u_names[d]) = {NS::superficial_velocity_vector[d]};
  else
    for (unsigned int d = 0; d < _dim; ++d)
      params.set<CoupledName>(u_names[d]) = {NS::velocity_vector[d]};
  params.set<CoupledName>(NS::mixing_length) = {NS::mixing_length};

  params.set<MaterialPropertyName>(NS::density) = _density_name;
  params.set<MaterialPropertyName>(NS::mu) = _dynamic_viscosity_name;

  _problem->addMaterial("MixingLengthTurbulentViscosityMaterial", "mixing_length_material", params);
}

void
NSFVAction::addRelationshipManager(std::string name,
                                   unsigned int no_layers,
                                   const InputParameters & obj_params)
{
  auto & factory = _app.getFactory();
  auto rm_params = factory.getValidParams("ElementSideNeighborLayers");

  rm_params.set<std::string>("for_whom") = name;
  rm_params.set<MooseMesh *>("mesh") = _mesh.get();
  rm_params.set<Moose::RelationshipManagerType>("rm_type") =
      Moose::RelationshipManagerType::GEOMETRIC | Moose::RelationshipManagerType::ALGEBRAIC |
      Moose::RelationshipManagerType::COUPLING;
  rm_params.set<unsigned short>("layers") = no_layers;
  rm_params.set<bool>("attach_geometric_early") = false;
  if (obj_params.isParamValid("use_point_neighbors"))
    rm_params.set<bool>("use_point_neighbors") = obj_params.get<bool>("use_point_neighbors");

  rm_params.set<bool>("use_displaced_mesh") = obj_params.get<bool>("use_displaced_mesh");
  mooseAssert(rm_params.areAllRequiredParamsValid(),
              "All relationship manager parameters should be valid.");

  auto rm_obj = factory.create<RelationshipManager>("ElementSideNeighborLayers", name, rm_params);

  if (!_app.addRelationshipManager(rm_obj))
    factory.releaseSharedObjects(*rm_obj);
}

void
NSFVAction::processBlocks()
{
  _dim = _mesh->dimension();
  _problem->needFV();

  for (const auto & subdomain_name : _blocks)
  {
    SubdomainID id = _mesh->getSubdomainID(subdomain_name);
    _block_ids.insert(id);
    if (_problem->getCoordSystem(id) != Moose::COORD_XYZ)
      mooseError("RZ has not been added in action");
  }
  if (_blocks.size() == 0)
  {
    for (auto & id : _mesh->meshSubdomains())
      if (_problem->getCoordSystem(id) != Moose::COORD_XYZ)
        mooseError("RZ has not been added in action");
  }

  if (_momentum_inlet_function.size() != _inlet_boundaries.size() * _dim)
    paramError("momentum_inlet_function",
               "Size is not the same as the number of boundaries in 'inlet_boundaries' times "
               "the mesh dimension");
}

void
NSFVAction::checkGeneralControErrors()
{
  if (isParamValid("add_energy_equation"))
    if (_compressibility == "compressible" && !_has_energy_equation)
      paramError("add_energy_equation",
                 "The user must have an energy equation for compressible simulation! Either delete "
                 "the parameter or set it to 'true'!");

  if ((_compressibility == "weakly-compressible" || _compressibility == "compressible") &&
      _boussinesq_approximation == true)
    paramError("boussinesq_approximation",
               "We cannot use boussinesq approximation while running in compressible or "
               "weakly-compressible modes!");
}

void
NSFVAction::checkBoundaryParameterErrors()
{
  unsigned int no_pressure_outlets = 0;
  for (unsigned int enum_ind = 0; enum_ind < _outlet_boundaries.size(); ++enum_ind)
    if (_momentum_outlet_types[enum_ind] == "fixed-pressure" ||
        _momentum_outlet_types[enum_ind] == "fixed-pressure-zero-gradient")
      no_pressure_outlets += 1;

  if (_outlet_boundaries.size() > 0 && _pressure_function.size() != no_pressure_outlets)
    paramError("pressure_function",
               "Size is not the same as the number of pressure outlet boundaries!");

  if (_compressibility == "incompressible")
    if (no_pressure_outlets == 0 && !(getParam<bool>("pin_pressure")))
      mooseError("The pressure must be fixed for an incompressible simulation! Try setting "
                 "pin_pressure or change the compressibility settings!");

  if (_outlet_boundaries.size() > 0 && _outlet_boundaries.size() != _momentum_outlet_types.size())
    paramError("velocity_outlet_types",
               "Size is not the same as the number of outlet boundaries in 'outlet_boundaries'");

  if (_wall_boundaries.size() > 0 && _wall_boundaries.size() != _momentum_wall_types.size())
    paramError("velocity_wall_types",
               "Size is not the same as the number of wall boundaries in 'wall_boundaries'");

  if (_has_energy_equation)
  {
    if (_inlet_boundaries.size() > 0 && _energy_inlet_types.size() != _energy_inlet_function.size())
      paramError("energy_inlet_function",
                 "Size is not the same as the number of boundaries in 'energy_inlet_types'");

    unsigned int no_fixed_energy_walls = 0;
    for (unsigned int enum_ind = 0; enum_ind < _energy_wall_types.size(); ++enum_ind)
      if (_energy_wall_types[enum_ind] == "fixed-temperature" ||
          _energy_wall_types[enum_ind] == "heatflux")
        no_fixed_energy_walls += 1;

    if (_wall_boundaries.size() > 0 && _energy_wall_function.size() != no_fixed_energy_walls)
      paramError("energy_wall_function",
                 "Size " + std::to_string(_energy_wall_function.size()) +
                     " is not the same as the number of Dirichlet/Neumann conditions in "
                     "'energy_wall_types' (" +
                     std::to_string(no_fixed_energy_walls) + ")");

    if (_inlet_boundaries.size() > 0 && _inlet_boundaries.size() != _energy_inlet_types.size())
      paramError("energy_inlet_types",
                 "Size is not the same as the number of inlet boundaries in 'inlet_boundaries'");

    if (_wall_boundaries.size() > 0 && _wall_boundaries.size() != _energy_wall_types.size())
      paramError("energy_wall_types",
                 "Size is not the same as the number of wall boundaries in 'wall_boundaries'");
  }
}

void
NSFVAction::checkAmbientConvectionParameterErrors()
{
  if (_has_energy_equation)
  {
    if (_ambient_convection_blocks.size() && _blocks.size())
    {
      for (auto const & block : _ambient_convection_blocks)
        if (std::find(_blocks.begin(), _blocks.end(), block) == _blocks.end())
          paramError("ambient_convection_blocks",
                     "Block '" + block + "' is not present in the block IDs of the module!");

      if (_ambient_convection_blocks.size() != _ambient_convection_alpha.size())
        paramError("ambient_convection_alpha",
                   "The number of heat exchange coefficients is not the same as the number of "
                   "ambient convection blocks!");

      if (_ambient_convection_blocks.size() != _ambient_temperature.size())
        paramError("ambient_temperature",
                   "The number of ambient temperatures is not the same as the number of "
                   "ambient convection blocks!");
    }
    else if ((!_ambient_convection_blocks.size() && !_blocks.size()) ||
             (!_ambient_convection_blocks.size() && _blocks.size()))
    {
      if (_ambient_convection_alpha.size() > 1)
        paramError("ambient_convection_alpha",
                   "The user should only use one or zero heat exchange coefficient if the ambient "
                   "convection blocks are not defined!");
      if (_ambient_convection_alpha.size() != _ambient_temperature.size())
        paramError("ambient_temperature",
                   "The number of ambient temperatures is not the same as the number of "
                   "heat exchange coefficients!");
    }
    else if (_ambient_convection_blocks.size() && !_blocks.size())
      paramError("ambient_convection_blocks",
                 "If there are no subdomains defined in 'blocks', ambient convection blocks should "
                 "not be defined either!");
  }
}

void
NSFVAction::checkFrictionParameterErrors()
{
  if (_friction_blocks.size() && _blocks.size())
  {
    for (auto const & block : _friction_blocks)
      if (std::find(_blocks.begin(), _blocks.end(), block) == _blocks.end())
        paramError("friction_blocks",
                   "Block '" + block + "' is not present in the block IDs of the module!");

    if (_friction_blocks.size() != _friction_types.size())
      paramError("friction_types",
                 "The number of zones defined is not equal with the number of zones in "
                 "'friction_blocks'!");

    if (_friction_blocks.size() != _friction_coeffs.size())
      paramError("friction_coeffs",
                 "The number of zones defined is not equal with the number of zones in "
                 "'friction_blocks'!");
  }
  else if ((!_friction_blocks.size() && !_blocks.size()) ||
           (!_friction_blocks.size() && _blocks.size()))
  {
    if (_friction_types.size() > 1)
      paramError("friction_types",
                 "The user should only use one or zero zones in the friction type definition!");
    if (_friction_coeffs.size() != _friction_types.size())
      paramError("friction_coeffs",
                 "The number of zones for the friction coefficients should also be one or zero!");
  }
  else if (_friction_blocks.size() && !_blocks.size())
    paramError("friction_blocks",
               "If there are no subdomains defined in 'blocks', friction blocks should "
               "not be defined either!");

  for (unsigned int block_i = 0; block_i < _friction_blocks.size(); ++block_i)
    if (_friction_types[block_i].size() != _friction_coeffs[block_i].size())
    {
      paramError("friction_coeffs",
                 "The number of friction coefficients for block: " + _friction_blocks[block_i] +
                     " is not the same as the number of requested friction types!");
    }

  for (unsigned int block_i = 0; block_i < _friction_types.size(); ++block_i)
  {
    MultiMooseEnum ft("darcy forchheimer");
    ft.push_back(_friction_types[block_i]);

    for (const auto & name : ft.getNames())
    {
      unsigned int c = std::count(ft.begin(), ft.end(), name);
      if (c > 1)
        paramError("friction_types",
                   "The following keyword: " + name + " appeared more than once in block " +
                       std::to_string(block_i) + " of 'friction_types'.");
    }
  }
}
