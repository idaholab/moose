//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowSinglePhaseBase.h"

#include "FEProblem.h"
#include "Conversion.h"
#include "libmesh/string_to_enum.h"

InputParameters
PorousFlowSinglePhaseBase::validParams()
{
  InputParameters params = PorousFlowActionBase::validParams();
  params.addParam<bool>("add_darcy_aux", true, "Add AuxVariables that record Darcy velocity");
  params.addParam<bool>("add_stress_aux", true, "Add AuxVariables that record effective stress");
  params.addDeprecatedParam<bool>("use_brine",
                                  false,
                                  "Whether to use a PorousFlowBrine Material",
                                  "This parameter should no longer be used.  Instead use "
                                  "fluid_properties_type = PorousFlowBrine");
  params.addRequiredParam<VariableName>("porepressure", "The name of the porepressure variable");
  MooseEnum coupling_type("Hydro ThermoHydro HydroMechanical ThermoHydroMechanical", "Hydro");
  params.addParam<MooseEnum>("coupling_type",
                             coupling_type,
                             "The type of simulation.  For simulations involving Mechanical "
                             "deformations, you will need to supply the correct Biot coefficient.  "
                             "For simulations involving Thermal flows, you will need an associated "
                             "ConstantThermalExpansionCoefficient Material");
  MooseEnum fluid_properties_type("PorousFlowSingleComponentFluid PorousFlowBrine Custom",
                                  "PorousFlowSingleComponentFluid");
  params.addParam<MooseEnum>(
      "fluid_properties_type",
      fluid_properties_type,
      "Type of fluid properties to use.  For 'PorousFlowSingleComponentFluid' you must provide a "
      "fp UserObject.  For 'PorousFlowBrine' you must supply a nacl_name.  For "
      "'Custom' your input file must include a Material that provides fluid properties such as "
      "density, viscosity, enthalpy and internal energy");
  MooseEnum simulation_type_choice("steady transient", "transient");
  params.addDeprecatedParam<MooseEnum>(
      "simulation_type",
      simulation_type_choice,
      "Whether a transient or steady-state simulation is being performed",
      "The execution type is now determined automatically. This parameter should no longer be "
      "used");
  params.addParam<UserObjectName>(
      "fp",
      "The name of the user object for fluid "
      "properties. Only needed if fluid_properties_type = PorousFlowSingleComponentFluid");
  params.addCoupledVar("mass_fraction_vars",
                       {},
                       "List of variables that represent the mass fractions.  With only one fluid "
                       "component, this may be left empty.  With N fluid components, the format is "
                       "'f_0 f_1 f_2 ... f_(N-1)'.  That is, the N^th component need not be "
                       "specified because f_N = 1 - (f_0 + f_1 + ... + f_(N-1)).  It is best "
                       "numerically to choose the N-1 mass fraction variables so that they "
                       "represent the fluid components with small concentrations.  This Action "
                       "will associated the i^th mass fraction variable to the equation for the "
                       "i^th fluid component, and the pressure variable to the N^th fluid "
                       "component.");
  params.addDeprecatedParam<unsigned>(
      "nacl_index",
      0,
      "Index of NaCl variable in mass_fraction_vars, for "
      "calculating brine properties. Only required if use_brine is true.",
      "This parameter should no longer be used.  Instead use nacl_name = the_nacl_variable_name");
  params.addParam<VariableName>(
      "nacl_name",
      "Name of the NaCl variable.  Only required if fluid_properties_type = PorousFlowBrine");
  params.addParam<Real>(
      "biot_coefficient",
      1.0,
      "The Biot coefficient (relevant only for mechanically-coupled simulations)");
  params.addParam<std::vector<AuxVariableName>>(
      "save_component_rate_in",
      {},
      "List of AuxVariables into which the rate-of-change of each fluid component at each node "
      "will be saved.  There must be exactly N of these to match the N fluid components.  The "
      "result will be measured in kg/s, where the kg is the mass of the fluid component at the "
      "node (or m^3/s if multiply_by_density=false).  Note that this saves the result from the "
      "MassTimeDerivative Kernels, but NOT from the MassVolumetricExpansion Kernels.");
  MooseEnum temp_unit_choice("Kelvin Celsius", "Kelvin");
  params.addParam<MooseEnum>(
      "temperature_unit", temp_unit_choice, "The unit of the temperature variable");
  MooseEnum p_unit_choice("Pa MPa", "Pa");
  params.addParam<MooseEnum>(
      "pressure_unit",
      p_unit_choice,
      "The unit of the pressure variable used everywhere in the input file "
      "except for in the FluidProperties-module objects.  This can be set to the non-default value "
      "only for fluid_properties_type = PorousFlowSingleComponentFluid");
  MooseEnum time_unit_choice("seconds hours days years", "seconds");
  params.addParam<MooseEnum>(
      "time_unit",
      time_unit_choice,
      "The unit of time used everywhere in the input file except for in the "
      "FluidProperties-module objects.  This can be set to the non-default value only for "
      "fluid_properties_type = PorousFlowSingleComponentFluid");
  params.addParam<std::string>("base_name",
                               "The base_name used in the TensorMechanics strain calculator.  This "
                               "is only relevant for mechanically-coupled models.");
  params.addClassDescription("Base class for single-phase simulations");
  return params;
}

PorousFlowSinglePhaseBase::PorousFlowSinglePhaseBase(const InputParameters & params)
  : PorousFlowActionBase(params),
    _pp_var(getParam<VariableName>("porepressure")),
    _coupling_type(getParam<MooseEnum>("coupling_type").getEnum<CouplingTypeEnum>()),
    _thermal(_coupling_type == CouplingTypeEnum::ThermoHydro ||
             _coupling_type == CouplingTypeEnum::ThermoHydroMechanical),
    _mechanical(_coupling_type == CouplingTypeEnum::HydroMechanical ||
                _coupling_type == CouplingTypeEnum::ThermoHydroMechanical),
    _fluid_properties_type(
        getParam<MooseEnum>("fluid_properties_type").getEnum<FluidPropertiesTypeEnum>()),
    _biot_coefficient(getParam<Real>("biot_coefficient")),
    _add_darcy_aux(getParam<bool>("add_darcy_aux")),
    _add_stress_aux(getParam<bool>("add_stress_aux")),
    _save_component_rate_in(getParam<std::vector<AuxVariableName>>("save_component_rate_in")),
    _temperature_unit(getParam<MooseEnum>("temperature_unit")),
    _pressure_unit(getParam<MooseEnum>("pressure_unit")),
    _time_unit(getParam<MooseEnum>("time_unit")),
    _base_name(isParamValid("base_name") ? getParam<std::string>("base_name") + "_" : "")
{
  if (_thermal && _temperature_var.size() != 1)
    mooseError("PorousFlowSinglePhaseBase: You need to specify a temperature variable to perform "
               "non-isothermal simulations");

  if (_fluid_properties_type == FluidPropertiesTypeEnum::PorousFlowSingleComponentFluid)
  {
    if (params.isParamValid("nacl_name"))
      paramError("nacl_name",
                 "PorousFlowSinglePhaseBase: You should not specify a nacl_name when "
                 "fluid_properties_type = PorousFlowSingleComponentFluid");
    if (!params.isParamValid("fp"))
      paramError("fp",
                 "PorousFlowSinglePhaseBase: You must specify fp when fluid_properties_type = "
                 "PorousFlowSingleComponentFluid");
    _fp = getParam<UserObjectName>("fp");
  }

  if (_fluid_properties_type == FluidPropertiesTypeEnum::PorousFlowBrine)
  {
    if (!params.isParamValid("nacl_name"))
      paramError("nacl_name",
                 "PorousFlowSinglePhaseBase: You must specify nacl_name when "
                 "fluid_properties_type = PorousFlowBrine");
    if (params.isParamValid("fp"))
      paramError("fp",
                 "PorousFlowSinglePhaseBase: You should not specify fp when "
                 "fluid_properties_type = PorousFlowBrine");
    if (_pressure_unit != "Pa")
      paramError("pressure_unit",
                 "Must use pressure_unit = Pa for fluid_properties_type = PorousFlowBrine");
    if (_time_unit != "seconds")
      paramError("time_unit",
                 "Must use time_unit = seconds for fluid_properties_type = PorousFlowBrine");
    _nacl_name = getParam<VariableName>("nacl_name");
  }

  auto save_component_rate_in_size = _save_component_rate_in.size();
  if (save_component_rate_in_size && save_component_rate_in_size != _num_mass_fraction_vars + 1)
    paramError("save_component_rate_in",
               "The number of save_component_rate_in variables must be the number of fluid "
               "components + 1");
}

void
PorousFlowSinglePhaseBase::addMaterialDependencies()
{
  PorousFlowActionBase::addMaterialDependencies();

  // Add necessary objects to list of PorousFlow objects added by this action
  if (_mechanical)
  {
    _included_objects.push_back("StressDivergenceTensors");
    _included_objects.push_back("Gravity");
    _included_objects.push_back("PorousFlowEffectiveStressCoupling");
  }

  if (_thermal)
  {
    _included_objects.push_back("PorousFlowHeatConduction");
    if (_transient)
      _included_objects.push_back("PorousFlowEnergyTimeDerivative");
  }

  if (_thermal && _mechanical && _transient)
    _included_objects.push_back("PorousFlowHeatVolumetricExpansion");

  if (_add_darcy_aux)
    _included_objects.push_back("PorousFlowDarcyVelocityComponent");

  if (_add_stress_aux && _mechanical)
    _included_objects.push_back("StressAux");
}

void
PorousFlowSinglePhaseBase::addKernels()
{
  PorousFlowActionBase::addKernels();

  if (_mechanical)
  {
    for (unsigned i = 0; i < _ndisp; ++i)
    {
      std::string kernel_name = "PorousFlowUnsaturated_grad_stress" + Moose::stringify(i);
      std::string kernel_type = "StressDivergenceTensors";
      if (_coord_system == Moose::COORD_RZ)
        kernel_type = "StressDivergenceRZTensors";
      InputParameters params = _factory.getValidParams(kernel_type);
      if (_subdomain_names_set)
        params.set<std::vector<SubdomainName>>("block") = _subdomain_names;
      params.set<NonlinearVariableName>("variable") = _displacements[i];
      params.set<std::vector<VariableName>>("displacements") = _coupled_displacements;
      if (_thermal)
      {
        params.set<std::vector<VariableName>>("temperature") = _temperature_var;
        if (parameters().isParamValid("eigenstrain_names"))
        {
          params.set<std::vector<MaterialPropertyName>>("eigenstrain_names") =
              getParam<std::vector<MaterialPropertyName>>("eigenstrain_names");
        }
      }
      params.set<unsigned>("component") = i;
      params.set<bool>("use_displaced_mesh") = getParam<bool>("use_displaced_mesh");
      _problem->addKernel(kernel_type, kernel_name, params);

      if (_gravity(i) != 0)
      {
        kernel_name = "PorousFlowUnsaturated_gravity" + Moose::stringify(i);
        kernel_type = "Gravity";
        params = _factory.getValidParams(kernel_type);
        if (_subdomain_names_set)
          params.set<std::vector<SubdomainName>>("block") = _subdomain_names;
        params.set<NonlinearVariableName>("variable") = _displacements[i];
        params.set<Real>("value") = _gravity(i);
        params.set<bool>("use_displaced_mesh") = getParam<bool>("use_displaced_mesh");
        _problem->addKernel(kernel_type, kernel_name, params);
      }

      kernel_name = "PorousFlowUnsaturated_EffStressCoupling" + Moose::stringify(i);
      kernel_type = "PorousFlowEffectiveStressCoupling";
      params = _factory.getValidParams(kernel_type);
      if (_subdomain_names_set)
        params.set<std::vector<SubdomainName>>("block") = _subdomain_names;
      params.set<UserObjectName>("PorousFlowDictator") = _dictator_name;
      params.set<NonlinearVariableName>("variable") = _displacements[i];
      params.set<Real>("biot_coefficient") = _biot_coefficient;
      params.set<unsigned>("component") = i;
      params.set<bool>("use_displaced_mesh") = getParam<bool>("use_displaced_mesh");
      _problem->addKernel(kernel_type, kernel_name, params);
    }
  }

  if (_thermal)
  {
    std::string kernel_name = "PorousFlowUnsaturated_HeatConduction";
    std::string kernel_type = "PorousFlowHeatConduction";
    InputParameters params = _factory.getValidParams(kernel_type);
    params.set<NonlinearVariableName>("variable") = _temperature_var[0];
    params.set<UserObjectName>("PorousFlowDictator") = _dictator_name;
    _problem->addKernel(kernel_type, kernel_name, params);

    if (_transient)
    {
      kernel_name = "PorousFlowUnsaturated_EnergyTimeDerivative";
      kernel_type = "PorousFlowEnergyTimeDerivative";
      params = _factory.getValidParams(kernel_type);
      params.set<NonlinearVariableName>("variable") = _temperature_var[0];
      params.set<UserObjectName>("PorousFlowDictator") = _dictator_name;
      params.set<bool>("strain_at_nearest_qp") = _strain_at_nearest_qp;
      if (!_base_name.empty())
        params.set<std::string>("base_name") = _base_name;
      _problem->addKernel(kernel_type, kernel_name, params);
    }
  }

  if (_thermal && _mechanical && _transient)
  {
    std::string kernel_name = "PorousFlowUnsaturated_HeatVolumetricExpansion";
    std::string kernel_type = "PorousFlowHeatVolumetricExpansion";
    InputParameters params = _factory.getValidParams(kernel_type);
    params.set<UserObjectName>("PorousFlowDictator") = _dictator_name;
    params.set<NonlinearVariableName>("variable") = _temperature_var[0];
    params.set<bool>("strain_at_nearest_qp") = _strain_at_nearest_qp;
    _problem->addKernel(kernel_type, kernel_name, params);
  }
}

void
PorousFlowSinglePhaseBase::addAuxObjects()
{
  PorousFlowActionBase::addAuxObjects();

  if (_add_darcy_aux)
    addDarcyAux(_gravity);

  if (_add_stress_aux && _mechanical)
    addStressAux();
}

void
PorousFlowSinglePhaseBase::addMaterials()
{
  PorousFlowActionBase::addMaterials();

  // add Materials
  if (_deps.dependsOn(_included_objects, "temperature_qp"))
    addTemperatureMaterial(false);

  if (_deps.dependsOn(_included_objects, "temperature_nodal"))
    addTemperatureMaterial(true);

  if (_deps.dependsOn(_included_objects, "mass_fraction_qp"))
    addMassFractionMaterial(false);

  if (_deps.dependsOn(_included_objects, "mass_fraction_nodal"))
    addMassFractionMaterial(true);

  const bool compute_rho_mu_qp = _deps.dependsOn(_included_objects, "density_qp") ||
                                 _deps.dependsOn(_included_objects, "viscosity_qp");
  const bool compute_e_qp = _deps.dependsOn(_included_objects, "internal_energy_qp");
  const bool compute_h_qp = _deps.dependsOn(_included_objects, "enthalpy_qp");

  if (compute_rho_mu_qp || compute_e_qp || compute_h_qp)
  {
    if (_fluid_properties_type == FluidPropertiesTypeEnum::PorousFlowBrine)
      addBrineMaterial(
          _nacl_name, false, 0, compute_rho_mu_qp, compute_e_qp, compute_h_qp, _temperature_unit);
    else if (_fluid_properties_type == FluidPropertiesTypeEnum::PorousFlowSingleComponentFluid)
      addSingleComponentFluidMaterial(false,
                                      0,
                                      compute_rho_mu_qp,
                                      compute_e_qp,
                                      compute_h_qp,
                                      _fp,
                                      _temperature_unit,
                                      _pressure_unit,
                                      _time_unit);
  }

  const bool compute_rho_mu_nodal = _deps.dependsOn(_included_objects, "density_nodal") ||
                                    _deps.dependsOn(_included_objects, "viscosity_nodal");
  const bool compute_e_nodal = _deps.dependsOn(_included_objects, "internal_energy_nodal");
  const bool compute_h_nodal = _deps.dependsOn(_included_objects, "enthalpy_nodal");

  if (compute_rho_mu_nodal || compute_e_nodal || compute_h_nodal)
  {
    if (_fluid_properties_type == FluidPropertiesTypeEnum::PorousFlowBrine)
      addBrineMaterial(_nacl_name,
                       true,
                       0,
                       compute_rho_mu_nodal,
                       compute_e_nodal,
                       compute_h_nodal,
                       _temperature_unit);
    else if (_fluid_properties_type == FluidPropertiesTypeEnum::PorousFlowSingleComponentFluid)
      addSingleComponentFluidMaterial(true,
                                      0,
                                      compute_rho_mu_nodal,
                                      compute_e_nodal,
                                      compute_h_nodal,
                                      _fp,
                                      _temperature_unit,
                                      _pressure_unit,
                                      _time_unit);
  }

  if (_deps.dependsOn(_included_objects, "effective_pressure_qp"))
    addEffectiveFluidPressureMaterial(false);

  if (_deps.dependsOn(_included_objects, "effective_pressure_nodal"))
    addEffectiveFluidPressureMaterial(true);
}

void
PorousFlowSinglePhaseBase::addDictator()
{
  const std::string uo_name = _dictator_name;
  const std::string uo_type = "PorousFlowDictator";
  InputParameters params = _factory.getValidParams(uo_type);
  std::vector<VariableName> pf_vars = _mass_fraction_vars;
  pf_vars.push_back(_pp_var);
  if (_thermal)
    pf_vars.push_back(_temperature_var[0]);
  if (_mechanical)
    pf_vars.insert(pf_vars.end(), _coupled_displacements.begin(), _coupled_displacements.end());
  params.set<std::vector<VariableName>>("porous_flow_vars") = pf_vars;
  params.set<unsigned int>("number_fluid_phases") = 1;
  params.set<unsigned int>("number_fluid_components") = _num_mass_fraction_vars + 1;
  params.set<unsigned int>("number_aqueous_equilibrium") = _num_aqueous_equilibrium;
  params.set<unsigned int>("number_aqueous_kinetic") = _num_aqueous_kinetic;
  _problem->addUserObject(uo_type, uo_name, params);
}
