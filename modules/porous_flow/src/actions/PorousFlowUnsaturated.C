//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowUnsaturated.h"

#include "FEProblem.h"
#include "Conversion.h"
#include "libmesh/string_to_enum.h"

template <>
InputParameters
validParams<PorousFlowUnsaturated>()
{
  InputParameters params = validParams<PorousFlowSinglePhaseBase>();
  params.addParam<bool>("add_saturation_aux", true, "Add an AuxVariable that records saturation");
  params.addRangeCheckedParam<Real>(
      "van_genuchten_alpha",
      1.0E-6,
      "van_genuchten_alpha > 0.0",
      "Van Genuchten alpha parameter used to determine saturation from porepressure");
  params.addRangeCheckedParam<Real>(
      "van_genuchten_m",
      0.6,
      "van_genuchten_m > 0 & van_genuchten_m < 1",
      "Van Genuchten m parameter used to determine saturation from porepressure");
  MooseEnum relperm_type_choice("FLAC Corey", "FLAC");
  params.addParam<MooseEnum>("relative_permeability_type",
                             relperm_type_choice,
                             "Type of relative-permeability function.  FLAC relperm = (1+m)S^m - "
                             "mS^(1+m).  Corey relperm = S^m.  m is the exponent.  Here S = "
                             "(saturation - residual)/(1 - residual)");
  params.addRangeCheckedParam<Real>("relative_permeability_exponent",
                                    3.0,
                                    "relative_permeability_exponent>=0",
                                    "Relative permeability exponent");
  params.addRangeCheckedParam<Real>(
      "residual_saturation",
      0.0,
      "residual_saturation>=0.0 & residual_saturation<1.0",
      "Residual saturation to use in the relative permeability expression");
  params.addClassDescription("Adds Kernels and fluid-property Materials necessary to simulate a "
                             "single-phase saturated-unsaturated flow problem.  The saturation is "
                             "computed using van Genuchten's expression.  No Kernels for diffusion "
                             "and dispersion of fluid components are added.  To run a simulation "
                             "you will also need to provide various other Materials for each mesh "
                             "block, depending on your simulation type, viz: permeability, "
                             "porosity, elasticity tensor, strain calculator, stress calculator, "
                             "matrix internal energy, thermal conductivity, diffusivity");
  return params;
}

PorousFlowUnsaturated::PorousFlowUnsaturated(const InputParameters & params)
  : PorousFlowSinglePhaseBase(params),
    _add_saturation_aux(getParam<bool>("add_saturation_aux")),
    _van_genuchten_alpha(getParam<Real>("van_genuchten_alpha")),
    _van_genuchten_m(getParam<Real>("van_genuchten_m")),
    _relperm_type(
        getParam<MooseEnum>("relative_permeability_type").getEnum<RelpermTypeChoiceEnum>()),
    _relative_permeability_exponent(getParam<Real>("relative_permeability_exponent")),
    _s_res(getParam<Real>("residual_saturation"))
{
  _objects_to_add.push_back("PorousFlowAdvectiveFlux");
  if (_simulation_type == SimulationTypeChoiceEnum::TRANSIENT)
    _objects_to_add.push_back("PorousFlowMassTimeDerivative");
  if ((_coupling_type == CouplingTypeEnum::HydroMechanical ||
       _coupling_type == CouplingTypeEnum::ThermoHydroMechanical) &&
      _simulation_type == SimulationTypeChoiceEnum::TRANSIENT)
    _objects_to_add.push_back("PorousFlowMassVolumetricExpansion");
  if (_coupling_type == CouplingTypeEnum::ThermoHydro ||
      _coupling_type == CouplingTypeEnum::ThermoHydroMechanical)
    _objects_to_add.push_back("PorousFlowHeatAdvection");
  if (_add_saturation_aux)
    _objects_to_add.push_back("SaturationAux");
}

void
PorousFlowUnsaturated::act()
{
  PorousFlowSinglePhaseBase::act();

  // add the kernels
  if (_current_task == "add_kernel")
  {
    std::string kernel_name = "PorousFlowUnsaturated_AdvectiveFlux";
    std::string kernel_type = "PorousFlowAdvectiveFlux";
    InputParameters params = _factory.getValidParams(kernel_type);
    params.set<UserObjectName>("PorousFlowDictator") = _dictator_name;
    params.set<RealVectorValue>("gravity") = _gravity;

    for (unsigned i = 0; i < _num_mass_fraction_vars; ++i)
    {
      kernel_name = "PorousFlowUnsaturated_AdvectiveFlux" + Moose::stringify(i);
      params.set<unsigned int>("fluid_component") = i;
      params.set<NonlinearVariableName>("variable") = _mass_fraction_vars[i];
      _problem->addKernel(kernel_type, kernel_name, params);
    }
    kernel_name = "PorousFlowUnsaturated_AdvectiveFlux" + Moose::stringify(_num_mass_fraction_vars);
    params.set<unsigned int>("fluid_component") = _num_mass_fraction_vars;
    params.set<NonlinearVariableName>("variable") = _pp_var;
    _problem->addKernel(kernel_type, kernel_name, params);
  }
  if (_current_task == "add_kernel" && _simulation_type == SimulationTypeChoiceEnum::TRANSIENT)
  {
    std::string kernel_name = "PorousFlowUnsaturated_MassTimeDerivative";
    std::string kernel_type = "PorousFlowMassTimeDerivative";
    InputParameters params = _factory.getValidParams(kernel_type);
    params.set<UserObjectName>("PorousFlowDictator") = _dictator_name;

    for (unsigned i = 0; i < _num_mass_fraction_vars; ++i)
    {
      kernel_name = "PorousFlowUnsaturated_MassTimeDerivative" + Moose::stringify(i);
      params.set<unsigned int>("fluid_component") = i;
      params.set<NonlinearVariableName>("variable") = _mass_fraction_vars[i];
      _problem->addKernel(kernel_type, kernel_name, params);
    }
    kernel_name =
        "PorousFlowUnsaturated_MassTimeDerivative" + Moose::stringify(_num_mass_fraction_vars);
    params.set<unsigned int>("fluid_component") = _num_mass_fraction_vars;
    params.set<NonlinearVariableName>("variable") = _pp_var;
    _problem->addKernel(kernel_type, kernel_name, params);
  }

  if ((_coupling_type == CouplingTypeEnum::HydroMechanical ||
       _coupling_type == CouplingTypeEnum::ThermoHydroMechanical) &&
      _current_task == "add_kernel" && _simulation_type == SimulationTypeChoiceEnum::TRANSIENT)
  {
    std::string kernel_name = "PorousFlowUnsaturated_MassVolumetricExpansion";
    std::string kernel_type = "PorousFlowMassVolumetricExpansion";
    InputParameters params = _factory.getValidParams(kernel_type);
    params.set<UserObjectName>("PorousFlowDictator") = _dictator_name;
    for (unsigned i = 0; i < _num_mass_fraction_vars; ++i)
    {
      kernel_name = "PorousFlowUnsaturated_MassVolumetricExpansion" + Moose::stringify(i);
      params.set<unsigned>("fluid_component") = i;
      params.set<NonlinearVariableName>("variable") = _mass_fraction_vars[i];
      _problem->addKernel(kernel_type, kernel_name, params);
    }
    kernel_name =
        "PorousFlowUnsaturated_MassVolumetricExpansion" + Moose::stringify(_num_mass_fraction_vars);
    params.set<unsigned>("fluid_component") = _num_mass_fraction_vars;
    params.set<NonlinearVariableName>("variable") = _pp_var;
    _problem->addKernel(kernel_type, kernel_name, params);
  }

  if ((_coupling_type == CouplingTypeEnum::ThermoHydro ||
       _coupling_type == CouplingTypeEnum::ThermoHydroMechanical) &&
      _current_task == "add_kernel")
  {
    std::string kernel_name = "PorousFlowUnsaturated_HeatAdvection";
    std::string kernel_type = "PorousFlowHeatAdvection";
    InputParameters params = _factory.getValidParams(kernel_type);
    params.set<NonlinearVariableName>("variable") = _temperature_var[0];
    params.set<UserObjectName>("PorousFlowDictator") = _dictator_name;
    params.set<RealVectorValue>("gravity") = _gravity;
    _problem->addKernel(kernel_type, kernel_name, params);
  }

  // Add the capillary pressure UserObject
  std::string capillary_pressure_name = "PorousFlowUnsaturated_CapillaryPressureVG";
  addCapillaryPressureVG(_van_genuchten_m, _van_genuchten_alpha, capillary_pressure_name);

  if (_deps.dependsOn(_objects_to_add, "PorousFlowPS_qp") && _current_task == "add_material")
  {
    std::string material_type = "PorousFlow1PhaseP";
    InputParameters params = _factory.getValidParams(material_type);

    std::string material_name = "PorousFlowUnsaturated_1PhaseP_VG_qp";
    params.set<UserObjectName>("PorousFlowDictator") = _dictator_name;
    params.set<std::vector<VariableName>>("porepressure") = {_pp_var};
    params.set<UserObjectName>("capillary_pressure") = capillary_pressure_name;
    _problem->addMaterial(material_type, material_name, params);
  }
  if (_deps.dependsOn(_objects_to_add, "PorousFlowPS_nodal") && _current_task == "add_material")
  {
    std::string material_type = "PorousFlow1PhaseP";
    InputParameters params = _factory.getValidParams(material_type);

    std::string material_name = "PorousFlowUnsaturated_1PhaseP_VG_nodal";
    params.set<UserObjectName>("PorousFlowDictator") = _dictator_name;
    params.set<std::vector<VariableName>>("porepressure") = {_pp_var};
    params.set<UserObjectName>("capillary_pressure") = capillary_pressure_name;
    params.set<bool>("at_nodes") = true;
    _problem->addMaterial(material_type, material_name, params);
  }

  if (_deps.dependsOn(_objects_to_add, "PorousFlowRelativePermeability_qp"))
  {
    if (_relperm_type == RelpermTypeChoiceEnum::FLAC)
      addRelativePermeabilityFLAC(false, 0, _relative_permeability_exponent, _s_res, _s_res);
    else
      addRelativePermeabilityCorey(false, 0, _relative_permeability_exponent, _s_res, _s_res);
  }
  if (_deps.dependsOn(_objects_to_add, "PorousFlowRelativePermeability_nodal"))
  {
    if (_relperm_type == RelpermTypeChoiceEnum::FLAC)
      addRelativePermeabilityFLAC(true, 0, _relative_permeability_exponent, _s_res, _s_res);
    else
      addRelativePermeabilityCorey(true, 0, _relative_permeability_exponent, _s_res, _s_res);
  }

  if (_deps.dependsOn(_objects_to_add, "PorousFlowVolumetricStrain_qp") ||
      _deps.dependsOn(_objects_to_add, "PorousFlowVolumetricStrain_nodal"))
    addVolumetricStrainMaterial(_coupled_displacements, true);

  // add relevant AuxVariables and AuxKernels
  if (_add_saturation_aux)
    addSaturationAux(0);
}
