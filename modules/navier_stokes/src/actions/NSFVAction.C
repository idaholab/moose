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

registerMooseAction("NavierStokesApp", NSFVAction, "append_mesh_generator");
registerMooseAction("NavierStokesApp", NSFVAction, "add_navier_stokes_variables");
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
      "thermal_expansion_name", "alpha", "The name of the thermal expansion");
  params.addParam<bool>("add_energy_equation", false, "True to add energy equation");
  params.addParam<VariableName>(
      "fluid_temperature_variable", NS::T_fluid, "Fluid temperature variable name");
  params.addParam<VariableName>("solid_temperature_variable",
                                NS::T_solid,
                                "Solid subscale structure temperature variable name");
  params.addParam<Real>(
      "initial_temperature", 0, "The initial temperature, assumed constant everywhere");
  params.addParam<MaterialPropertyName>(
      "thermal_conductivity_name", "k", "The name of the thermal conductivity");
  params.addParam<MaterialPropertyName>(
      "specific_heat_name", "cp", "The name of the specific heat");
  params.addParam<std::vector<BoundaryName>>("natural_temperature_boundary",
                                             std::vector<BoundaryName>(),
                                             "Natural boundaries for temperature equation");
  params.addParam<std::vector<BoundaryName>>("fixed_temperature_boundary",
                                             std::vector<BoundaryName>(),
                                             "Dirichlet boundaries for temperature equation");
  params.addParam<std::vector<FunctionName>>(
      "temperature_function", std::vector<FunctionName>(), "Temperature on Dirichlet boundaries");
  addAmbientConvectionParams(params);
  params.addParam<bool>(
      "has_heat_source", false, "Whether there is a heat source function object in the simulation");
  params.addParam<FunctionName>("heat_source_function", "The function describing the heat source");
  params.addCoupledVar("heat_source_var", "The coupled variable describing the heat source");

  params.addParam<RealVectorValue>(
      "gravity", RealVectorValue(0, 0, 0), "Direction of the gravity vector");

  params.addParam<MaterialPropertyName>(
      "dynamic_viscosity_name", NS::mu, "The name of the dynamic viscosity");
  params.addParam<MaterialPropertyName>("density_name", "rho", "The name of the density");

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

  params.addParam<std::vector<FunctionName>>("velocity_inlet_function",
                                             std::vector<FunctionName>(),
                                             "Functions for inlet boundary velocities");
  MultiMooseEnum outlet_type("pressure mass-outflow momentum-outflow", "pressure");
  params.addParam<MultiMooseEnum>(
      "velocity_outlet_types", outlet_type, "Boundaries with given velocities");

  MultiMooseEnum wall_type("symmetry noslip slip wallfunction", "noslip");
  params.addParam<MultiMooseEnum>(
      "velocity_wall_types", outlet_type, "Boundaries with given velocities");

  params.addParam<unsigned int>(
      "pressure_pinned_dof",
      "The dof where pressure needs to be pinned for incompressible simulations.");

  params.addParam<std::vector<FunctionName>>("pressure_function",
                                             std::vector<FunctionName>(),
                                             "Functions for boundary pressures at outlets.");

  params.addParam<Real>("initial_pressure", 0, "The initial pressure, assumed constant everywhere");

  // We perturb slightly from zero to avoid divide by zero exceptions from stabilization terms
  // involving a velocity norm in the denominator
  params.addParam<RealVectorValue>("initial_velocity",
                                   RealVectorValue(1e-15, 1e-15, 1e-15),
                                   "The initial velocity, assumed constant everywhere");
  params.addParam<RealVectorValue>("initial_momentum",
                                   RealVectorValue(1e-15, 1e-15, 1e-15),
                                   "The initial momentum, assumed constant everywhere");
  params.addParam<std::string>("pressure_variable_name",
                               NS::pressure,
                               "A name for the pressure variable. If this is not provided, a "
                               "sensible default will be used.");

  params.addParamNamesToGroup(
      "simulation_type block gravity dynamic_viscosity_name density_name boussinesq_approximation "
      "ref_temperature thermal_expansion_name",
      "Base");
  params.addParamNamesToGroup("velocity_boundary velocity_function pressure_pinned_node "
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
    _turbulence_handling(getParam<MooseEnum>("turbulence_handling")),
    _blocks(getParam<std::vector<SubdomainName>>("block")),
    _inlet_boundaries(getParam<std::vector<BoundaryName>>("inlet_boundaries")),
    _outlet_boundaries(getParam<std::vector<BoundaryName>>("outlet_boundaries")),
    _wall_boundaries(getParam<std::vector<BoundaryName>>("wall_boundaries")),
    _velocity_inlet_function(getParam<std::vector<FunctionName>>("velocity_inlet_function")),
    _velocity_outlet_types(getParam<MultiMooseEnum>("velocity_outlet_types")),
    _velocity_wall_types(getParam<MultiMooseEnum>("velocity_wall_types")),
    _pressure_boundary(getParam<std::vector<BoundaryName>>("pressure_boundary")),
    _pressure_function(getParam<std::vector<FunctionName>>("pressure_function")),
    _has_pinned_dof(isParamValid("pressure_pinned_dof")),
    _pinned_dof("ins_pinned_dof"),
    _fixed_temperature_boundary(getParam<std::vector<BoundaryName>>("fixed_temperature_boundary")),
    _temperature_function(getParam<std::vector<FunctionName>>("temperature_function")),
    _fluid_temperature_variable_name(getParam<VariableName>("fluid_temperature_variable")),
    _solid_temperature_variable_name(getParam<VariableName>("solid_temperature_variable")),
    _pressure_variable_name(getParam<std::string>("pressure_variable_name"))
{
  if (_pressure_function.size() != _pressure_boundary.size())
    paramError("pressure_function",
               "Size is not the same as the number of boundaries in 'pressure_boundary'");
  if (_temperature_function.size() != _fixed_temperature_boundary.size())
    paramError("temperature_function",
               "Size is not the same as the number of boundaries in 'fixed_temperature_boundary'");

  if (_outlet_boundaries.size() != _velocity_outlet_types.size())
  {
    paramError("velocity_outlet_types",
               "Size is not the same as the number of outlet boundaries in 'outlet_boundaries'");
  }

  if (_wall_boundaries.size() != _velocity_wall_types.size())
  {
    paramError("velocity_wall_types",
               "Size is not the same as the number of outlet boundaries in 'wall_boundaries'");
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
  if (_current_task == "append_mesh_generator")
  {
    // if (_has_pinned_node)
    // {
    //   if (_app.getMeshGeneratorNames().size() == 0)
    //     mooseError("The new mesh generator system is required to pin pressure");
    //
    //   InputParameters params = _factory.getValidParams("ExtraNodesetGenerator");
    //   params.set<std::vector<BoundaryName>>("new_boundary") = {_pinned_node};
    //   params.set<std::vector<unsigned int>>("nodes") = {
    //       getParam<unsigned int>("pressure_pinned_node")};
    //   _app.appendMeshGenerator("ExtraNodesetGenerator", _pinned_node, params);
    // }
  }

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

    if (_velocity_inlet_function.size() != _inlet_boundaries.size() * _dim)
      paramError("velocity_inlet_function",
                 "Size is not the same as the number of boundaries in 'inlet_boundaries' times "
                 "the mesh dimension");

    auto base_params = _factory.getValidParams("MooseVariableFVReal");
    if (_block_ids.size() != 0)
      for (const SubdomainID & id : _block_ids)
        base_params.set<std::vector<SubdomainName>>("block").push_back(Moose::stringify(id));

    if (_porous_medium_treatment)
      _problem->addAuxVariable("MooseVariableFVReal", NS::porosity, base_params);

    if (_compressibility == "compressible")
    {
      if (_porous_medium_treatment)
        for (unsigned int d = 0; d < _dim; ++d)
          _problem->addVariable(
              "MooseVariableFVReal", NS::superficial_momentum_vector[d], base_params);
      else
        for (unsigned int d = 0; d < _dim; ++d)
          _problem->addVariable("MooseVariableFVReal", NS::momentum_vector[d], base_params);

      _problem->addVariable("MooseVariableFVReal", _pressure_variable_name, base_params);
      _problem->addVariable("MooseVariableFVReal", _fluid_temperature_variable_name, base_params);

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

      _problem->addVariable("INSFVPressureVariable", _pressure_variable_name, base_params);

      if (getParam<bool>("add_energy_equation") &&
          !_problem->getNonlinearSystemBase().hasVariable(_fluid_temperature_variable_name))
        _problem->addVariable("INSFVEnergyVariable", _fluid_temperature_variable_name, base_params);
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
      params.set<VariableName>("variable") = _fluid_temperature_variable_name;
      params.set<Real>("value") = tvalue;
      _problem->addInitialCondition("ConstantIC", _fluid_temperature_variable_name + "_ic", params);
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
        params.set<VariableName>("variable") = _fluid_temperature_variable_name;
        params.set<Real>("value") = tvalue;
        _problem->addInitialCondition(
            "ConstantIC", _fluid_temperature_variable_name + "_ic", params);
      }
    }

    if (pvalue != 0)
    {
      InputParameters params = _factory.getValidParams("ConstantIC");
      params.set<VariableName>("variable") = _pressure_variable_name;
      params.set<Real>("value") = pvalue;
      _problem->addInitialCondition("ConstantIC", "pressure_ic", params);
    }
  }

  if (_current_task == "add_navier_stokes_kernels")
  {
    if (_compressibility == "compressible")
    {
      if (_type == "transient")
        addCNSTimeKernels();

      addCNSMass();
      addCNSMomentum();
      addCNSEnergy();
    }
    else if (_compressibility == "weakly-compressible")
    {
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
    // if (_velocity_boundary.size() > 0)
    //   addINSVelocityBC();
    //
    // if (_has_pinned_node)
    //   addINSPinnedPressureBC();
    //
    // if (_no_bc_boundary.size() > 0)
    //   addINSNoBCBC();
    //
    // if (_pressure_boundary.size() > 0)
    //   addINSPressureBC();
    //
    // if (getParam<bool>("add_temperature_equation"))
    // {
    //   if (_fixed_temperature_boundary.size() > 0)
    //     addINSTemperatureBC();
    // }
  }

  if (_current_task == "add_material")
  {
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
    params.set<MaterialPropertyName>(NS::density) = NS::density;

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
      params.set<AuxVariableName>(NS::porosity) = NS::porosity;
      params.set<MaterialPropertyName>(NS::density) = NS::density;
      params.set<MaterialPropertyName>(NS::time_deriv(NS::density)) = NS::time_deriv(NS::density);
      params.set<MaterialPropertyName>(NS::cp) = NS::cp;
      params.set<MaterialPropertyName>(NS::time_deriv(NS::cp)) = NS::time_deriv(NS::cp);
      _problem->addKernel(en_kernel_type, "pins_energy_time", params);
    }
  }
  else
  {
    const std::string mom_kernel_type = "INSFVMomentumTimeDerivative";
    InputParameters params = _factory.getValidParams(mom_kernel_type);
    if (_blocks.size() > 0)
      params.set<std::vector<SubdomainName>>("block") = _blocks;
    params.set<MaterialPropertyName>(NS::density) = NS::density;

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
      params.set<MaterialPropertyName>(NS::density) = NS::density;
      params.set<MaterialPropertyName>(NS::time_deriv(NS::density)) = NS::time_deriv(NS::density);
      params.set<MaterialPropertyName>(NS::cp) = NS::cp;
      params.set<MaterialPropertyName>(NS::time_deriv(NS::cp)) = NS::time_deriv(NS::cp);
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

      params.set<NonlinearVariableName>("variable") = _pressure_variable_name;
      params.set<AuxVariableName>("porosity") = NS::porosity;
      params.set<MaterialPropertyName>(NS::time_deriv(NS::density)) = NS::time_deriv(NS::density);

      _problem->addKernel(mass_kernel_type, "pwcns_mass_time", params);
    }
    {
      const std::string mom_kernel_type = "WCNSFVMomentumTimeDerivative";
      InputParameters params = _factory.getValidParams(mom_kernel_type);
      if (_blocks.size() > 0)
        params.set<std::vector<SubdomainName>>("block") = _blocks;

      params.set<MaterialPropertyName>(NS::density) = NS::density;
      params.set<MaterialPropertyName>(NS::time_deriv(NS::density)) = NS::time_deriv(NS::density);

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
      params.set<AuxVariableName>(NS::porosity) = NS::porosity;
      params.set<MaterialPropertyName>(NS::density) = NS::density;
      params.set<MaterialPropertyName>(NS::time_deriv(NS::density)) = NS::time_deriv(NS::density);
      params.set<MaterialPropertyName>(NS::cp) = NS::cp;
      params.set<MaterialPropertyName>(NS::time_deriv(NS::cp)) = NS::time_deriv(NS::cp);
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
      params.set<NonlinearVariableName>("variable") = _pressure_variable_name;
      params.set<MaterialPropertyName>(NS::time_deriv(NS::density)) = NS::time_deriv(NS::density);
      _problem->addKernel(mass_kernel_type, "wcns_mass_time", params);
    }
    {
      const std::string mom_kernel_type = "WCNSFVMomentumTimeDerivative";
      InputParameters params = _factory.getValidParams(mom_kernel_type);
      if (_blocks.size() > 0)
        params.set<std::vector<SubdomainName>>("block") = _blocks;

      params.set<MaterialPropertyName>(NS::density) = NS::density;
      params.set<MaterialPropertyName>(NS::time_deriv(NS::density)) = NS::time_deriv(NS::density);

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
      params.set<MaterialPropertyName>(NS::density) = NS::density;
      params.set<MaterialPropertyName>(NS::time_deriv(NS::density)) = NS::time_deriv(NS::density);
      params.set<MaterialPropertyName>(NS::cp) = NS::cp;
      params.set<MaterialPropertyName>(NS::time_deriv(NS::cp)) = NS::time_deriv(NS::cp);
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

    params.set<NonlinearVariableName>("variable") = _pressure_variable_name;
    params.set<AuxVariableName>(NS::porosity) = NS::porosity;
    params.set<MaterialPropertyName>(NS::density) = NS::density;
    params.set<MooseEnum>("velocity_interp_method") = "rc";
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

    params.set<NonlinearVariableName>("variable") = _pressure_variable_name;
    params.set<MaterialPropertyName>(NS::density) = NS::density;
    params.set<MooseEnum>("velocity_interp_method") = "rc";
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

      params.set<MaterialPropertyName>(NS::density) = NS::density;
      params.set<AuxVariableName>(NS::porosity) = NS::porosity;
      params.set<MooseEnum>("velocity_interp_method") = "rc";
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

      params.set<MaterialPropertyName>(NS::mu) = NS::mu;
      params.set<AuxVariableName>(NS::porosity) = NS::porosity;

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

      params.set<AuxVariableName>(NS::porosity) = NS::porosity;
      params.set<NonlinearVariableName>("pressure") = _pressure_variable_name;

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

      params.set<AuxVariableName>(NS::porosity) = NS::porosity;
      params.set<MaterialPropertyName>(NS::density) = NS::density;
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

      params.set<AuxVariableName>(NS::porosity) = NS::porosity;
      params.set<MaterialPropertyName>(NS::density) = NS::density;
      params.set<RealVectorValue>("gravity") = getParam<RealVectorValue>("gravity");
      params.set<Real>("ref_temperature") = getParam<Real>("ref_temperature");
      params.set<MaterialPropertyName>("alpha_name") =
          getParam<MaterialPropertyName>("thermal_expansion_name");

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

      params.set<MaterialPropertyName>(NS::density) = NS::density;
      params.set<MooseEnum>("velocity_interp_method") = "rc";
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

      params.set<MaterialPropertyName>(NS::mu) = NS::mu;
      params.set<AuxVariableName>(NS::porosity) = NS::porosity;

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

      params.set<NonlinearVariableName>("pressure") = _pressure_variable_name;

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

      params.set<MaterialPropertyName>(NS::density) = NS::density;
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
      params.set<MaterialPropertyName>(NS::density) = NS::density;
      params.set<RealVectorValue>("gravity") = getParam<RealVectorValue>("gravity");
      params.set<Real>("ref_temperature") = getParam<Real>("ref_temperature");
      params.set<MaterialPropertyName>("alpha_name") =
          getParam<MaterialPropertyName>("thermal_expansion_name");

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
      params.set<NonlinearVariableName>("variable") = _fluid_temperature_variable_name;
      if (_blocks.size() > 0)
        params.set<std::vector<SubdomainName>>("block") = _blocks;

      params.set<MaterialPropertyName>(NS::density) = NS::density;
      params.set<AuxVariableName>(NS::porosity) = NS::porosity;
      params.set<MooseEnum>("velocity_interp_method") = "rc";
      params.set<MooseEnum>("advected_interp_method") = "average";
      for (unsigned int d = 0; d < _dim; ++d)
        params.set<NonlinearVariableName>(u_names[d]) = NS::superficial_velocity_vector[d];

      _problem->addKernel(kernel_type, "pins_energy_advection", params);
    }
    {
      const std::string kernel_type = "PINSFVEnergyDiffusion";
      InputParameters params = _factory.getValidParams(kernel_type);
      params.set<NonlinearVariableName>("variable") = _fluid_temperature_variable_name;
      if (_blocks.size() > 0)
        params.set<std::vector<SubdomainName>>("block") = _blocks;

      params.set<MaterialPropertyName>(NS::k) = NS::k;
      params.set<AuxVariableName>(NS::porosity) = NS::porosity;

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
      params.set<NonlinearVariableName>("variable") = _fluid_temperature_variable_name;
      if (_blocks.size() > 0)
        params.set<std::vector<SubdomainName>>("block") = _blocks;

      params.set<MaterialPropertyName>(NS::density) = NS::density;
      params.set<MooseEnum>("velocity_interp_method") = "rc";
      params.set<MooseEnum>("advected_interp_method") = "average";
      for (unsigned int d = 0; d < _dim; ++d)
        params.set<NonlinearVariableName>(u_names[d]) = NS::velocity_vector[d];

      _problem->addKernel(kernel_type, "ins_energy_convection", params);
    }
    {
      const std::string kernel_type = "PINSFVEnergyDiffusion";
      InputParameters params = _factory.getValidParams(kernel_type);
      params.set<NonlinearVariableName>("variable") = _fluid_temperature_variable_name;
      if (_blocks.size() > 0)
        params.set<std::vector<SubdomainName>>("block") = _blocks;

      params.set<MaterialPropertyName>(NS::k) = NS::k;

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
NSFVAction::addINSInletVelocityBC(unsigned int /*bc_index*/)
{
  _console << "Something here. " << std::endl;
}

void
NSFVAction::addINSVelocityBC()
{
  for (unsigned int bc_ind = 0; bc_ind < _inlet_boundaries.size(); ++bc_ind)
    addINSInletVelocityBC(bc_ind);

  for (unsigned int bc_ind = 0; bc_ind < _outlet_boundaries.size(); ++bc_ind)
    addINSOutletVelocityBC(bc_ind);

  for (unsigned int bc_ind = 0; bc_ind < _wall_boundaries.size(); ++bc_ind)
    addINSWallVelocityBC(bc_ind);

  // const static std::string momentums[3] = {NS::velocity_x, NS::velocity_y, NS::velocity_z};
  // for (unsigned int i = 0; i < _velocity_boundary.size(); ++i)
  // {
  //   if (_use_ad)
  //   {
  //     InputParameters params = _factory.getValidParams("ADVectorFunctionDirichletBC");
  //
  //     {
  //       const FunctionName funcx = _velocity_function[i * _dim];
  //       if (funcx == "NA")
  //         params.set<bool>("set_x_comp") = false;
  //       else
  //       {
  //         std::stringstream ss(funcx);
  //         Real val;
  //         if ((ss >> val).fail() || !ss.eof())
  //         {
  //           if (!_problem->hasFunction(funcx))
  //           {
  //             InputParameters func_params = _factory.getValidParams("ConstantFunction");
  //             func_params.set<Real>("value") = val;
  //             _problem->addFunction("ConstantFunction", funcx, func_params);
  //           }
  //         }
  //         params.set<FunctionName>("function_x") = funcx;
  //       }
  //     }
  //
  //     if (_dim >= 2)
  //     {
  //       const FunctionName funcy = _velocity_function[i * _dim + 1];
  //       if (funcy == "NA")
  //         params.set<bool>("set_y_comp") = false;
  //       else
  //       {
  //         std::stringstream ss(funcy);
  //         Real val;
  //         if ((ss >> val).fail() || !ss.eof())
  //         {
  //           if (!_problem->hasFunction(funcy))
  //           {
  //             InputParameters func_params = _factory.getValidParams("ConstantFunction");
  //             func_params.set<Real>("value") = val;
  //             _problem->addFunction("ConstantFunction", funcy, func_params);
  //           }
  //         }
  //         params.set<FunctionName>("function_y") = funcy;
  //       }
  //     }
  //
  //     if (_dim >= 3)
  //     {
  //       const FunctionName funcz = _velocity_function[i * _dim + 1];
  //       if (funcz == "NA")
  //         params.set<bool>("set_z_comp") = false;
  //       else
  //       {
  //         std::stringstream ss(funcz);
  //         Real val;
  //         if ((ss >> val).fail() || !ss.eof())
  //         {
  //           if (!_problem->hasFunction(funcz))
  //           {
  //             InputParameters func_params = _factory.getValidParams("ConstantFunction");
  //             func_params.set<Real>("value") = val;
  //             _problem->addFunction("ConstantFunction", funcz, func_params);
  //           }
  //         }
  //         params.set<FunctionName>("function_z") = funcz;
  //       }
  //     }
  //
  //     params.set<NonlinearVariableName>("variable") = NS::velocity;
  //     params.set<std::vector<BoundaryName>>("boundary") = {_velocity_boundary[i]};
  //     _problem->addBoundaryCondition(
  //         "ADVectorFunctionDirichletBC", "ins_velocity_bc_" + _velocity_boundary[i], params);
  //   }
  //   else
  //   {
  //     for (unsigned int component = 0; component < _dim; ++component)
  //     {
  //       const FunctionName func = _velocity_function[i * _dim + component];
  //       if (func == "NA")
  //         continue;
  //
  //       std::stringstream ss(func);
  //       Real val;
  //       if ((ss >> val).fail() || !ss.eof())
  //       {
  //         InputParameters params = _factory.getValidParams("FunctionDirichletBC");
  //         params.set<FunctionName>("function") = func;
  //         params.set<NonlinearVariableName>("variable") = momentums[component];
  //         params.set<std::vector<BoundaryName>>("boundary") = {_velocity_boundary[i]};
  //         _problem->addBoundaryCondition(
  //             "FunctionDirichletBC", momentums[component] + "_" + _velocity_boundary[i],
  //             params);
  //       }
  //       else
  //       {
  //         InputParameters params = _factory.getValidParams("DirichletBC");
  //         params.set<Real>("value") = val;
  //         params.set<NonlinearVariableName>("variable") = momentums[component];
  //         params.set<std::vector<BoundaryName>>("boundary") = {_velocity_boundary[i]};
  //         _problem->addBoundaryCondition(
  //             "DirichletBC", momentums[component] + "_" + _velocity_boundary[i], params);
  //       }
  //     }
  //   }
  // }
}

void
NSFVAction::addINSTemperatureBC()
{
  _console << "something here" << std::endl;
  // for (unsigned int i = 0; i < _fixed_temperature_boundary.size(); ++i)
  // {
  //   const FunctionName func = _temperature_function[i];
  //   if (func == "NA")
  //     continue;
  //
  //   std::stringstream ss(func);
  //   Real val;
  //   if ((ss >> val).fail() || !ss.eof())
  //   {
  //     InputParameters params = _factory.getValidParams("FunctionDirichletBC");
  //     params.set<FunctionName>("function") = func;
  //     params.set<NonlinearVariableName>("variable") = _temperature_variable_name;
  //     params.set<std::vector<BoundaryName>>("boundary") = {_fixed_temperature_boundary[i]};
  //     _problem->addBoundaryCondition("FunctionDirichletBC",
  //                                    _temperature_variable_name + "_" +
  //                                        _fixed_temperature_boundary[i],
  //                                    params);
  //   }
  //   else
  //   {
  //     InputParameters params = _factory.getValidParams("DirichletBC");
  //     params.set<Real>("value") = val;
  //     params.set<NonlinearVariableName>("variable") = _temperature_variable_name;
  //     params.set<std::vector<BoundaryName>>("boundary") = {_fixed_temperature_boundary[i]};
  //     _problem->addBoundaryCondition(
  //         "DirichletBC", _temperature_variable_name + "_" + _fixed_temperature_boundary[i],
  //         params);
  //   }
  // }
}

void
NSFVAction::addINSPressureBC()
{
  _console << "something here" << std::endl;
  // for (unsigned int i = 0; i < _pressure_boundary.size(); ++i)
  // {
  //   const FunctionName func = _pressure_function[i];
  //   std::stringstream ss(func);
  //   Real val;
  //   if ((ss >> val).fail() || !ss.eof())
  //   {
  //     InputParameters params = _factory.getValidParams("FunctionDirichletBC");
  //     params.set<FunctionName>("function") = func;
  //     params.set<NonlinearVariableName>("variable") = _pressure_variable_name;
  //     params.set<std::vector<BoundaryName>>("boundary") = {_pressure_boundary[i]};
  //     _problem->addBoundaryCondition(
  //         "FunctionDirichletBC", NS::pressure + _pressure_boundary[i], params);
  //   }
  //   else
  //   {
  //     InputParameters params = _factory.getValidParams("DirichletBC");
  //     params.set<Real>("value") = val;
  //     params.set<NonlinearVariableName>("variable") = _pressure_variable_name;
  //     params.set<std::vector<BoundaryName>>("boundary") = {_pressure_boundary[i]};
  //     _problem->addBoundaryCondition("DirichletBC", NS::pressure + _pressure_boundary[i],
  //     params);
  //   }
  // }
}

void
NSFVAction::addINSPinnedPressureBC()
{
  _console << "something here" << std::endl;
  // InputParameters params = _factory.getValidParams("DirichletBC");
  // params.set<Real>("value") = 0;
  // params.set<NonlinearVariableName>("variable") = _pressure_variable_name;
  // params.set<std::vector<BoundaryName>>("boundary") = {_pinned_node};
  // _problem->addBoundaryCondition("DirichletBC", "pressure_pin", params);
}

void
NSFVAction::setKernelCommonParams(InputParameters & params)
{
  if (_blocks.size() > 0)
    params.set<std::vector<SubdomainName>>("block") = _blocks;

  // coupled variables
  params.set<CoupledName>("u") = {NS::velocity_x};
  if (_dim >= 2)
    params.set<CoupledName>("v") = {NS::velocity_y};
  if (_dim >= 3)
    params.set<CoupledName>("w") = {NS::velocity_z};
  params.set<CoupledName>(NS::pressure) = {_pressure_variable_name};
  params.set<RealVectorValue>("gravity") = getParam<RealVectorValue>("gravity");
  params.set<MaterialPropertyName>("mu_name") =
      getParam<MaterialPropertyName>("dynamic_viscosity_name");
  params.set<MaterialPropertyName>("rho_name") = getParam<MaterialPropertyName>("density_name");
  params.set<Real>("alpha") = getParam<Real>("alpha");
  params.set<bool>("laplace") = getParam<bool>("laplace");
  // this parameter only affecting Jacobian evaluation in non-AD
  params.set<bool>("convective_term") = getParam<bool>("convective_term");
  // FIXME: this parameter seems not changing solution much?
  params.set<bool>("transient_term") = (_type == "transient");
}

void
NSFVAction::setNoBCCommonParams(InputParameters & params)
{
  // coupled variables
  params.set<CoupledName>("u") = {NS::velocity_x};
  if (_dim >= 2)
    params.set<CoupledName>("v") = {NS::velocity_y};
  if (_dim >= 3)
    params.set<CoupledName>("w") = {NS::velocity_z};
  params.set<CoupledName>(NS::pressure) = {_pressure_variable_name};
  params.set<RealVectorValue>("gravity") = getParam<RealVectorValue>("gravity");
  params.set<MaterialPropertyName>("mu_name") =
      getParam<MaterialPropertyName>("dynamic_viscosity_name");
  params.set<MaterialPropertyName>("rho_name") = getParam<MaterialPropertyName>("density_name");
  params.set<bool>("integrate_p_by_parts") = getParam<bool>("integrate_p_by_parts");
}
