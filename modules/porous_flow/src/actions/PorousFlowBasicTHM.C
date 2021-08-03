//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowBasicTHM.h"

#include "FEProblem.h"
#include "Conversion.h"
#include "libmesh/string_to_enum.h"

registerMooseAction("PorousFlowApp", PorousFlowBasicTHM, "add_user_object");

registerMooseAction("PorousFlowApp", PorousFlowBasicTHM, "add_kernel");

registerMooseAction("PorousFlowApp", PorousFlowBasicTHM, "add_material");

registerMooseAction("PorousFlowApp", PorousFlowBasicTHM, "add_aux_variable");

registerMooseAction("PorousFlowApp", PorousFlowBasicTHM, "add_aux_kernel");

InputParameters
PorousFlowBasicTHM::validParams()
{
  InputParameters params = PorousFlowSinglePhaseBase::validParams();
  params.addParam<bool>("multiply_by_density",
                        false,
                        "If true, then the Kernels for fluid flow are multiplied by "
                        "the fluid density.  If false, this multiplication is not "
                        "performed, which means the problem linearises, but that care "
                        "must be taken when using other PorousFlow objects.");
  params.addClassDescription("Adds Kernels and fluid-property Materials necessary to simulate a "
                             "single-phase, single-component fully-saturated flow problem.  No "
                             "upwinding and no mass lumping of the fluid mass are used (the "
                             "stabilization input parameter is ignored).  The "
                             "fluid-mass time derivative is close to linear, and is perfectly "
                             "linear if multiply_by_density=false.  These features mean the "
                             "results may differ slightly from the "
                             "Unsaturated Action case.  To run a simulation "
                             "you will also need to provide various other Materials for each mesh "
                             "block, depending on your simulation type, viz: permeability, "
                             "constant Biot modulus, constant thermal expansion coefficient, "
                             "porosity, elasticity tensor, strain calculator, stress calculator, "
                             "matrix internal energy, thermal conductivity, diffusivity");
  return params;
}

PorousFlowBasicTHM::PorousFlowBasicTHM(const InputParameters & params)
  : PorousFlowSinglePhaseBase(params), _multiply_by_density(getParam<bool>("multiply_by_density"))
{
  if (_num_mass_fraction_vars != 0)
    mooseError("PorousFlowBasicTHM can only be used for a single-component fluid, so that no "
               "mass-fraction variables should be provided");
}

void
PorousFlowBasicTHM::addMaterialDependencies()
{
  PorousFlowSinglePhaseBase::addMaterialDependencies();

  // Add necessary objects to list of PorousFlow objects added by this action
  _included_objects.push_back("PorousFlowFullySaturatedDarcyBase");

  if (_transient)
    _included_objects.push_back("PorousFlowFullySaturatedMassTimeDerivative");

  if (_thermal)
    _included_objects.push_back("PorousFlowFullySaturatedHeatAdvection");
}

void
PorousFlowBasicTHM::addKernels()
{
  PorousFlowSinglePhaseBase::addKernels();

  std::string kernel_name = "PorousFlowBasicTHM_DarcyFlow";
  std::string kernel_type = "PorousFlowFullySaturatedDarcyBase";
  InputParameters params = _factory.getValidParams(kernel_type);
  params.set<UserObjectName>("PorousFlowDictator") = _dictator_name;
  params.set<RealVectorValue>("gravity") = _gravity;
  params.set<bool>("multiply_by_density") = _multiply_by_density;
  params.set<NonlinearVariableName>("variable") = _pp_var;
  _problem->addKernel(kernel_type, kernel_name, params);

  if (_transient)
  {
    std::string kernel_name = "PorousFlowBasicTHM_MassTimeDerivative";
    std::string kernel_type = "PorousFlowFullySaturatedMassTimeDerivative";
    InputParameters params = _factory.getValidParams(kernel_type);
    params.set<UserObjectName>("PorousFlowDictator") = _dictator_name;
    params.set<NonlinearVariableName>("variable") = _pp_var;
    params.set<Real>("biot_coefficient") = _biot_coefficient;
    params.set<bool>("multiply_by_density") = _multiply_by_density;
    params.set<MooseEnum>("coupling_type") = parameters().get<MooseEnum>("coupling_type");
    if (_save_component_rate_in.size() != 0)
      params.set<std::vector<AuxVariableName>>("save_in") = _save_component_rate_in;
    params.set<NonlinearVariableName>("variable") = _pp_var;
    _problem->addKernel(kernel_type, kernel_name, params);
  }

  if (_thermal)
  {
    std::string kernel_name = "PorousFlowBasicTHM_HeatAdvection";
    std::string kernel_type = "PorousFlowFullySaturatedHeatAdvection";
    InputParameters params = _factory.getValidParams(kernel_type);
    params.set<NonlinearVariableName>("variable") = _temperature_var[0];
    params.set<UserObjectName>("PorousFlowDictator") = _dictator_name;
    params.set<RealVectorValue>("gravity") = _gravity;
    _problem->addKernel(kernel_type, kernel_name, params);
  }
}

void
PorousFlowBasicTHM::addMaterials()
{
  PorousFlowSinglePhaseBase::addMaterials();

  if (_deps.dependsOn(_included_objects, "pressure_saturation_qp"))
  {
    std::string material_type = "PorousFlow1PhaseFullySaturated";
    InputParameters params = _factory.getValidParams(material_type);
    std::string material_name = "PorousFlowBasicTHM_1PhaseP_qp";
    params.set<UserObjectName>("PorousFlowDictator") = _dictator_name;
    params.set<std::vector<VariableName>>("porepressure") = {_pp_var};
    params.set<bool>("at_nodes") = false;
    _problem->addMaterial(material_type, material_name, params);
  }

  if (_deps.dependsOn(_included_objects, "pressure_saturation_nodal"))
  {
    std::string material_type = "PorousFlow1PhaseFullySaturated";
    InputParameters params = _factory.getValidParams(material_type);
    std::string material_name = "PorousFlowBasicTHM_1PhaseP";
    params.set<UserObjectName>("PorousFlowDictator") = _dictator_name;
    params.set<std::vector<VariableName>>("porepressure") = {_pp_var};
    params.set<bool>("at_nodes") = true;
    _problem->addMaterial(material_type, material_name, params);
  }

  if ((_deps.dependsOn(_included_objects, "volumetric_strain_qp") ||
       _deps.dependsOn(_included_objects, "volumetric_strain_nodal")) &&
      _mechanical)
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
