//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowFullySaturated.h"

#include "FEProblem.h"
#include "Conversion.h"
#include "libmesh/string_to_enum.h"

registerMooseAction("PorousFlowApp", PorousFlowFullySaturated, "add_user_object");

registerMooseAction("PorousFlowApp", PorousFlowFullySaturated, "add_kernel");

registerMooseAction("PorousFlowApp", PorousFlowFullySaturated, "add_material");

registerMooseAction("PorousFlowApp", PorousFlowFullySaturated, "add_aux_variable");

registerMooseAction("PorousFlowApp", PorousFlowFullySaturated, "add_aux_kernel");

InputParameters
PorousFlowFullySaturated::validParams()
{
  InputParameters params = PorousFlowSinglePhaseBase::validParams();
  params.addParam<bool>(
      "multiply_by_density",
      true,
      "If true, then the Kernels for fluid flow are multiplied by "
      "the fluid density.  If false, this multiplication is not "
      "performed, which means the problem becomes more linear, but care must be taken when using "
      "other PorousFlow objects, since MOOSE will be computing volume fluxes, not mass fluxes.");
  params.addClassDescription(
      "Adds Kernels and fluid-property Materials necessary to simulate a "
      "single-phase fully-saturated flow problem.  Numerical stabilization options for the fluid "
      "and heat flow are: no upwinding, full-upwinding or KT stabilization.  No Kernels for "
      "diffusion and dispersion of "
      "fluid components are added.  To run a simulation you will also "
      "need to provide various other Materials for each mesh "
      "block, depending on your simulation type, viz: permeability, "
      "porosity, elasticity tensor, strain calculator, stress calculator, "
      "matrix internal energy, thermal conductivity, diffusivity");
  return params;
}

PorousFlowFullySaturated::PorousFlowFullySaturated(const InputParameters & params)
  : PorousFlowSinglePhaseBase(params), _multiply_by_density(getParam<bool>("multiply_by_density"))
{
}

void
PorousFlowFullySaturated::addMaterialDependencies()
{
  PorousFlowSinglePhaseBase::addMaterialDependencies();

  // Add necessary objects to list of PorousFlow objects added by this action
  if (_stabilization == StabilizationEnum::None)
    _included_objects.push_back("PorousFlowFullySaturatedDarcyFlow");
  else if (_stabilization == StabilizationEnum::Full)
    _included_objects.push_back("PorousFlowFullySaturatedAdvectiveFlux");
  else if (_stabilization == StabilizationEnum::KT)
    _included_objects.push_back("PorousFlowFluxLimitedTVDAdvection");

  if (_transient)
    _included_objects.push_back("PorousFlowMassTimeDerivative");

  if (_mechanical && _transient)
    _included_objects.push_back("PorousFlowMassVolumetricExpansion");

  if (_thermal)
  {
    if (_stabilization == StabilizationEnum::None)
      _included_objects.push_back("PorousFlowFullySaturatedHeatAdvection");
    else if (_stabilization == StabilizationEnum::Full)
      _included_objects.push_back("PorousFlowFullySaturatedUpwindHeatAdvection");
    else if (_stabilization == StabilizationEnum::KT)
      _included_objects.push_back("PorousFlowFluxLimitedTVDAdvection");
  }

  if (_stabilization == StabilizationEnum::KT && _thermal)
    _included_objects.push_back("PorousFlowAdvectiveFluxCalculatorSaturatedHeat");

  if (_stabilization == StabilizationEnum::KT)
    _included_objects.push_back("PorousFlowAdvectiveFluxCalculatorSaturatedMultiComponent");

  if (_stabilization == StabilizationEnum::KT && _thermal)
    _included_objects.push_back("PorousFlowAdvectiveFluxCalculatorSaturatedHeat");
}

void
PorousFlowFullySaturated::addKernels()
{
  PorousFlowSinglePhaseBase::addKernels();

  if (_stabilization == StabilizationEnum::None)
  {
    const std::string kernel_type = "PorousFlowFullySaturatedDarcyFlow";
    InputParameters params = _factory.getValidParams(kernel_type);
    params.set<UserObjectName>("PorousFlowDictator") = _dictator_name;
    params.set<RealVectorValue>("gravity") = _gravity;
    params.set<bool>("multiply_by_density") = _multiply_by_density;

    for (unsigned i = 0; i < _num_mass_fraction_vars; ++i)
    {
      const std::string kernel_name = "PorousFlowFullySaturated_DarcyFlow" + Moose::stringify(i);
      params.set<unsigned int>("fluid_component") = i;
      params.set<NonlinearVariableName>("variable") = _mass_fraction_vars[i];
      _problem->addKernel(kernel_type, kernel_name, params);
    }

    const std::string kernel_name =
        "PorousFlowFullySaturated_DarcyFlow" + Moose::stringify(_num_mass_fraction_vars);
    params.set<unsigned int>("fluid_component") = _num_mass_fraction_vars;
    params.set<NonlinearVariableName>("variable") = _pp_var;
    _problem->addKernel(kernel_type, kernel_name, params);
  }
  else if (_stabilization == StabilizationEnum::Full)
  {
    const std::string kernel_type = "PorousFlowFullySaturatedAdvectiveFlux";
    InputParameters params = _factory.getValidParams(kernel_type);
    params.set<UserObjectName>("PorousFlowDictator") = _dictator_name;
    params.set<RealVectorValue>("gravity") = _gravity;
    params.set<bool>("multiply_by_density") = _multiply_by_density;

    for (unsigned i = 0; i < _num_mass_fraction_vars; ++i)
    {
      const std::string kernel_name =
          "PorousFlowFullySaturated_AdvectiveFlux" + Moose::stringify(i);
      params.set<unsigned int>("fluid_component") = i;
      params.set<NonlinearVariableName>("variable") = _mass_fraction_vars[i];
      _problem->addKernel(kernel_type, kernel_name, params);
    }

    const std::string kernel_name =
        "PorousFlowFullySaturated_AdvectiveFlux" + Moose::stringify(_num_mass_fraction_vars);
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
          "PorousFlowFullySaturated_AC_" + Moose::stringify(i);
      params.set<NonlinearVariableName>("variable") = _mass_fraction_vars[i];
      _problem->addKernel(kernel_type, kernel_name, params);
    }

    const std::string kernel_name =
        "PorousFlowFluxLimited_DarcyFlow" + Moose::stringify(_num_mass_fraction_vars);
    params.set<NonlinearVariableName>("variable") = _pp_var;
    params.set<UserObjectName>("advective_flux_calculator") =
        "PorousFlowFullySaturated_AC_" + Moose::stringify(_num_mass_fraction_vars);
    _problem->addKernel(kernel_type, kernel_name, params);
  }

  if (_transient)
  {
    std::string kernel_name = "PorousFlowFullySaturated_MassTimeDerivative";
    std::string kernel_type = "PorousFlowMassTimeDerivative";
    InputParameters params = _factory.getValidParams(kernel_type);
    params.set<UserObjectName>("PorousFlowDictator") = _dictator_name;
    params.set<bool>("multiply_by_density") = _multiply_by_density;
    params.set<bool>("strain_at_nearest_qp") = _strain_at_nearest_qp;
    if (!_base_name.empty())
      params.set<std::string>("base_name") = _base_name;

    for (unsigned i = 0; i < _num_mass_fraction_vars; ++i)
    {
      kernel_name = "PorousFlowFullySaturated_MassTimeDerivative" + Moose::stringify(i);
      params.set<unsigned int>("fluid_component") = i;
      params.set<NonlinearVariableName>("variable") = _mass_fraction_vars[i];
      if (_save_component_rate_in.size() != 0)
        params.set<std::vector<AuxVariableName>>("save_in") = {_save_component_rate_in[i]};
      _problem->addKernel(kernel_type, kernel_name, params);
    }

    kernel_name =
        "PorousFlowFullySaturated_MassTimeDerivative" + Moose::stringify(_num_mass_fraction_vars);
    params.set<unsigned int>("fluid_component") = _num_mass_fraction_vars;
    params.set<NonlinearVariableName>("variable") = _pp_var;
    if (_save_component_rate_in.size() != 0)
      params.set<std::vector<AuxVariableName>>("save_in") = {
          _save_component_rate_in[_num_mass_fraction_vars]};
    _problem->addKernel(kernel_type, kernel_name, params);
  }

  if (_mechanical && _transient)
  {
    std::string kernel_name = "PorousFlowFullySaturated_MassVolumetricExpansion";
    std::string kernel_type = "PorousFlowMassVolumetricExpansion";
    InputParameters params = _factory.getValidParams(kernel_type);
    params.set<UserObjectName>("PorousFlowDictator") = _dictator_name;
    params.set<bool>("multiply_by_density") = _multiply_by_density;
    params.set<bool>("strain_at_nearest_qp") = _strain_at_nearest_qp;

    for (unsigned i = 0; i < _num_mass_fraction_vars; ++i)
    {
      kernel_name = "PorousFlowFullySaturated_MassVolumetricExpansion" + Moose::stringify(i);
      params.set<unsigned>("fluid_component") = i;
      params.set<NonlinearVariableName>("variable") = _mass_fraction_vars[i];
      _problem->addKernel(kernel_type, kernel_name, params);
    }

    kernel_name = "PorousFlowFullySaturated_MassVolumetricExpansion" +
                  Moose::stringify(_num_mass_fraction_vars);
    params.set<unsigned>("fluid_component") = _num_mass_fraction_vars;
    params.set<NonlinearVariableName>("variable") = _pp_var;
    _problem->addKernel(kernel_type, kernel_name, params);
  }

  if (_thermal)
  {
    if (_stabilization == StabilizationEnum::None)
    {
      std::string kernel_name = "PorousFlowFullySaturated_HeatAdvection";
      std::string kernel_type = "PorousFlowFullySaturatedHeatAdvection";
      InputParameters params = _factory.getValidParams(kernel_type);
      params.set<NonlinearVariableName>("variable") = _temperature_var[0];
      params.set<UserObjectName>("PorousFlowDictator") = _dictator_name;
      params.set<RealVectorValue>("gravity") = _gravity;
      params.set<bool>("multiply_by_density") = true;
      _problem->addKernel(kernel_type, kernel_name, params);
    }
    else if (_stabilization == StabilizationEnum::Full)
    {
      std::string kernel_name = "PorousFlowFullySaturatedUpwind_HeatAdvection";
      std::string kernel_type = "PorousFlowFullySaturatedUpwindHeatAdvection";
      InputParameters params = _factory.getValidParams(kernel_type);
      params.set<NonlinearVariableName>("variable") = _temperature_var[0];
      params.set<UserObjectName>("PorousFlowDictator") = _dictator_name;
      params.set<RealVectorValue>("gravity") = _gravity;
      _problem->addKernel(kernel_type, kernel_name, params);
    }
    else if (_stabilization == StabilizationEnum::KT)
    {
      const std::string kernel_name = "PorousFlowFullySaturated_HeatAdvection";
      const std::string kernel_type = "PorousFlowFluxLimitedTVDAdvection";
      InputParameters params = _factory.getValidParams(kernel_type);
      params.set<NonlinearVariableName>("variable") = _temperature_var[0];
      params.set<UserObjectName>("PorousFlowDictator") = _dictator_name;
      params.set<UserObjectName>("advective_flux_calculator") = "PorousFlowFullySaturatedHeat_AC";
      _problem->addKernel(kernel_type, kernel_name, params);
    }
  }
}

void
PorousFlowFullySaturated::addUserObjects()
{
  PorousFlowSinglePhaseBase::addUserObjects();

  // add Advective Flux calculator UserObjects, if required
  if (_stabilization == StabilizationEnum::KT)
  {
    for (unsigned i = 0; i < _num_mass_fraction_vars; ++i)
    {
      const std::string userobject_name = "PorousFlowFullySaturated_AC_" + Moose::stringify(i);
      addAdvectiveFluxCalculatorSaturatedMultiComponent(
          0, i, _multiply_by_density, userobject_name);
    }

    const std::string userobject_name =
        "PorousFlowFullySaturated_AC_" + Moose::stringify(_num_mass_fraction_vars);

    if (_num_mass_fraction_vars == 0)
      addAdvectiveFluxCalculatorSaturated(
          0, _multiply_by_density, userobject_name); // 1 component only
    else
      addAdvectiveFluxCalculatorSaturatedMultiComponent(
          0, _num_mass_fraction_vars, _multiply_by_density, userobject_name);

    if (_thermal)
    {
      const std::string userobject_name = "PorousFlowFullySaturatedHeat_AC";
      addAdvectiveFluxCalculatorSaturatedHeat(0, true, userobject_name);
    }
  }
}

void
PorousFlowFullySaturated::addMaterials()
{
  PorousFlowSinglePhaseBase::addMaterials();

  // add Materials
  if (_deps.dependsOn(_included_objects, "pressure_saturation_qp"))
  {
    // saturation is always unity, so is trivially calculated using PorousFlow1PhaseFullySaturated
    std::string material_type = "PorousFlow1PhaseFullySaturated";
    std::string material_name = "PorousFlowFullySaturated_1PhaseP_qp";
    InputParameters params = _factory.getValidParams(material_type);
    if (_subdomain_names_set)
      params.set<std::vector<SubdomainName>>("block") = _subdomain_names;
    params.set<UserObjectName>("PorousFlowDictator") = _dictator_name;
    params.set<std::vector<VariableName>>("porepressure") = {_pp_var};
    params.set<bool>("at_nodes") = false;
    _problem->addMaterial(material_type, material_name, params);
  }

  if (_deps.dependsOn(_included_objects, "pressure_saturation_nodal"))
  {
    std::string material_type = "PorousFlow1PhaseFullySaturated";
    std::string material_name = "PorousFlowFullySaturated_1PhaseP";
    InputParameters params = _factory.getValidParams(material_type);
    if (_subdomain_names_set)
      params.set<std::vector<SubdomainName>>("block") = _subdomain_names;
    params.set<UserObjectName>("PorousFlowDictator") = _dictator_name;
    params.set<std::vector<VariableName>>("porepressure") = {_pp_var};
    params.set<bool>("at_nodes") = true;
    _problem->addMaterial(material_type, material_name, params);
  }

  if (_deps.dependsOn(_included_objects, "volumetric_strain_qp") ||
      _deps.dependsOn(_included_objects, "volumetric_strain_nodal"))
    addVolumetricStrainMaterial(_coupled_displacements, _base_name);

  // Relative permeability might be needed by Darcy-velocity Aux, so add a material
  // setting kr=1
  if (_deps.dependsOn(_included_objects, "relative_permeability_qp"))
    addRelativePermeabilityConst(false, 0, 1.0);

  // Some obects not added by this action might have a use_mobility = true param,
  // which needs a nodal relative permeability
  if (_deps.dependsOn(_included_objects, "relative_permeability_nodal"))
    addRelativePermeabilityConst(true, 0, 1.0);
}
