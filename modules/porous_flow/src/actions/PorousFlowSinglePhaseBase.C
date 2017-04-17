/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "PorousFlowSinglePhaseBase.h"

#include "FEProblem.h"
#include "Conversion.h"
#include "libmesh/string_to_enum.h"

template <>
InputParameters
validParams<PorousFlowSinglePhaseBase>()
{
  InputParameters params = validParams<PorousFlowActionBase>();
  params.addParam<bool>("add_darcy_aux", true, "Add AuxVariables that record Darcy velocity");
  params.addParam<bool>("add_stress_aux", true, "Add AuxVariables that record effective stress");
  params.addRequiredParam<NonlinearVariableName>("porepressure",
                                                 "The name of the porepressure variable");
  MooseEnum coupling_type("Hydro ThermoHydro HydroMechanical ThermoHydroMechanical", "Hydro");
  params.addParam<MooseEnum>("coupling_type",
                             coupling_type,
                             "The type of simulation.  For simulations involving Mechanical "
                             "deformations, you will need to supply the correct Biot coefficient.  "
                             "For simulations involving Thermal flows, you will need an associated "
                             "ConstantThermalExpansionCoefficient Material");
  MooseEnum simulation_type_choice("steady transient", "transient");
  params.addParam<MooseEnum>("simulation_type",
                             simulation_type_choice,
                             "Whether a transient or steady-state simulation is being performed");
  params.addRequiredParam<UserObjectName>("fp", "The name of the user object for fluid properties");
  params.addCoupledVar("mass_fraction_vars",
                       "List of variables that represent the mass fractions.  With only one fluid "
                       "component, this may be left empty.  With N fluid components, the format is "
                       "'f_0 f_1 f_2 ... f_(N-1)'.  That is, the N^th component need not be "
                       "specified because f_N = 1 - (f_0 + f_1 + ... + f_(N-1)).  It is best "
                       "numerically to choose the N-1 mass fraction variables so that they "
                       "represent the fluid components with small concentrations.  This Action "
                       "will associated the i^th mass fraction variable to the equation for the "
                       "i^th fluid component, and the pressure variable to the N^th fluid "
                       "component.");
  params.addParam<Real>(
      "biot_coefficient",
      1.0,
      "The Biot coefficient (relevant only for mechanically-coupled simulations)");
  params.addClassDescription("Base class for single-phase simulations");
  return params;
}

PorousFlowSinglePhaseBase::PorousFlowSinglePhaseBase(const InputParameters & params)
  : PorousFlowActionBase(params),
    _pp_var(getParam<NonlinearVariableName>("porepressure")),
    _coupling_type(getParam<MooseEnum>("coupling_type").getEnum<CouplingTypeEnum>()),
    _simulation_type(getParam<MooseEnum>("simulation_type").getEnum<SimulationTypeChoiceEnum>()),
    _fp(getParam<UserObjectName>("fp")),
    _biot_coefficient(getParam<Real>("biot_coefficient")),
    _add_darcy_aux(getParam<bool>("add_darcy_aux")),
    _add_stress_aux(getParam<bool>("add_stress_aux"))
{
  if ((_coupling_type == CouplingTypeEnum::ThermoHydro ||
       _coupling_type == CouplingTypeEnum::ThermoHydroMechanical) &&
      _temperature_var.size() != 1)
    mooseError("PorousFlowSinglePhaseBase: You need to specify a temperature variable to perform "
               "non-isothermal simulations");

  if (_coupling_type == CouplingTypeEnum::HydroMechanical ||
      _coupling_type == CouplingTypeEnum::ThermoHydroMechanical)
  {
    _objects_to_add.push_back("StressDivergenceTensors");
    _objects_to_add.push_back("Gravity");
    _objects_to_add.push_back("PorousFlowEffectiveStressCoupling");
  }
  if (_coupling_type == CouplingTypeEnum::ThermoHydro ||
      _coupling_type == CouplingTypeEnum::ThermoHydroMechanical)
  {
    _objects_to_add.push_back("PorousFlowHeatConduction");
    if (_simulation_type == SimulationTypeChoiceEnum::TRANSIENT)
      _objects_to_add.push_back("PorousFlowEnergyTimeDerivative");
  }
  if (_coupling_type == CouplingTypeEnum::ThermoHydroMechanical &&
      _simulation_type == SimulationTypeChoiceEnum::TRANSIENT)
    _objects_to_add.push_back("PorousFlowHeatVolumetricExpansion");
  if (_add_darcy_aux)
    _objects_to_add.push_back("PorousFlowDarcyVelocityComponent");
  if (_add_stress_aux && (_coupling_type == CouplingTypeEnum::HydroMechanical ||
                          _coupling_type == CouplingTypeEnum::ThermoHydroMechanical))
    _objects_to_add.push_back("StressAux");
}

void
PorousFlowSinglePhaseBase::act()
{
  PorousFlowActionBase::act();

  if ((_coupling_type == CouplingTypeEnum::HydroMechanical ||
       _coupling_type == CouplingTypeEnum::ThermoHydroMechanical) &&
      _current_task == "add_kernel")
  {
    for (unsigned i = 0; i < _ndisp; ++i)
    {
      std::string kernel_name = "PorousFlowUnsaturated_grad_stress" + Moose::stringify(i);
      std::string kernel_type = "StressDivergenceTensors";
      InputParameters params = _factory.getValidParams(kernel_type);
      params.set<NonlinearVariableName>("variable") = _displacements[i];
      params.set<std::vector<VariableName>>("displacements") = _coupled_displacements;
      params.set<unsigned>("component") = i;
      _problem->addKernel(kernel_type, kernel_name, params);

      if (_gravity(i) != 0)
      {
        kernel_name = "PorousFlowUnsaturated_gravity" + Moose::stringify(i);
        kernel_type = "Gravity";
        params = _factory.getValidParams(kernel_type);
        params.set<NonlinearVariableName>("variable") = _displacements[i];
        params.set<Real>("value") = _gravity(i);
        _problem->addKernel(kernel_type, kernel_name, params);
      }

      kernel_name = "PorousFlowUnsaturated_EffStressCoupling" + Moose::stringify(i);
      kernel_type = "PorousFlowEffectiveStressCoupling";
      params = _factory.getValidParams(kernel_type);
      params.set<UserObjectName>("PorousFlowDictator") = _dictator_name;
      params.set<NonlinearVariableName>("variable") = _displacements[i];
      params.set<Real>("biot_coefficient") = _biot_coefficient;
      params.set<unsigned>("component") = i;
      _problem->addKernel(kernel_type, kernel_name, params);
    }
  }

  if ((_coupling_type == CouplingTypeEnum::ThermoHydro ||
       _coupling_type == CouplingTypeEnum::ThermoHydroMechanical) &&
      _current_task == "add_kernel")
  {
    std::string kernel_name = "PorousFlowUnsaturated_HeatConduction";
    std::string kernel_type = "PorousFlowHeatConduction";
    InputParameters params = _factory.getValidParams(kernel_type);
    params.set<NonlinearVariableName>("variable") = _temperature_var[0];
    params.set<UserObjectName>("PorousFlowDictator") = _dictator_name;
    _problem->addKernel(kernel_type, kernel_name, params);

    if (_simulation_type == SimulationTypeChoiceEnum::TRANSIENT)
    {
      kernel_name = "PorousFlowUnsaturated_EnergyTimeDerivative";
      kernel_type = "PorousFlowEnergyTimeDerivative";
      params = _factory.getValidParams(kernel_type);
      params.set<NonlinearVariableName>("variable") = _temperature_var[0];
      params.set<UserObjectName>("PorousFlowDictator") = _dictator_name;
      _problem->addKernel(kernel_type, kernel_name, params);
    }
  }

  if (_coupling_type == CouplingTypeEnum::ThermoHydroMechanical &&
      _simulation_type == SimulationTypeChoiceEnum::TRANSIENT && _current_task == "add_kernel")
  {
    std::string kernel_name = "PorousFlowUnsaturated_HeatVolumetricExpansion";
    std::string kernel_type = "PorousFlowHeatVolumetricExpansion";
    InputParameters params = _factory.getValidParams(kernel_type);
    params.set<UserObjectName>("PorousFlowDictator") = _dictator_name;
    params.set<NonlinearVariableName>("variable") = _temperature_var[0];
    _problem->addKernel(kernel_type, kernel_name, params);
  }

  // add Materials
  if (_deps.dependsOn(_objects_to_add, "PorousFlowTemperature_qp"))
    addTemperatureMaterial(false);
  if (_deps.dependsOn(_objects_to_add, "PorousFlowTemperature_nodal"))
    addTemperatureMaterial(true);
  if (_deps.dependsOn(_objects_to_add, "PorousFlowMassFraction_qp"))
    addMassFractionMaterial(false);
  if (_deps.dependsOn(_objects_to_add, "PorousFlowMassFraction_nodal"))
    addMassFractionMaterial(true);

  const bool compute_rho_mu_qp = _deps.dependsOn(_objects_to_add, "PorousFlowDensity_qp") ||
                                 _deps.dependsOn(_objects_to_add, "PorousFlowViscosity_qp");
  const bool compute_e_qp = _deps.dependsOn(_objects_to_add, "PorousFlowInternalEnergy_qp");
  const bool compute_h_qp = _deps.dependsOn(_objects_to_add, "PorousFlowEnthalpy_qp");
  if (compute_rho_mu_qp || compute_e_qp || compute_h_qp)
    addSingleComponentFluidMaterial(false, 0, compute_rho_mu_qp, compute_e_qp, compute_h_qp, _fp);
  const bool compute_rho_mu_nodal = _deps.dependsOn(_objects_to_add, "PorousFlowDensity_nodal") ||
                                    _deps.dependsOn(_objects_to_add, "PorousFlowViscosity_nodal");
  const bool compute_e_nodal = _deps.dependsOn(_objects_to_add, "PorousFlowInternalEnergy_nodal");
  const bool compute_h_nodal = _deps.dependsOn(_objects_to_add, "PorousFlowEnthalpy_nodal");
  if (compute_rho_mu_nodal || compute_e_nodal || compute_h_nodal)
    addSingleComponentFluidMaterial(
        true, 0, compute_rho_mu_nodal, compute_e_nodal, compute_h_nodal, _fp);

  if (compute_rho_mu_qp)
  {
    joinDensity(false);
    joinViscosity(false);
  }
  if (compute_rho_mu_nodal)
  {
    joinDensity(true);
    joinViscosity(true);
  }
  if (compute_e_qp)
    joinInternalEnergy(false);
  if (compute_e_nodal)
    joinInternalEnergy(true);
  if (compute_h_qp)
    joinEnthalpy(false);
  if (compute_h_nodal)
    joinEnthalpy(true);

  if (_deps.dependsOn(_objects_to_add, "PorousFlowRelativePermeability_qp"))
    joinRelativePermeability(false);
  if (_deps.dependsOn(_objects_to_add, "PorousFlowRelativePermeability_nodal"))
    joinRelativePermeability(true);

  if (_deps.dependsOn(_objects_to_add, "PorousFlowEffectiveFluidPressure_qp"))
    addEffectiveFluidPressureMaterial(false);
  if (_deps.dependsOn(_objects_to_add, "PorousFlowEffectiveFluidPressure_nodal"))
    addEffectiveFluidPressureMaterial(true);

  // add AuxVariables and AuxKernels
  if (_add_darcy_aux)
    addDarcyAux(_gravity);
  if (_add_stress_aux && (_coupling_type == CouplingTypeEnum::HydroMechanical ||
                          _coupling_type == CouplingTypeEnum::ThermoHydroMechanical))
    addStressAux();
}

void
PorousFlowSinglePhaseBase::addDictator()
{
  std::string uo_name = _dictator_name;
  std::string uo_type = "PorousFlowDictator";
  InputParameters params = _factory.getValidParams(uo_type);
  std::vector<VariableName> pf_vars = _mass_fraction_vars;
  pf_vars.push_back(_pp_var);
  if (_coupling_type == CouplingTypeEnum::ThermoHydro ||
      _coupling_type == CouplingTypeEnum::ThermoHydroMechanical)
    pf_vars.push_back(_temperature_var[0]);
  if (_coupling_type == CouplingTypeEnum::HydroMechanical ||
      _coupling_type == CouplingTypeEnum::ThermoHydroMechanical)
    pf_vars.insert(pf_vars.end(), _coupled_displacements.begin(), _coupled_displacements.end());
  params.set<std::vector<VariableName>>("porous_flow_vars") = pf_vars;
  params.set<unsigned int>("number_fluid_phases") = 1;
  params.set<unsigned int>("number_fluid_components") = _num_mass_fraction_vars + 1;
  _problem->addUserObject(uo_type, uo_name, params);
}
