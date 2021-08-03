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

registerMooseAction("PorousFlowApp", PorousFlowUnsaturated, "add_user_object");

registerMooseAction("PorousFlowApp", PorousFlowUnsaturated, "add_kernel");

registerMooseAction("PorousFlowApp", PorousFlowUnsaturated, "add_material");

registerMooseAction("PorousFlowApp", PorousFlowUnsaturated, "add_aux_variable");

registerMooseAction("PorousFlowApp", PorousFlowUnsaturated, "add_aux_kernel");

InputParameters
PorousFlowUnsaturated::validParams()
{
  InputParameters params = PorousFlowSinglePhaseBase::validParams();
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
    _s_res(getParam<Real>("residual_saturation")),
    _capillary_pressure_name("PorousFlowUnsaturated_CapillaryPressureVG")
{
  if (_stabilization == StabilizationEnum::None)
    paramError("stabilization", "Some stabilization must be used in PorousFlowUnsaturated");
}

void
PorousFlowUnsaturated::addMaterialDependencies()
{
  PorousFlowSinglePhaseBase::addMaterialDependencies();

  // Add necessary objects to list of PorousFlow objects added by this action
  _included_objects.push_back("PorousFlowAdvectiveFlux");

  if (_transient)
    _included_objects.push_back("PorousFlowMassTimeDerivative");

  if (_mechanical && _transient)
    _included_objects.push_back("PorousFlowMassVolumetricExpansion");

  if (_thermal)
    _included_objects.push_back("PorousFlowHeatAdvection");

  if (_add_saturation_aux)
    _included_objects.push_back("SaturationAux");

  if (_stabilization == StabilizationEnum::KT)
    _included_objects.push_back("PorousFlowAdvectiveFluxCalculatorUnsaturatedMultiComponent");

  if (_stabilization == StabilizationEnum::KT && _thermal)
    _included_objects.push_back("PorousFlowAdvectiveFluxCalculatorUnsaturatedHeat");
}

void
PorousFlowUnsaturated::addKernels()
{
  PorousFlowSinglePhaseBase::addKernels();

  // add the kernels
  if (_stabilization == StabilizationEnum::Full)
  {
    const std::string kernel_type = "PorousFlowAdvectiveFlux";
    InputParameters params = _factory.getValidParams(kernel_type);
    params.set<UserObjectName>("PorousFlowDictator") = _dictator_name;
    params.set<RealVectorValue>("gravity") = _gravity;

    for (unsigned i = 0; i < _num_mass_fraction_vars; ++i)
    {
      const std::string kernel_name = "PorousFlowUnsaturated_AdvectiveFlux" + Moose::stringify(i);
      params.set<unsigned int>("fluid_component") = i;
      params.set<NonlinearVariableName>("variable") = _mass_fraction_vars[i];
      _problem->addKernel(kernel_type, kernel_name, params);
    }
    const std::string kernel_name =
        "PorousFlowUnsaturated_AdvectiveFlux" + Moose::stringify(_num_mass_fraction_vars);
    params.set<unsigned int>("fluid_component") = _num_mass_fraction_vars;
    params.set<NonlinearVariableName>("variable") = _pp_var;
    _problem->addKernel(kernel_type, kernel_name, params);
  }
  else if (_stabilization == StabilizationEnum::KT)
  {
    const std::string kernel_type = "PorousFlowFluxLimitedTVDAdvection";
    InputParameters params = _factory.getValidParams(kernel_type);
    params.set<UserObjectName>("PorousFlowDictator") = _dictator_name;

    for (unsigned i = 0; i < _num_mass_fraction_vars; ++i)
    {
      const std::string kernel_name = "PorousFlowFluxLimited_DarcyFlow" + Moose::stringify(i);
      params.set<UserObjectName>("advective_flux_calculator") =
          "PorousFlowUnsaturated_AC_" + Moose::stringify(i);
      params.set<NonlinearVariableName>("variable") = _mass_fraction_vars[i];
      _problem->addKernel(kernel_type, kernel_name, params);
    }
    const std::string kernel_name =
        "PorousFlowFluxLimited_DarcyFlow" + Moose::stringify(_num_mass_fraction_vars);
    params.set<NonlinearVariableName>("variable") = _pp_var;
    params.set<UserObjectName>("advective_flux_calculator") =
        "PorousFlowUnsaturated_AC_" + Moose::stringify(_num_mass_fraction_vars);
    _problem->addKernel(kernel_type, kernel_name, params);
  }

  if (_transient)
  {
    std::string kernel_name = "PorousFlowUnsaturated_MassTimeDerivative";
    std::string kernel_type = "PorousFlowMassTimeDerivative";
    InputParameters params = _factory.getValidParams(kernel_type);
    params.set<UserObjectName>("PorousFlowDictator") = _dictator_name;
    params.set<bool>("strain_at_nearest_qp") = _strain_at_nearest_qp;
    if (!_base_name.empty())
      params.set<std::string>("base_name") = _base_name;

    for (unsigned i = 0; i < _num_mass_fraction_vars; ++i)
    {
      kernel_name = "PorousFlowUnsaturated_MassTimeDerivative" + Moose::stringify(i);
      params.set<unsigned int>("fluid_component") = i;
      params.set<NonlinearVariableName>("variable") = _mass_fraction_vars[i];
      if (_save_component_rate_in.size() != 0)
        params.set<std::vector<AuxVariableName>>("save_in") = {_save_component_rate_in[i]};
      _problem->addKernel(kernel_type, kernel_name, params);
    }
    kernel_name =
        "PorousFlowUnsaturated_MassTimeDerivative" + Moose::stringify(_num_mass_fraction_vars);
    params.set<unsigned int>("fluid_component") = _num_mass_fraction_vars;
    params.set<NonlinearVariableName>("variable") = _pp_var;
    if (_save_component_rate_in.size() != 0)
      params.set<std::vector<AuxVariableName>>("save_in") = {
          _save_component_rate_in[_num_mass_fraction_vars]};
    _problem->addKernel(kernel_type, kernel_name, params);
  }

  if (_mechanical && _transient)
  {
    std::string kernel_name = "PorousFlowUnsaturated_MassVolumetricExpansion";
    std::string kernel_type = "PorousFlowMassVolumetricExpansion";
    InputParameters params = _factory.getValidParams(kernel_type);
    params.set<UserObjectName>("PorousFlowDictator") = _dictator_name;
    params.set<bool>("strain_at_nearest_qp") = _strain_at_nearest_qp;

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

  if (_thermal)
  {
    if (_stabilization == StabilizationEnum::Full)
    {
      const std::string kernel_name = "PorousFlowUnsaturated_HeatAdvection";
      const std::string kernel_type = "PorousFlowHeatAdvection";
      InputParameters params = _factory.getValidParams(kernel_type);
      params.set<NonlinearVariableName>("variable") = _temperature_var[0];
      params.set<UserObjectName>("PorousFlowDictator") = _dictator_name;
      params.set<RealVectorValue>("gravity") = _gravity;
      _problem->addKernel(kernel_type, kernel_name, params);
    }
    else if (_stabilization == StabilizationEnum::KT)
    {
      const std::string kernel_name = "PorousFlowUnsaturated_HeatAdvection";
      const std::string kernel_type = "PorousFlowFluxLimitedTVDAdvection";
      InputParameters params = _factory.getValidParams(kernel_type);
      params.set<NonlinearVariableName>("variable") = _temperature_var[0];
      params.set<UserObjectName>("PorousFlowDictator") = _dictator_name;
      params.set<UserObjectName>("advective_flux_calculator") = "PorousFlowUnsaturatedHeat_AC";
      _problem->addKernel(kernel_type, kernel_name, params);
    }
  }
}

void
PorousFlowUnsaturated::addUserObjects()
{
  PorousFlowSinglePhaseBase::addUserObjects();

  // Add the capillary pressure UserObject
  addCapillaryPressureVG(_van_genuchten_m, _van_genuchten_alpha, _capillary_pressure_name);

  // add Advective Flux calculator UserObjects, if required
  if (_stabilization == StabilizationEnum::KT)
  {
    for (unsigned i = 0; i < _num_mass_fraction_vars; ++i)
    {
      const std::string userobject_name = "PorousFlowUnsaturated_AC_" + Moose::stringify(i);
      addAdvectiveFluxCalculatorUnsaturatedMultiComponent(0, i, true, userobject_name);
    }
    const std::string userobject_name =
        "PorousFlowUnsaturated_AC_" + Moose::stringify(_num_mass_fraction_vars);
    if (_num_mass_fraction_vars == 0)
      addAdvectiveFluxCalculatorUnsaturated(0, true, userobject_name); // 1 component only
    else
      addAdvectiveFluxCalculatorUnsaturatedMultiComponent(
          0, _num_mass_fraction_vars, true, userobject_name);

    if (_thermal)
    {
      const std::string userobject_name = "PorousFlowUnsaturatedHeat_AC";
      addAdvectiveFluxCalculatorUnsaturatedHeat(0, true, userobject_name);
    }
  }
}

void
PorousFlowUnsaturated::addMaterials()
{
  PorousFlowSinglePhaseBase::addMaterials();

  if (_deps.dependsOn(_included_objects, "pressure_saturation_qp"))
  {
    const std::string material_type = "PorousFlow1PhaseP";
    InputParameters params = _factory.getValidParams(material_type);
    const std::string material_name = "PorousFlowUnsaturated_1PhaseP_VG_qp";
    params.set<UserObjectName>("PorousFlowDictator") = _dictator_name;
    params.set<std::vector<VariableName>>("porepressure") = {_pp_var};
    params.set<UserObjectName>("capillary_pressure") = _capillary_pressure_name;
    params.set<bool>("at_nodes") = false;
    _problem->addMaterial(material_type, material_name, params);
  }
  if (_deps.dependsOn(_included_objects, "pressure_saturation_nodal"))
  {
    const std::string material_type = "PorousFlow1PhaseP";
    InputParameters params = _factory.getValidParams(material_type);
    const std::string material_name = "PorousFlowUnsaturated_1PhaseP_VG_nodal";
    params.set<UserObjectName>("PorousFlowDictator") = _dictator_name;
    params.set<std::vector<VariableName>>("porepressure") = {_pp_var};
    params.set<UserObjectName>("capillary_pressure") = _capillary_pressure_name;
    params.set<bool>("at_nodes") = true;
    _problem->addMaterial(material_type, material_name, params);
  }

  if (_deps.dependsOn(_included_objects, "relative_permeability_qp"))
  {
    if (_relperm_type == RelpermTypeChoiceEnum::FLAC)
      addRelativePermeabilityFLAC(false, 0, _relative_permeability_exponent, _s_res, _s_res);
    else
      addRelativePermeabilityCorey(false, 0, _relative_permeability_exponent, _s_res, _s_res);
  }

  if (_deps.dependsOn(_included_objects, "relative_permeability_nodal"))
  {
    if (_relperm_type == RelpermTypeChoiceEnum::FLAC)
      addRelativePermeabilityFLAC(true, 0, _relative_permeability_exponent, _s_res, _s_res);
    else
      addRelativePermeabilityCorey(true, 0, _relative_permeability_exponent, _s_res, _s_res);
  }

  if (_deps.dependsOn(_included_objects, "volumetric_strain_qp") ||
      _deps.dependsOn(_included_objects, "volumetric_strain_nodal"))
    addVolumetricStrainMaterial(_coupled_displacements, _base_name);
}

void
PorousFlowUnsaturated::addAuxObjects()
{
  PorousFlowSinglePhaseBase::addAuxObjects();

  if (_add_saturation_aux)
    addSaturationAux(0);
}
