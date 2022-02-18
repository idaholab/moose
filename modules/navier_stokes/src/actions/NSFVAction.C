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
  params.addClassDescription(
      "This class allows us to have a section of the input file for "
      "setting up Navier-Stokes equations for porous medium or clean fluid flows.");

  // General simulation control parameters
  MooseEnum sim_type("steady-state transient", "steady-state");
  params.addParam<MooseEnum>("simulation_type", sim_type, "Navier-Stokes equation type");

  MooseEnum comp_type("incompressible weakly-compressible compressible", "incompressible");
  params.addParam<MooseEnum>(
      "compressibility", comp_type, "Compressibility constraint for the Navier-Stokes equations.");

  params.addParam<bool>(
      "porous_medium_treatment", false, "Whether to use porous medium solvers or not.");

  params.addParam<AuxVariableName>(
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

  MultiMooseEnum en_inlet_type("fixed-temperature fixed-enthalpy fixed-totalenergy",
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

  // params.addCoupledVar("ambient_temperature",
  //                      "The ambient temperature for each block in 'ambient_convection_blocks'.");
  params.addParam<std::vector<MooseFunctorName>>(
      "ambient_temperature",
      std::vector<MooseFunctorName>(),
      "The ambient temperature for each block in 'ambient_convection_blocks'.");

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
    _porosity_name(getParam<AuxVariableName>("porosity")),
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
    _solid_temperature_variable_name(getParam<VariableName>("solid_temperature_variable")),
    _density_name(getParam<MaterialPropertyName>("density")),
    _dynamic_viscosity_name(getParam<MaterialPropertyName>("dynamic_viscosity")),
    _specific_heat_name(getParam<MaterialPropertyName>("specific_heat")),
    _thermal_conductivity_name(getParam<MaterialPropertyName>("thermal_conductivity")),
    _thermal_expansion_name(getParam<MaterialPropertyName>("thermal_expansion"))
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

  if (getParam<bool>("add_energy_equation"))
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

    if (_ambient_convection_blocks.size() && _blocks.size())
    {
      _ambient_convection_alpha =
          getParam<std::vector<MaterialPropertyName>>("ambient_convection_alpha");
      _ambient_temperature = getParam<std::vector<MooseFunctorName>>("ambient_temperature");

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
    else if (!_ambient_convection_blocks.size() && !_blocks.size())
    {
      _ambient_convection_alpha =
          getParam<std::vector<MaterialPropertyName>>("ambient_convection_alpha");
      _ambient_temperature = getParam<std::vector<MooseFunctorName>>("ambient_temperature");

      if (_ambient_convection_alpha.size() != 1)
        paramError("ambient_convection_alpha",
                   "The user should only use one heat exchange coefficient if the ambient "
                   "convection blocks are not defined!");

      if (_ambient_temperature.size() != 1)
        paramError("ambient_temperature",
                   "The user should only use one ambient temperature if the ambient "
                   "convection blocks are not defined!");
    }
    else if (_ambient_convection_blocks.size() && !_blocks.size())
      paramError("ambient_convection_blocks",
                 "If there are no subdomains defined in 'blocks', ambient convection blocks should "
                 "not be defined either!");

    if (getParam<bool>("has_heat_source"))
    {
      bool has_coupled = isParamValid("heat_source_var");
      bool has_function = isParamValid("heat_source_function");
      if (!has_coupled && !has_function)
        mooseError("Either the 'heat_source_var' or 'heat_source_function' param must be set.");
      else if (has_coupled && has_function)
        mooseError("Both the 'heat_source_var' or 'heat_source_function' param are set."
                   "Please use one or the other.");
    }
  }

  if (getParam<bool>("has_coupled_force"))
  {
    bool has_coupled = isParamValid("coupled_force_var");
    bool has_function = isParamValid("coupled_force_vector_function");
    if (!has_coupled && !has_function)
      mooseError(
          "Either the 'coupled_force_var' or 'coupled_force_vector_function' param must be set.");
  }
}

void
NSFVAction::act()
{
  if (_current_task == "add_navier_stokes_variables")
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

    auto base_params = _factory.getValidParams("MooseVariableFVReal");
    if (_block_ids.size() != 0)
      for (const SubdomainID & id : _block_ids)
        base_params.set<std::vector<SubdomainName>>("block").push_back(Moose::stringify(id));

    if (_compressibility == "compressible")
    {
      if (_porous_medium_treatment)
        for (unsigned int d = 0; d < _dim; ++d)
          _problem->addVariable(
              "MooseVariableFVReal", NS::superficial_momentum_vector[d], base_params);
      else
        for (unsigned int d = 0; d < _dim; ++d)
          _problem->addVariable("MooseVariableFVReal", NS::momentum_vector[d], base_params);

      _problem->addVariable("MooseVariableFVReal", NS::pressure, base_params);
      _problem->addVariable("MooseVariableFVReal", NS::T_fluid, base_params);

      for (unsigned int d = 0; d < _dim; ++d)
        _problem->addVariable("MooseVariableFVReal", NS::velocity_vector[d], base_params);
    }
    else if (_compressibility == "weakly-compressible" || _compressibility == "incompressible")
    {
      if (_porous_medium_treatment)
      {
        for (unsigned int d = 0; d < _dim; ++d)
          _problem->addVariable(
              "PINSFVSuperficialVelocityVariable", NS::superficial_velocity_vector[d], base_params);

        for (unsigned int d = 0; d < _dim; ++d)
          _problem->addVariable("INSFVVelocityVariable", NS::velocity_vector[d], base_params);
      }
      else
        for (unsigned int d = 0; d < _dim; ++d)
          _problem->addVariable("INSFVVelocityVariable", NS::velocity_vector[d], base_params);

      _problem->addVariable("INSFVPressureVariable", NS::pressure, base_params);
      if (getParam<bool>("pin_pressure"))
      {
        auto lm_params = _factory.getValidParams("MooseVariableScalar");
        lm_params.set<MooseEnum>("family") = "scalar";
        lm_params.set<MooseEnum>("order") = "first";
        _problem->addVariable("MooseVariableScalar", "lambda", lm_params);
      }

      if (getParam<bool>("add_energy_equation"))
        _problem->addVariable("INSFVEnergyVariable", NS::T_fluid, base_params);
    }
  }

  if (_current_task == "add_navier_stokes_user_objects")
  {
    if (_compressibility == "incompressible" || _compressibility == "weakly-compressible")
    {
      const std::string u_names[3] = {"u", "v", "w"};
      if (_porous_medium_treatment)
      {
        auto params = _factory.getValidParams("PINSFVRhieChowInterpolator");
        for (unsigned int d = 0; d < _dim; ++d)
          params.set<VariableName>(u_names[d]) = NS::superficial_velocity_vector[d];

        params.set<VariableName>("pressure") = NS::pressure;
        params.set<AuxVariableName>(NS::porosity) = _porosity_name;

        _problem->addUserObject(
            "PINSFVRhieChowInterpolator", "pins_rhie_chow_interpolator", params);
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
  }

  if (_current_task == "add_navier_stokes_ics")
  {
    Real pvalue = getParam<Real>("initial_pressure");

    if (_compressibility == "compressible")
    {
      auto mvalue = getParam<RealVectorValue>("initial_momentum");

      if (_porous_medium_treatment)
        for (unsigned int d = 0; d < _dim; ++d)
        {
          InputParameters params = _factory.getValidParams("ConstantIC");
          params.set<VariableName>("variable") = NS::superficial_momentum_vector[d];
          params.set<Real>("value") = mvalue(d);
          _problem->addInitialCondition(
              "ConstantIC", NS::superficial_momentum_vector[d] + "_ic", params);
        }
      else
        for (unsigned int d = 0; d < _dim; ++d)
        {
          InputParameters params = _factory.getValidParams("ConstantIC");
          params.set<VariableName>("variable") = NS::momentum_vector[d];
          params.set<Real>("value") = mvalue(d);
          _problem->addInitialCondition("ConstantIC", NS::momentum_vector[d] + "_ic", params);
        }

      Real tvalue = getParam<Real>("initial_temperature");
      InputParameters params = _factory.getValidParams("ConstantIC");
      params.set<VariableName>("variable") = NS::T_fluid;
      params.set<Real>("value") = tvalue;
      _problem->addInitialCondition("ConstantIC", NS::T_fluid + "_ic", params);
    }
    else if (_compressibility == "weakly-compressible" || _compressibility == "incompressible")
    {
      auto vvalue = getParam<RealVectorValue>("initial_velocity");

      if (_porous_medium_treatment)
        for (unsigned int d = 0; d < _dim; ++d)
        {
          InputParameters params = _factory.getValidParams("ConstantIC");
          params.set<VariableName>("variable") = NS::superficial_velocity_vector[d];
          params.set<Real>("value") = vvalue(d);
          _problem->addInitialCondition(
              "ConstantIC", NS::superficial_velocity_vector[d] + "_ic", params);
        }
      else
        for (unsigned int d = 0; d < _dim; ++d)
        {
          InputParameters params = _factory.getValidParams("ConstantIC");
          params.set<VariableName>("variable") = NS::velocity_vector[d];
          params.set<Real>("value") = vvalue(d);
          _problem->addInitialCondition("ConstantIC", NS::velocity_vector[d] + "_ic", params);
        }

      if (getParam<bool>("add_energy_equation"))
      {
        Real tvalue = getParam<Real>("initial_temperature");
        InputParameters params = _factory.getValidParams("ConstantIC");
        params.set<VariableName>("variable") = NS::T_fluid;
        params.set<Real>("value") = tvalue;
        _problem->addInitialCondition("ConstantIC", NS::T_fluid + "_ic", params);
      }
    }

    if (pvalue != 0)
    {
      InputParameters params = _factory.getValidParams("ConstantIC");
      params.set<VariableName>("variable") = NS::pressure;
      params.set<Real>("value") = pvalue;
      _problem->addInitialCondition("ConstantIC", "pressure_ic", params);
    }
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

      addINSMass();
      addINSMomentum();

      if (getParam<bool>("add_energy_equation"))
        addWCNSEnergy();
    }
    else if (_compressibility == "incompressible")
    {
      if (_type == "transient")
        addINSTimeKernels();

      addINSMass();
      addINSMomentum();

      if (getParam<bool>("add_energy_equation"))
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
      if (getParam<bool>("add_energy_equation"))
      {
        InputParameters params = _factory.getValidParams("INSFVEnthalpyMaterial");

        if (_blocks.size() > 0)
          params.set<std::vector<SubdomainName>>("block") = _blocks;

        params.set<MooseFunctorName>(NS::density) = _density_name;
        params.set<MooseFunctorName>("temperature") = NS::T_fluid;
        _problem->addMaterial("INSFVEnthalpyMaterial", "ins_enthalpy_material", params);
      }
    }
    else
    {
      mooseError("Compressible simulations are not supported yet.");
    }
  }
}

void
NSFVAction::addINSTimeKernels()
{
  if (_porous_medium_treatment)
  {
    const std::string mom_kernel_type = "PINSFVMomentumTimeDerivative";
    InputParameters params = _factory.getValidParams(mom_kernel_type);
    if (_blocks.size() > 0)
      params.set<std::vector<SubdomainName>>("block") = _blocks;
    params.set<MaterialPropertyName>(NS::density) = _density_name;

    for (unsigned int d = 0; d < _dim; ++d)
    {
      params.set<NonlinearVariableName>("variable") = NS::superficial_velocity_vector[d];
      _problem->addKernel(mom_kernel_type, "pins_momentum_" + NS::directions[d] + "_time", params);
    }

    if (getParam<bool>("add_energy_equation"))
    {
      const std::string en_kernel_type = "PINSFVEnergyTimeDerivative";
      params = _factory.getValidParams(en_kernel_type);
      if (_blocks.size() > 0)
        params.set<std::vector<SubdomainName>>("block") = _blocks;

      params.set<NonlinearVariableName>("variable") = NS::T_fluid;
      params.set<MooseFunctorName>(NS::porosity) = _porosity_name;
      params.set<MooseFunctorName>(NS::density) = _density_name;
      params.set<MooseFunctorName>(NS::time_deriv(NS::density)) = NS::time_deriv(_density_name);
      params.set<MooseFunctorName>(NS::cp) = _specific_heat_name;
      params.set<MooseFunctorName>(NS::time_deriv(NS::cp)) = NS::time_deriv(_specific_heat_name);
      _problem->addKernel(en_kernel_type, "pins_energy_time", params);
    }
  }
  else
  {
    const std::string mom_kernel_type = "INSFVMomentumTimeDerivative";
    InputParameters params = _factory.getValidParams(mom_kernel_type);
    if (_blocks.size() > 0)
      params.set<std::vector<SubdomainName>>("block") = _blocks;
    params.set<MaterialPropertyName>(NS::density) = _density_name;

    for (unsigned int d = 0; d < _dim; ++d)
    {
      params.set<NonlinearVariableName>("variable") = NS::velocity_vector[d];
      _problem->addKernel(mom_kernel_type, "ins_momentum_" + NS::directions[d] + "_time", params);
    }

    if (getParam<bool>("add_energy_equation"))
    {
      const std::string en_kernel_type = "INSFVEnergyTimeDerivative";
      params = _factory.getValidParams(en_kernel_type);
      if (_blocks.size() > 0)
        params.set<std::vector<SubdomainName>>("block") = _blocks;

      params.set<NonlinearVariableName>("variable") = NS::T_fluid;
      params.set<MooseFunctorName>(NS::density) = _density_name;
      params.set<MooseFunctorName>(NS::time_deriv(NS::density)) = NS::time_deriv(_density_name);
      params.set<MooseFunctorName>(NS::cp) = _specific_heat_name;
      params.set<MooseFunctorName>(NS::time_deriv(NS::cp)) = NS::time_deriv(_specific_heat_name);
      _problem->addKernel(en_kernel_type, "ins_energy_time", params);
    }
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
    if (getParam<bool>("add_energy_equation"))
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
    if (getParam<bool>("add_energy_equation"))
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
  if (_porous_medium_treatment)
  {
    _console << "last one to finish" << std::endl;
  }
  else
  {
    _console << "last one to finish" << std::endl;
  }
}

void
NSFVAction::addINSMass()
{
  if (_porous_medium_treatment)
  {
    const std::string kernel_type = "PINSFVMassAdvection";
    InputParameters params = _factory.getValidParams(kernel_type);
    if (_blocks.size() > 0)
      params.set<std::vector<SubdomainName>>("block") = _blocks;

    params.set<NonlinearVariableName>("variable") = NS::pressure;
    params.set<MooseFunctorName>(NS::porosity) = _porosity_name;
    params.set<MooseFunctorName>(NS::density) = _density_name;
    params.set<MooseEnum>("velocity_interp_method") = "rc";
    params.set<UserObjectName>("rhie_chow_user_object") = "pins_rhie_chow_interpolator";
    params.set<MooseEnum>("advected_interp_method") = "average";

    _problem->addFVKernel(kernel_type, "pins_mass_advection", params);
  }
  else
  {
    const std::string kernel_type = "INSFVMassAdvection";
    InputParameters params = _factory.getValidParams(kernel_type);
    if (_blocks.size() > 0)
      params.set<std::vector<SubdomainName>>("block") = _blocks;

    params.set<NonlinearVariableName>("variable") = NS::pressure;
    params.set<MooseFunctorName>(NS::density) = _density_name;
    params.set<MooseEnum>("velocity_interp_method") = "rc";
    params.set<UserObjectName>("rhie_chow_user_object") = "ins_rhie_chow_interpolator";
    params.set<MooseEnum>("advected_interp_method") = "average";

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
NSFVAction::addINSMomentum()
{
  if (_porous_medium_treatment)
  {
    {
      const std::string adv_kernel_type = "PINSFVMomentumAdvection";
      InputParameters params = _factory.getValidParams(adv_kernel_type);
      if (_blocks.size() > 0)
        params.set<std::vector<SubdomainName>>("block") = _blocks;

      params.set<MooseFunctorName>(NS::density) = _density_name;
      params.set<MooseFunctorName>(NS::porosity) = _porosity_name;
      params.set<MooseEnum>("velocity_interp_method") = "rc";
      params.set<UserObjectName>("rhie_chow_user_object") = "pins_rhie_chow_interpolator";
      params.set<MooseEnum>("advected_interp_method") = "average";

      for (unsigned int d = 0; d < _dim; ++d)
      {
        params.set<NonlinearVariableName>("variable") = NS::superficial_velocity_vector[d];
        params.set<MooseEnum>("momentum_component") = NS::directions[d];
        _problem->addFVKernel(
            adv_kernel_type, "pins_momentum_" + NS::directions[d] + "_advection", params);
      }
    }
    {
      const std::string diff_kernel_type = "PINSFVMomentumDiffusion";
      InputParameters params = _factory.getValidParams(diff_kernel_type);
      if (_blocks.size() > 0)
        params.set<std::vector<SubdomainName>>("block") = _blocks;

      params.set<UserObjectName>("rhie_chow_user_object") = "pins_rhie_chow_interpolator";

      params.set<MooseFunctorName>(NS::mu) = _dynamic_viscosity_name;
      params.set<MooseFunctorName>(NS::porosity) = _porosity_name;

      for (unsigned int d = 0; d < _dim; ++d)
      {
        params.set<NonlinearVariableName>("variable") = NS::superficial_velocity_vector[d];
        params.set<MooseEnum>("momentum_component") = NS::directions[d];
        _problem->addFVKernel(
            diff_kernel_type, "pins_momentum_" + NS::directions[d] + "_diffusion", params);
      }
    }
    {
      const std::string press_kernel_type = "PINSFVMomentumPressure";
      InputParameters params = _factory.getValidParams(press_kernel_type);
      if (_blocks.size() > 0)
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
    {
      const std::string grav_kernel_type = "PINSFVMomentumGravity";
      InputParameters params = _factory.getValidParams(grav_kernel_type);
      if (_blocks.size() > 0)
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
    }

    if (getParam<bool>("boussinesq_approximation"))
    {
      const std::string boussinesq_kernel_type = "PINSFVMomentumBoussinesq";
      InputParameters params = _factory.getValidParams(boussinesq_kernel_type);
      if (_blocks.size() > 0)
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
    if (_turbulence_handling == "mixing-length")
    {
      mooseError("Turbulence handling is not implemented yet!");
    }
  }
  else
  {
    {
      const std::string adv_kernel_type = "INSFVMomentumAdvection";
      InputParameters params = _factory.getValidParams(adv_kernel_type);
      if (_blocks.size() > 0)
        params.set<std::vector<SubdomainName>>("block") = _blocks;

      params.set<MooseFunctorName>(NS::density) = _density_name;
      params.set<MooseEnum>("velocity_interp_method") = "rc";
      params.set<UserObjectName>("rhie_chow_user_object") = "ins_rhie_chow_interpolator";
      params.set<MooseEnum>("advected_interp_method") = "average";

      for (unsigned int d = 0; d < _dim; ++d)
      {
        params.set<NonlinearVariableName>("variable") = NS::velocity_vector[d];
        params.set<MooseEnum>("momentum_component") = NS::directions[d];
        _problem->addFVKernel(
            adv_kernel_type, "ins_momentum_" + NS::directions[d] + "_advection", params);
      }
    }
    {
      const std::string diff_kernel_type = "INSFVMomentumDiffusion";
      InputParameters params = _factory.getValidParams(diff_kernel_type);
      if (_blocks.size() > 0)
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
    }
    {
      const std::string press_kernel_type = "INSFVMomentumPressure";
      InputParameters params = _factory.getValidParams(press_kernel_type);

      if (_blocks.size() > 0)
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
    {
      // const std::string grav_kernel_type = "INSFVMomentumGravity";
      // InputParameters params = _factory.getValidParams(grav_kernel_type);
      // if (_blocks.size() > 0)
      //   params.set<std::vector<SubdomainName>>("block") = _blocks;
      //
      // params.set<UserObjectName>("rhie_chow_user_object") = "ins_rhie_chow_interpolator";
      //
      // params.set<MooseFunctorName>(NS::density) = _density_name;
      // params.set<RealVectorValue>("gravity") = getParam<RealVectorValue>("gravity");
      //
      // for (unsigned int d = 0; d < _dim; ++d)
      // {
      //   params.set<MooseEnum>("momentum_component") = NS::directions[d];
      //   params.set<NonlinearVariableName>("variable") = NS::velocity_vector[d];
      //   _problem->addFVKernel(
      //       grav_kernel_type, "ins_momentum_" + NS::directions[d] + "_gravity", params);
      // }
    }

    if (getParam<bool>("boussinesq_approximation") && !(_compressibility == "weakly-compressible"))
    {
      // const std::string boussinesq_kernel_type = "INSFVMomentumBoussinesq";
      // InputParameters params = _factory.getValidParams(boussinesq_kernel_type);
      // if (_blocks.size() > 0)
      //   params.set<std::vector<SubdomainName>>("block") = _blocks;
      //
      // params.set<UserObjectName>("rhie_chow_user_object") = "ins_rhie_chow_interpolator";
      //
      // params.set<NonlinearVariableName>("variable") = NS::velocity_x;
      // params.set<MooseFunctorName>(NS::density) = _density_name;
      // params.set<RealVectorValue>("gravity") = getParam<RealVectorValue>("gravity");
      // params.set<Real>("ref_temperature") = getParam<Real>("ref_temperature");
      // params.set<MooseFunctorName>("alpha_name") = _thermal_expansion_name;
      //
      // for (unsigned int d = 0; d < _dim; ++d)
      // {
      //   params.set<MooseEnum>("momentum_component") = NS::directions[d];
      //   params.set<NonlinearVariableName>("variable") = NS::velocity_vector[d];
      //   _problem->addFVKernel(
      //       boussinesq_kernel_type, "ins_momentum_" + NS::directions[d] + "_boussinesq", params);
      // }
    }
    if (_turbulence_handling == "mixing-length")
    {
      mooseError("Turbulence handling is not implemented yet!");
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

      params.set<MooseFunctorName>(NS::porosity) = _porosity_name;
      params.set<MooseEnum>("velocity_interp_method") = "rc";
      params.set<UserObjectName>("rhie_chow_user_object") = "pins_rhie_chow_interpolator";
      params.set<MooseEnum>("advected_interp_method") = "average";

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
      params.set<MooseEnum>("advected_interp_method") = "average";

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
      const std::string kernel_type = "NSFVEnergyAmbientConvection";
      InputParameters params = _factory.getValidParams(kernel_type);
      params.set<NonlinearVariableName>("variable") = NS::T_fluid;
      params.set<std::vector<SubdomainName>>("block") = {_ambient_convection_blocks[block_i]};
      params.set<MaterialPropertyName>("alpha") = _ambient_convection_alpha[block_i];
      params.set<MooseFunctorName>("T_ambient") = _ambient_temperature[block_i];

      _problem->addFVKernel(
          kernel_type, "ambient_convection_" + _ambient_convection_blocks[block_i], params);
    }
  }
  else
  {
    const std::string kernel_type = "NSFVEnergyAmbientConvection";
    InputParameters params = _factory.getValidParams(kernel_type);
    params.set<NonlinearVariableName>("variable") = NS::T_fluid;
    params.set<std::vector<SubdomainName>>("block") = _ambient_convection_blocks;
    params.set<MaterialPropertyName>("alpha") = _ambient_convection_alpha[0];
    params.set<MooseFunctorName>("T_ambient") = _ambient_temperature[0];

    _problem->addFVKernel(kernel_type, "ambient_convection", params);
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

      for (unsigned int d = 0; d < _dim; ++d)
      {
        NonlinearVariableName vname;
        if (_porous_medium_treatment)
          vname = NS::superficial_velocity_vector[d];
        else
          vname = NS::velocity_vector[d];

        params.set<NonlinearVariableName>("variable") = vname;
        params.set<FunctionName>("function") = _momentum_inlet_function[bc_ind * _dim + d];
        params.set<std::vector<BoundaryName>>("boundary") = {_inlet_boundaries[bc_ind]};

        _problem->addFVBC(bc_type, vname + "_" + _inlet_boundaries[bc_ind], params);
      }
    }
    else
      mooseError(_momentum_inlet_types[bc_ind] +
                 " inlet BC is not supported for INS simulations at the moment!");

    if (getParam<bool>("add_energy_equation"))
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
        const std::string bc_type = "INSFVMomentumAdvectionOutflowBC";
        InputParameters params = _factory.getValidParams(bc_type);

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
          params.set<std::vector<BoundaryName>>("boundary") = {_outlet_boundaries[bc_ind]};

          _problem->addFVBC(bc_type, vname + "_" + _outlet_boundaries[bc_ind], params);
        }
      }
    }
    else if (_momentum_outlet_types[bc_ind] == "fixed-pressure-zero-gradient")
    {
      {
        const std::string bc_type = "INSFVMomentumAdvectionOutflowBC";
        InputParameters params = _factory.getValidParams(bc_type);

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
          params.set<std::vector<BoundaryName>>("boundary") = {_outlet_boundaries[bc_ind]};

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

      for (unsigned int d = 0; d < _dim; ++d)
      {
        NonlinearVariableName vname;
        if (_porous_medium_treatment)
          vname = NS::superficial_velocity_vector[d];
        else
          vname = NS::velocity_vector[d];

        params.set<NonlinearVariableName>("variable") = vname;
        params.set<std::vector<BoundaryName>>("boundary") = {_wall_boundaries[bc_ind]};
        params.set<FunctionName>("function") = "0";

        _problem->addFVBC(bc_type, vname + "_" + _wall_boundaries[bc_ind], params);
      }
    }
    else if (_momentum_wall_types[bc_ind] == "slip")
    {
      const std::string bc_type = "INSFVNaturalFreeSlipBC";
      InputParameters params = _factory.getValidParams(bc_type);

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
        params.set<std::vector<BoundaryName>>("boundary") = {_wall_boundaries[bc_ind]};

        _problem->addFVBC(bc_type, vname + "_" + _wall_boundaries[bc_ind], params);
      }
    }
    else if (_momentum_wall_types[bc_ind] == "symmetry")
    {
      {
        const std::string bc_type = "INSFVSymmetryVelocityBC";
        InputParameters params = _factory.getValidParams(bc_type);

        for (unsigned int d = 0; d < _dim; ++d)
        {
          NonlinearVariableName vname;
          if (_porous_medium_treatment)
          {
            vname = NS::superficial_velocity_vector[d];
            for (unsigned int d = 0; d < _dim; ++d)
              params.set<VariableName>(u_names[d]) = NS::superficial_velocity_vector[d];
            params.set<UserObjectName>("rhie_chow_user_object") = "pins_rhie_chow_interpolator";
          }
          else
          {
            vname = NS::velocity_vector[d];
            for (unsigned int d = 0; d < _dim; ++d)
              params.set<VariableName>(u_names[d]) = NS::velocity_vector[d];
            params.set<UserObjectName>("rhie_chow_user_object") = "ins_rhie_chow_interpolator";
          }

          params.set<NonlinearVariableName>("variable") = vname;
          params.set<MooseEnum>("momentum_component") = NS::directions[d];
          params.set<std::vector<BoundaryName>>("boundary") = {_wall_boundaries[bc_ind]};

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

    if (getParam<bool>("add_energy_equation"))
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
        mooseError(_energy_wall_types[bc_ind] +
                   " wall BC is not supported for INS simulations at the moment!");
    }
  }
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
