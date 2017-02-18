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
    _biot_coefficient(getParam<Real>("biot_coefficient"))
{
  if ((_coupling_type == ThermoHydro || _coupling_type == ThermoHydroMechanical) &&
      _temperature_var.size() != 1)
    mooseError("PorousFlowSinglePhaseBase: You need to specify a temperature variable to perform "
               "non-isothermal simulations");
}

void
PorousFlowSinglePhaseBase::act()
{
  PorousFlowActionBase::act();

  if ((_coupling_type == HydroMechanical || _coupling_type == ThermoHydroMechanical) &&
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
      params.set<std::vector<SubdomainName>>("block") = {};
      _problem->addKernel(kernel_type, kernel_name, params);

      if (_gravity(i) != 0)
      {
        kernel_name = "PorousFlowUnsaturated_gravity" + Moose::stringify(i);
        kernel_type = "Gravity";
        params = _factory.getValidParams(kernel_type);
        params.set<NonlinearVariableName>("variable") = _displacements[i];
        params.set<Real>("value") = _gravity(i);
        params.set<std::vector<SubdomainName>>("block") = {};
        _problem->addKernel(kernel_type, kernel_name, params);
      }

      kernel_name = "PorousFlowUnsaturated_EffStressCoupling" + Moose::stringify(i);
      kernel_type = "PorousFlowEffectiveStressCoupling";
      params = _factory.getValidParams(kernel_type);
      params.set<UserObjectName>("PorousFlowDictator") = _dictator_name;
      params.set<NonlinearVariableName>("variable") = _displacements[i];
      params.set<Real>("biot_coefficient") = _biot_coefficient;
      params.set<unsigned>("component") = i;
      params.set<std::vector<SubdomainName>>("block") = {};
      _problem->addKernel(kernel_type, kernel_name, params);
    }
  }

  if ((_coupling_type == ThermoHydro || _coupling_type == ThermoHydroMechanical) &&
      _current_task == "add_kernel")
  {
    std::string kernel_name = "PorousFlowUnsaturated_HeatConduction";
    std::string kernel_type = "PorousFlowHeatConduction";
    InputParameters params = _factory.getValidParams(kernel_type);
    params.set<NonlinearVariableName>("variable") = _temperature_var[0];
    params.set<UserObjectName>("PorousFlowDictator") = _dictator_name;
    _problem->addKernel(kernel_type, kernel_name, params);

    if (_simulation_type == TRANSIENT)
    {
      kernel_name = "PorousFlowUnsaturated_EnergyTimeDerivative";
      kernel_type = "PorousFlowEnergyTimeDerivative";
      params = _factory.getValidParams(kernel_type);
      params.set<NonlinearVariableName>("variable") = _temperature_var[0];
      params.set<UserObjectName>("PorousFlowDictator") = _dictator_name;
      _problem->addKernel(kernel_type, kernel_name, params);
    }
  }

  if (_coupling_type == ThermoHydroMechanical && _simulation_type == TRANSIENT &&
      _current_task == "add_kernel")
  {
    std::string kernel_name = "PorousFlowUnsaturated_HeatVolumetricExpansion";
    std::string kernel_type = "PorousFlowHeatVolumetricExpansion";
    InputParameters params = _factory.getValidParams(kernel_type);
    params.set<UserObjectName>("PorousFlowDictator") = _dictator_name;
    params.set<std::vector<SubdomainName>>("block") = {};
    params.set<NonlinearVariableName>("variable") = _temperature_var[0];
    _problem->addKernel(kernel_type, kernel_name, params);
  }

  // add Materials
  addTemperatureMaterial();
  addMassFractionMaterial();
  addSingleComponentFluidMaterial(0, _fp);
  joinDensity(false);
  joinDensity(true);
  joinViscosity(false);
  joinViscosity(true);
  joinRelativePermeability(true);
  joinRelativePermeability(false);

  if (_coupling_type == ThermoHydro || _coupling_type == ThermoHydroMechanical)
  {
    joinInternalEnergy(true);
    joinInternalEnergy(false);
    joinEnthalpy(true);
    joinEnthalpy(false);
  }
  if (_coupling_type == HydroMechanical || _coupling_type == ThermoHydroMechanical)
    addEffectiveFluidPressureMaterial();

  // add AuxVariables and AuxKernels
  addDarcyAux(_gravity);
  if (_coupling_type == HydroMechanical || _coupling_type == ThermoHydroMechanical)
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
  if (_coupling_type == ThermoHydro || _coupling_type == ThermoHydroMechanical)
    pf_vars.push_back(_temperature_var[0]);
  if (_coupling_type == HydroMechanical || _coupling_type == ThermoHydroMechanical)
    pf_vars.insert(pf_vars.end(), _coupled_displacements.begin(), _coupled_displacements.end());
  params.set<std::vector<VariableName>>("porous_flow_vars") = pf_vars;
  params.set<unsigned int>("number_fluid_phases") = 1;
  params.set<unsigned int>("number_fluid_components") = _num_mass_fraction_vars + 1;
  _problem->addUserObject(uo_type, uo_name, params);
}
