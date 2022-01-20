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
  params.addClassDescription("This class allows us to have a section of the input file for "
                             "setting up incompressible Navier-Stokes equations.");

  MooseEnum sim_type("steady-state transient", "steady-state");
  params.addParam<MooseEnum>("simulation_type", sim_type, "Navier-Stokes equation type");

  MooseEnum comp_type("incompressible weakly-compressible compressible", "steady-state");
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

  // temperature equation parameters
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

  params.addParam<RealVectorValue>(
      "gravity", RealVectorValue(0, 0, 0), "The gravitational acceleration vector.");

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

  params.addParam<std::vector<BoundaryName>>(
      "inlet_boundaries", std::vector<BoundaryName>(), "Names of inlet boundaries");
  params.addParam<std::vector<BoundaryName>>(
      "outlet_boundaries", std::vector<BoundaryName>(), "Names of outlet boundaries");
  params.addParam<std::vector<BoundaryName>>(
      "wall_boundaries", std::vector<BoundaryName>(), "Names of wall boundaries");

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

  addAmbientConvectionParams(params);

  params.addParam<unsigned int>(
      "pressure_pinned_dof",
      "The dof where pressure needs to be pinned for incompressible simulations.");

  params.addParam<Real>("initial_pressure", 0, "The initial pressure, assumed constant everywhere");

  // We perturb slightly from zero to avoid divide by zero exceptions from stabilization terms
  // involving a velocity norm in the denominator
  params.addParam<RealVectorValue>("initial_velocity",
                                   RealVectorValue(1e-15, 1e-15, 1e-15),
                                   "The initial velocity, assumed constant everywhere");
  params.addParam<RealVectorValue>("initial_momentum",
                                   RealVectorValue(1e-15, 1e-15, 1e-15),
                                   "The initial momentum, assumed constant everywhere");

  params.addParamNamesToGroup(
      "simulation_type block gravity dynamic_viscosity_name density_name boussinesq_approximation "
      "ref_temperature thermal_expansion_name",
      "Base");
  params.addParamNamesToGroup("inlet_boundary inlet_velocity_function pressure_pinned_node "
                              "pressure_boundary pressure_function",
                              "BoundaryCondition");
  params.addParamNamesToGroup("initial_pressure initial_velocity", "Variable");
  params.addParamNamesToGroup(
      "add_energy_equation temperature_variable initial_temperature "
      "thermal_conductivity_name specific_heat_name natural_temperature_boundary "
      "fixed_temperature_boundary temperature_function",
      "Temperature");
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
    _momentum_inlet_function(getParam<std::vector<FunctionName>>("velocity_inlet_function")),
    _momentum_outlet_types(getParam<MultiMooseEnum>("momentum_outlet_types")),
    _momentum_wall_types(getParam<MultiMooseEnum>("momentum_wall_types")),
    _energy_inlet_types(getParam<MultiMooseEnum>("energy_inlet_types")),
    _energy_inlet_function(getParam<std::vector<FunctionName>>("energy_inlet_function")),
    _energy_wall_types(getParam<MultiMooseEnum>("energy_wall_types")),
    _energy_wall_function(getParam<std::vector<FunctionName>>("energy_wall_function")),
    _pressure_function(getParam<std::vector<FunctionName>>("pressure_function")),
    _has_pinned_dof(isParamValid("pressure_pinned_dof")),
    _pinned_dof("ins_pinned_dof"),
    _solid_temperature_variable_name(getParam<VariableName>("solid_temperature_variable")),
    _density_name(getParam<MaterialPropertyName>("density")),
    _dynamic_viscosity_name(getParam<MaterialPropertyName>("dynamic_viscosity")),
    _specific_heat_name(getParam<MaterialPropertyName>("specific_heat")),
    _thermal_conductivity_name(getParam<MaterialPropertyName>("thermal_conductivity")),
    _thermal_expansion_name(getParam<MaterialPropertyName>("thermal_expansion"))
{
  unsigned int no_pressure_outlets = 0;
  for (unsigned int enum_ind = 0; enum_ind < _momentum_outlet_types.size(); ++enum_ind)
    if (_momentum_outlet_types[enum_ind] == "fixed-pressure")
      no_pressure_outlets += 1;

  if (_pressure_function.size() != no_pressure_outlets)
    paramError("pressure_function",
               "Size is not the same as the number of pressure outlet boundaries!");

  if (_outlet_boundaries.size() != _momentum_outlet_types.size())
  {
    paramError("velocity_outlet_types",
               "Size is not the same as the number of outlet boundaries in 'outlet_boundaries'");
  }

  if (_wall_boundaries.size() != _momentum_wall_types.size())
  {
    paramError("velocity_wall_types",
               "Size is not the same as the number of wall boundaries in 'wall_boundaries'");
  }

  if (_energy_inlet_types.size() != _energy_inlet_function.size())
    paramError("energy_inlet_function",
               "Size is not the same as the number of boundaries in 'energy_inlet_types'");

  unsigned int no_fixed_energy_walls = 0;
  for (unsigned int enum_ind = 0; enum_ind < _energy_wall_types.size(); ++enum_ind)
    if (_energy_wall_types[enum_ind] == "fixed-temperature" ||
        _energy_wall_types[enum_ind] == "heatflux")
      no_fixed_energy_walls += 1;

  if (_energy_wall_function.size() != no_fixed_energy_walls)
    paramError("energy_wall_function",
               "Size " + std::to_string(_energy_wall_function.size()) +
                   " is not the same as the number of Dirichlet/Neumann conditions in "
                   "'energy_wall_types' (" +
                   std::to_string(no_fixed_energy_walls) + ")");

  if (_inlet_boundaries.size() != _energy_inlet_types.size())
  {
    paramError("energy_inlet_types",
               "Size is not the same as the number of inlet boundaries in 'inlet_boundaries'");
  }

  if (_wall_boundaries.size() != _energy_wall_types.size())
  {
    paramError("energy_wall_types",
               "Size is not the same as the number of wall boundaries in 'wall_boundaries'");
  }

  if (getParam<bool>("has_ambient_convection"))
  {
    if (!isParamValid("ambient_convection_alpha"))
      mooseError(
          "If 'has_ambient_convection' is true, then 'ambient_convection_alpha' must be set.");

    if (!isParamValid("ambient_temperature"))
      mooseError("If 'has_ambient_convection' is true, then 'ambient_temperature' must be set.");
  }

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
      paramError("velocity_inlet_function",
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
          params.set<NonlinearVariableName>(u_names[d]) = NS::superficial_velocity_vector[d];

        params.set<NonlinearVariableName>("pressure") = NS::pressure;
        params.set<AuxVariableName>(NS::porosity) = _porosity_name;

        _problem->addUserObject(
            "PINSFVRhieChowInterpolator", "pins_rhie_chow_interpolator", params);
      }
      else
      {
        auto params = _factory.getValidParams("INSFVRhieChowInterpolator");
        for (unsigned int d = 0; d < _dim; ++d)
          params.set<NonlinearVariableName>(u_names[d]) = NS::velocity_vector[d];

        params.set<NonlinearVariableName>("pressure") = NS::pressure;

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
    if (_compressibility == "incompressible")
    {
      InputParameters params = _factory.getValidParams("INSFVMaterial");
      if (_blocks.size() > 0)
        params.set<std::vector<SubdomainName>>("block") = _blocks;

      _problem->addMaterial("INSFVMaterial", "ins_material", params);
    }
    else
    {
      mooseError("Weakly-compressible and compressible simulations are not supported yet.");
    }

    //   auto set_common_parameters = [&](InputParameters & params)
    //   {
    //     if (_blocks.size() > 0)
    //       params.set<std::vector<SubdomainName>>("block") = _blocks;
    //     params.set<CoupledName>("velocity") = {NS::velocity};
    //     params.set<CoupledName>(NS::pressure) = {_pressure_variable_name};
    //     params.set<MaterialPropertyName>("mu_name") =
    //         getParam<MaterialPropertyName>("dynamic_viscosity_name");
    //     params.set<MaterialPropertyName>("rho_name") =
    //     getParam<MaterialPropertyName>("density_name");
    //   };
    //
    //   auto set_common_3eqn_parameters = [&](InputParameters & params)
    //   {
    //     set_common_parameters(params);
    //     params.set<CoupledName>("temperature") = {_temperature_variable_name};
    //     params.set<MaterialPropertyName>("cp_name") =
    //         getParam<MaterialPropertyName>("specific_heat_name");
    //   };
    //
    //   if (getParam<bool>("add_temperature_equation"))
    //   {
    //     if (getParam<bool>("supg") || getParam<bool>("pspg"))
    //     {
    //       InputParameters params = _factory.getValidParams("INSADStabilized3Eqn");
    //       set_common_3eqn_parameters(params);
    //       params.set<Real>("alpha") = getParam<Real>("alpha");
    //       params.set<MaterialPropertyName>("k_name") =
    //           getParam<MaterialPropertyName>("thermal_conductivity_name");
    //       _problem->addMaterial("INSADStabilized3Eqn", "ins_ad_material", params);
    //     }
    //     else
    //     {
    //       InputParameters params = _factory.getValidParams("INSAD3Eqn");
    //       set_common_3eqn_parameters(params);
    //       _problem->addMaterial("INSAD3Eqn", "ins_ad_material", params);
    //     }
    //   }
    //   else
    //   {
    //     if (getParam<bool>("supg") || getParam<bool>("pspg"))
    //     {
    //       InputParameters params = _factory.getValidParams("INSADTauMaterial");
    //       set_common_parameters(params);
    //       params.set<Real>("alpha") = getParam<Real>("alpha");
    //       _problem->addMaterial("INSADTauMaterial", "ins_ad_material", params);
    //     }
    //     else
    //     {
    //       InputParameters params = _factory.getValidParams("INSADMaterial");
    //       set_common_parameters(params);
    //       _problem->addMaterial("INSADMaterial", "ins_ad_material", params);
    //     }
    //   }
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
      params.set<AuxVariableName>(NS::porosity) = _porosity_name;
      params.set<MaterialPropertyName>(NS::density) = _density_name;
      params.set<MaterialPropertyName>(NS::time_deriv(NS::density)) = NS::time_deriv(_density_name);
      params.set<MaterialPropertyName>(NS::cp) = _specific_heat_name;
      params.set<MaterialPropertyName>(NS::time_deriv(NS::cp)) =
          NS::time_deriv(_specific_heat_name);
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
      params.set<MaterialPropertyName>(NS::density) = _density_name;
      params.set<MaterialPropertyName>(NS::time_deriv(NS::density)) = NS::time_deriv(_density_name);
      params.set<MaterialPropertyName>(NS::cp) = _specific_heat_name;
      params.set<MaterialPropertyName>(NS::time_deriv(NS::cp)) =
          NS::time_deriv(_specific_heat_name);
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
      params.set<AuxVariableName>(NS::porosity) = _porosity_name;
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
      params.set<AuxVariableName>(NS::porosity) = _porosity_name;
      params.set<MaterialPropertyName>(NS::density) = _density_name;
      params.set<MaterialPropertyName>(NS::time_deriv(NS::density)) = NS::time_deriv(_density_name);
      params.set<MaterialPropertyName>(NS::cp) = _specific_heat_name;
      params.set<MaterialPropertyName>(NS::time_deriv(NS::cp)) =
          NS::time_deriv(_specific_heat_name);
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
      params.set<MaterialPropertyName>(NS::density) = _density_name;
      params.set<MaterialPropertyName>(NS::time_deriv(NS::density)) = NS::time_deriv(_density_name);
      params.set<MaterialPropertyName>(NS::cp) = _specific_heat_name;
      params.set<MaterialPropertyName>(NS::time_deriv(NS::cp)) =
          NS::time_deriv(_specific_heat_name);
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
  const std::string u_names[3] = {"u", "v", "w"};
  if (_porous_medium_treatment)
  {
    const std::string kernel_type = "PINSFVMassAdvection";
    InputParameters params = _factory.getValidParams(kernel_type);
    if (_blocks.size() > 0)
      params.set<std::vector<SubdomainName>>("block") = _blocks;

    params.set<NonlinearVariableName>("variable") = NS::pressure;
    params.set<AuxVariableName>(NS::porosity) = _porosity_name;
    params.set<MaterialPropertyName>(NS::density) = _density_name;
    params.set<MooseEnum>("velocity_interp_method") = "pins_rhie_chow_interpolator";
    params.set<MooseEnum>("advected_interp_method") = "average";

    for (unsigned int d = 0; d < _dim; ++d)
      params.set<NonlinearVariableName>(u_names[d]) = NS::superficial_velocity_vector[d];

    _problem->addKernel(kernel_type, "pins_mass_advection", params);
  }
  else
  {
    const std::string kernel_type = "INSFVMassAdvection";
    InputParameters params = _factory.getValidParams(kernel_type);
    if (_blocks.size() > 0)
      params.set<std::vector<SubdomainName>>("block") = _blocks;

    params.set<NonlinearVariableName>("variable") = NS::pressure;
    params.set<MaterialPropertyName>(NS::density) = _density_name;
    params.set<MooseEnum>("velocity_interp_method") = "ins_rhie_chow_interpolator";
    params.set<MooseEnum>("advected_interp_method") = "average";

    for (unsigned int d = 0; d < _dim; ++d)
      params.set<NonlinearVariableName>(u_names[d]) = NS::velocity_vector[d];

    _problem->addKernel(kernel_type, "ins_mass_advection", params);
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
  const std::string u_names[3] = {"u", "v", "w"};
  if (_porous_medium_treatment)
  {
    {
      const std::string adv_kernel_type = "PINSFVMomentumAdvection";
      InputParameters params = _factory.getValidParams(adv_kernel_type);
      if (_blocks.size() > 0)
        params.set<std::vector<SubdomainName>>("block") = _blocks;

      params.set<MaterialPropertyName>(NS::density) = _density_name;
      params.set<AuxVariableName>(NS::porosity) = _porosity_name;
      params.set<MooseEnum>("velocity_interp_method") = "pins_rhie_chow_interpolator";
      params.set<MooseEnum>("advected_interp_method") = "average";
      for (unsigned int d = 0; d < _dim; ++d)
        params.set<NonlinearVariableName>(u_names[d]) = NS::superficial_velocity_vector[d];

      for (unsigned int d = 0; d < _dim; ++d)
      {
        params.set<NonlinearVariableName>("variable") = NS::superficial_velocity_vector[d];
        _problem->addKernel(
            adv_kernel_type, "pins_momentum_" + NS::directions[d] + "_advection", params);
      }
    }
    {
      const std::string diff_kernel_type = "PINSFVMomentumDiffusion";
      InputParameters params = _factory.getValidParams(diff_kernel_type);
      if (_blocks.size() > 0)
        params.set<std::vector<SubdomainName>>("block") = _blocks;

      params.set<MaterialPropertyName>(NS::mu) = _dynamic_viscosity_name;
      params.set<AuxVariableName>(NS::porosity) = _porosity_name;

      for (unsigned int d = 0; d < _dim; ++d)
      {
        params.set<NonlinearVariableName>("variable") = NS::superficial_velocity_vector[d];
        _problem->addKernel(
            diff_kernel_type, "pins_momentum_" + NS::directions[d] + "_diffusion", params);
      }
    }
    {
      const std::string press_kernel_type = "PINSFVMomentumPressure";
      InputParameters params = _factory.getValidParams(press_kernel_type);
      if (_blocks.size() > 0)
        params.set<std::vector<SubdomainName>>("block") = _blocks;

      params.set<AuxVariableName>(NS::porosity) = _porosity_name;
      params.set<NonlinearVariableName>("pressure") = NS::pressure;

      for (unsigned int d = 0; d < _dim; ++d)
      {
        params.set<NonlinearVariableName>("variable") = NS::superficial_velocity_vector[d];
        _problem->addKernel(
            press_kernel_type, "pins_momentum_" + NS::directions[d] + "_pressure", params);
      }
    }
    {
      const std::string grav_kernel_type = "PINSFVMomentumGravity";
      InputParameters params = _factory.getValidParams(grav_kernel_type);
      if (_blocks.size() > 0)
        params.set<std::vector<SubdomainName>>("block") = _blocks;

      params.set<AuxVariableName>(NS::porosity) = _porosity_name;
      params.set<MaterialPropertyName>(NS::density) = _density_name;
      params.set<RealVectorValue>("gravity") = getParam<RealVectorValue>("gravity");

      for (unsigned int d = 0; d < _dim; ++d)
      {
        params.set<MooseEnum>("momentum_component") = NS::directions[d];
        params.set<NonlinearVariableName>("variable") = NS::superficial_velocity_vector[d];
        _problem->addKernel(
            grav_kernel_type, "pins_momentum_" + NS::directions[d] + "_gravity", params);
      }
    }

    if (getParam<bool>("boussinesq_approximation"))
    {
      const std::string boussinesq_kernel_type = "PINSFVMomentumBoussinesq";
      InputParameters params = _factory.getValidParams(boussinesq_kernel_type);
      if (_blocks.size() > 0)
        params.set<std::vector<SubdomainName>>("block") = _blocks;

      params.set<AuxVariableName>(NS::porosity) = _porosity_name;
      params.set<MaterialPropertyName>(NS::density) = _density_name;
      params.set<RealVectorValue>("gravity") = getParam<RealVectorValue>("gravity");
      params.set<Real>("ref_temperature") = getParam<Real>("ref_temperature");
      params.set<MaterialPropertyName>("alpha_name") = _thermal_expansion_name;

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

      params.set<MaterialPropertyName>(NS::density) = _density_name;
      params.set<MooseEnum>("velocity_interp_method") = "ins_rhie_chow_interpolator";
      params.set<MooseEnum>("advected_interp_method") = "average";
      for (unsigned int d = 0; d < _dim; ++d)
        params.set<NonlinearVariableName>(u_names[d]) = NS::velocity_vector[d];

      for (unsigned int d = 0; d < _dim; ++d)
      {
        params.set<NonlinearVariableName>("variable") = NS::velocity_vector[d];
        _problem->addKernel(
            adv_kernel_type, "ins_momentum_" + NS::directions[d] + "_advection", params);
      }
    }
    {
      const std::string diff_kernel_type = "INSFVMomentumDiffusion";
      InputParameters params = _factory.getValidParams(diff_kernel_type);
      if (_blocks.size() > 0)
        params.set<std::vector<SubdomainName>>("block") = _blocks;

      params.set<MaterialPropertyName>(NS::mu) = _dynamic_viscosity_name;
      params.set<AuxVariableName>(NS::porosity) = _porosity_name;

      for (unsigned int d = 0; d < _dim; ++d)
      {
        params.set<NonlinearVariableName>("variable") = NS::velocity_vector[d];
        _problem->addKernel(
            diff_kernel_type, "ins_momentum_" + NS::directions[d] + "_diffusion", params);
      }
    }
    {
      const std::string press_kernel_type = "INSFVMomentumPressure";
      InputParameters params = _factory.getValidParams(press_kernel_type);
      if (_blocks.size() > 0)
        params.set<std::vector<SubdomainName>>("block") = _blocks;

      params.set<NonlinearVariableName>("pressure") = NS::pressure;

      for (unsigned int d = 0; d < _dim; ++d)
      {
        params.set<NonlinearVariableName>("variable") = NS::velocity_vector[d];
        _problem->addKernel(
            press_kernel_type, "ins_momentum_" + NS::directions[d] + "_pressure", params);
      }
    }
    {
      const std::string grav_kernel_type = "INSFVMomentumGravity";
      InputParameters params = _factory.getValidParams(grav_kernel_type);
      if (_blocks.size() > 0)
        params.set<std::vector<SubdomainName>>("block") = _blocks;

      params.set<MaterialPropertyName>(NS::density) = _density_name;
      params.set<RealVectorValue>("gravity") = getParam<RealVectorValue>("gravity");

      for (unsigned int d = 0; d < _dim; ++d)
      {
        params.set<MooseEnum>("momentum_component") = NS::directions[d];
        params.set<NonlinearVariableName>("variable") = NS::velocity_vector[d];
        _problem->addKernel(
            grav_kernel_type, "ins_momentum_" + NS::directions[d] + "_gravity", params);
      }
    }

    if (getParam<bool>("boussinesq_approximation") && !(_compressibility == "weakly-compressible"))
    {
      const std::string boussinesq_kernel_type = "INSFVMomentumBoussinesq";
      InputParameters params = _factory.getValidParams(boussinesq_kernel_type);
      if (_blocks.size() > 0)
        params.set<std::vector<SubdomainName>>("block") = _blocks;

      params.set<NonlinearVariableName>("variable") = NS::velocity_x;
      params.set<MaterialPropertyName>(NS::density) = _density_name;
      params.set<RealVectorValue>("gravity") = getParam<RealVectorValue>("gravity");
      params.set<Real>("ref_temperature") = getParam<Real>("ref_temperature");
      params.set<MaterialPropertyName>("alpha_name") = _thermal_expansion_name;

      for (unsigned int d = 0; d < _dim; ++d)
      {
        params.set<MooseEnum>("momentum_component") = NS::directions[d];
        params.set<NonlinearVariableName>("variable") = NS::velocity_vector[d];
        _problem->addKernel(
            boussinesq_kernel_type, "ins_momentum_" + NS::directions[d] + "_boussinesq", params);
      }
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
  const std::string u_names[3] = {"u", "v", "w"};
  if (_porous_medium_treatment)
  {
    {
      const std::string kernel_type = "PINSFVEnergyAdvection";
      InputParameters params = _factory.getValidParams(kernel_type);
      params.set<NonlinearVariableName>("variable") = NS::T_fluid;
      if (_blocks.size() > 0)
        params.set<std::vector<SubdomainName>>("block") = _blocks;

      params.set<MaterialPropertyName>(NS::density) = _density_name;
      params.set<AuxVariableName>(NS::porosity) = _porosity_name;
      params.set<MooseEnum>("velocity_interp_method") = "pins_rhie_chow_interpolator";
      params.set<MooseEnum>("advected_interp_method") = "average";
      for (unsigned int d = 0; d < _dim; ++d)
        params.set<NonlinearVariableName>(u_names[d]) = NS::superficial_velocity_vector[d];

      _problem->addKernel(kernel_type, "pins_energy_advection", params);
    }
    {
      const std::string kernel_type = "PINSFVEnergyDiffusion";
      InputParameters params = _factory.getValidParams(kernel_type);
      params.set<NonlinearVariableName>("variable") = NS::T_fluid;
      if (_blocks.size() > 0)
        params.set<std::vector<SubdomainName>>("block") = _blocks;

      params.set<MaterialPropertyName>(NS::k) = _thermal_conductivity_name;
      params.set<AuxVariableName>(NS::porosity) = _porosity_name;

      _problem->addKernel(kernel_type, "pins_energy_diffusion", params);
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

      params.set<MaterialPropertyName>(NS::density) = _density_name;
      params.set<MooseEnum>("velocity_interp_method") = "ins_rhie_chow_interpolator";
      params.set<MooseEnum>("advected_interp_method") = "average";
      for (unsigned int d = 0; d < _dim; ++d)
        params.set<NonlinearVariableName>(u_names[d]) = NS::velocity_vector[d];

      _problem->addKernel(kernel_type, "ins_energy_convection", params);
    }
    {
      const std::string kernel_type = "PINSFVEnergyDiffusion";
      InputParameters params = _factory.getValidParams(kernel_type);
      params.set<NonlinearVariableName>("variable") = NS::T_fluid;
      if (_blocks.size() > 0)
        params.set<std::vector<SubdomainName>>("block") = _blocks;

      params.set<MaterialPropertyName>(NS::k) = _thermal_conductivity_name;

      _problem->addKernel(kernel_type, "ins_energy_diffusion", params);
    }
    if (_turbulence_handling == "mixing-length")
    {
      mooseError("Turbulence handling is not implemented yet!");
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

        _problem->addBoundaryCondition(bc_type, vname + "_" + _inlet_boundaries[bc_ind], params);
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
        _problem->addBoundaryCondition(
            bc_type, NS::T_fluid + "_" + _inlet_boundaries[bc_ind], params);
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
      _problem->addBoundaryCondition(
          bc_type, NS::pressure + "_" + _outlet_boundaries[bc_ind], params);
    }
    else if (_momentum_outlet_types[bc_ind] == "zero-gradient")
    {
      {
        const std::string bc_type = "INSFVMassAdvectionOutflowBC";
        InputParameters params = _factory.getValidParams(bc_type);
        params.set<NonlinearVariableName>("variable") = NS::pressure;
        params.set<MaterialPropertyName>(NS::density) = _density_name;
        params.set<std::vector<BoundaryName>>("boundary") = {_outlet_boundaries[bc_ind]};
        for (unsigned int d = 0; d < _dim; ++d)
        {
          if (_porous_medium_treatment)
            params.set<NonlinearVariableName>(u_names[d]) = NS::superficial_velocity_vector[d];
          else
            params.set<NonlinearVariableName>(u_names[d]) = NS::velocity_vector[d];
        }
        _problem->addBoundaryCondition(
            bc_type, NS::pressure + "_" + _outlet_boundaries[bc_ind], params);
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
              params.set<NonlinearVariableName>(u_names[d]) = NS::superficial_velocity_vector[d];
          }
          else
          {
            vname = NS::velocity_vector[d];
            for (unsigned int d = 0; d < _dim; ++d)
              params.set<NonlinearVariableName>(u_names[d]) = NS::velocity_vector[d];
          }

          params.set<NonlinearVariableName>("variable") = vname;
          params.set<MooseEnum>("momentum_component") = NS::directions[d];
          params.set<MaterialPropertyName>(NS::density) = _density_name;
          params.set<std::vector<BoundaryName>>("boundary") = {_outlet_boundaries[bc_ind]};

          _problem->addBoundaryCondition(bc_type, vname + "_" + _outlet_boundaries[bc_ind], params);
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
              params.set<NonlinearVariableName>(u_names[d]) = NS::superficial_velocity_vector[d];
          }
          else
          {
            vname = NS::velocity_vector[d];
            for (unsigned int d = 0; d < _dim; ++d)
              params.set<NonlinearVariableName>(u_names[d]) = NS::velocity_vector[d];
          }

          params.set<NonlinearVariableName>("variable") = vname;
          params.set<MooseEnum>("momentum_component") = NS::directions[d];
          params.set<MaterialPropertyName>(NS::density) = _density_name;
          params.set<std::vector<BoundaryName>>("boundary") = {_outlet_boundaries[bc_ind]};

          _problem->addBoundaryCondition(bc_type, vname + "_" + _outlet_boundaries[bc_ind], params);
        }
      }
      {
        const std::string bc_type = "INSFVOutletPressureBC";
        InputParameters params = _factory.getValidParams(bc_type);
        params.set<NonlinearVariableName>("variable") = NS::pressure;
        params.set<FunctionName>("function") = _pressure_function[bc_ind];
        params.set<std::vector<BoundaryName>>("boundary") = {_outlet_boundaries[bc_ind]};
        _problem->addBoundaryCondition(
            bc_type, NS::pressure + "_" + _outlet_boundaries[bc_ind], params);
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

        _problem->addBoundaryCondition(bc_type, vname + "_" + _wall_boundaries[bc_ind], params);
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
          vname = NS::superficial_velocity_vector[d];
        else
          vname = NS::velocity_vector[d];

        params.set<NonlinearVariableName>("variable") = vname;
        params.set<MooseEnum>("momentum_component") = NS::directions[d];
        params.set<std::vector<BoundaryName>>("boundary") = {_wall_boundaries[bc_ind]};

        _problem->addBoundaryCondition(bc_type, vname + "_" + _wall_boundaries[bc_ind], params);
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
              params.set<NonlinearVariableName>(u_names[d]) = NS::superficial_velocity_vector[d];
          }
          else
          {
            vname = NS::velocity_vector[d];
            for (unsigned int d = 0; d < _dim; ++d)
              params.set<NonlinearVariableName>(u_names[d]) = NS::velocity_vector[d];
          }

          params.set<NonlinearVariableName>("variable") = vname;
          params.set<MooseEnum>("momentum_component") = NS::directions[d];
          params.set<std::vector<BoundaryName>>("boundary") = {_wall_boundaries[bc_ind]};

          _problem->addBoundaryCondition(bc_type, vname + "_" + _wall_boundaries[bc_ind], params);
        }
      }
      {
        const std::string bc_type = "INSFVSymmetryPressureBC";
        InputParameters params = _factory.getValidParams(bc_type);
        params.set<NonlinearVariableName>("variable") = NS::pressure;
        params.set<std::vector<BoundaryName>>("boundary") = {_wall_boundaries[bc_ind]};
        _problem->addBoundaryCondition(
            bc_type, NS::pressure + "_" + _wall_boundaries[bc_ind], params);
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
        _problem->addBoundaryCondition(
            bc_type, NS::T_fluid + "_" + _wall_boundaries[bc_ind], params);
      }
      else if (_energy_wall_types[bc_ind] == "heatflux")
      {
        const std::string bc_type = "FVFunctionNeumannBC";
        InputParameters params = _factory.getValidParams(bc_type);
        params.set<NonlinearVariableName>("variable") = NS::T_fluid;
        params.set<FunctionName>("function") = _energy_wall_function[bc_ind];
        params.set<std::vector<BoundaryName>>("boundary") = {_wall_boundaries[bc_ind]};
        _problem->addBoundaryCondition(
            bc_type, NS::T_fluid + "_" + _wall_boundaries[bc_ind], params);
      }
      else if (_energy_wall_types[bc_ind] == "symmetry")
      {
        const std::string bc_type = "INSFVSymmetryBC";
        InputParameters params = _factory.getValidParams(bc_type);
        params.set<NonlinearVariableName>("variable") = NS::T_fluid;
        params.set<std::vector<BoundaryName>>("boundary") = {_wall_boundaries[bc_ind]};
        _problem->addBoundaryCondition(
            bc_type, NS::T_fluid + "_" + _wall_boundaries[bc_ind], params);
      }
      else
        mooseError(_energy_wall_types[bc_ind] +
                   " wall BC is not supported for INS simulations at the moment!");
    }
  }
}
