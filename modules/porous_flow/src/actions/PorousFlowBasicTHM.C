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

template <>
InputParameters
validParams<PorousFlowBasicTHM>()
{
  InputParameters params = validParams<PorousFlowSinglePhaseBase>();
  params.addParam<bool>("multiply_by_density",
                        false,
                        "If true, then the Kernels for fluid flow are multiplied by "
                        "the fluid density.  If false, this multiplication is not "
                        "performed, which means the problem linearises, but that care "
                        "must be taken when using other PorousFlow objects.");
  params.addClassDescription("Adds Kernels and fluid-property Materials necessary to simulate a "
                             "single-phase, single-component fully-saturated flow problem.  No "
                             "upwinding and no mass lumping of the fluid mass are used.  The "
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
  _objects_to_add.push_back("PorousFlowFullySaturatedDarcyBase");
  if (_simulation_type == SimulationTypeChoiceEnum::TRANSIENT)
    _objects_to_add.push_back("PorousFlowFullySaturatedMassTimeDerivative");
  if (_coupling_type == CouplingTypeEnum::ThermoHydro ||
      _coupling_type == CouplingTypeEnum::ThermoHydroMechanical)
    _objects_to_add.push_back("PorousFlowFullySaturatedHeatAdvection");
}

void
PorousFlowBasicTHM::act()
{
  PorousFlowSinglePhaseBase::act();

  // add the kernels
  if (_current_task == "add_kernel")
  {
    std::string kernel_name = "PorousFlowBasicTHM_DarcyFlow";
    std::string kernel_type = "PorousFlowFullySaturatedDarcyBase";
    InputParameters params = _factory.getValidParams(kernel_type);
    params.set<UserObjectName>("PorousFlowDictator") = _dictator_name;
    params.set<RealVectorValue>("gravity") = _gravity;
    params.set<bool>("multiply_by_density") = _multiply_by_density;
    params.set<NonlinearVariableName>("variable") = _pp_var;
    _problem->addKernel(kernel_type, kernel_name, params);
  }
  if (_current_task == "add_kernel" && _simulation_type == SimulationTypeChoiceEnum::TRANSIENT)
  {
    std::string kernel_name = "PorousFlowBasicTHM_MassTimeDerivative";
    std::string kernel_type = "PorousFlowFullySaturatedMassTimeDerivative";
    InputParameters params = _factory.getValidParams(kernel_type);
    params.set<UserObjectName>("PorousFlowDictator") = _dictator_name;
    params.set<NonlinearVariableName>("variable") = _pp_var;
    params.set<Real>("biot_coefficient") = _biot_coefficient;
    params.set<bool>("multiply_by_density") = _multiply_by_density;
    params.set<MooseEnum>("coupling_type") = parameters().get<MooseEnum>("coupling_type");
    _problem->addKernel(kernel_type, kernel_name, params);
  }

  if ((_coupling_type == CouplingTypeEnum::ThermoHydro ||
       _coupling_type == CouplingTypeEnum::ThermoHydroMechanical) &&
      _current_task == "add_kernel")
  {
    std::string kernel_name = "PorousFlowBasicTHM_HeatAdvection";
    std::string kernel_type = "PorousFlowFullySaturatedHeatAdvection";
    InputParameters params = _factory.getValidParams(kernel_type);
    params.set<NonlinearVariableName>("variable") = _temperature_var[0];
    params.set<UserObjectName>("PorousFlowDictator") = _dictator_name;
    params.set<RealVectorValue>("gravity") = _gravity;
    _problem->addKernel(kernel_type, kernel_name, params);
  }

  // add Materials
  if (_deps.dependsOn(_objects_to_add, "PorousFlowPS_qp") && _current_task == "add_material")
  {
    std::string material_type = "PorousFlow1PhaseFullySaturated";
    InputParameters params = _factory.getValidParams(material_type);
    std::string material_name = "PorousFlowBasicTHM_1PhaseP_qp";
    params.set<UserObjectName>("PorousFlowDictator") = _dictator_name;
    params.set<std::vector<VariableName>>("porepressure") = {_pp_var};
    _problem->addMaterial(material_type, material_name, params);
  }
  if (_deps.dependsOn(_objects_to_add, "PorousFlowPS_nodal") && _current_task == "add_material")
  {
    std::string material_type = "PorousFlow1PhaseFullySaturated";
    InputParameters params = _factory.getValidParams(material_type);
    std::string material_name = "PorousFlowBasicTHM_1PhaseP";
    params.set<UserObjectName>("PorousFlowDictator") = _dictator_name;
    params.set<std::vector<VariableName>>("porepressure") = {_pp_var};
    params.set<bool>("at_nodes") = true;
    _problem->addMaterial(material_type, material_name, params);
  }

  if ((_deps.dependsOn(_objects_to_add, "PorousFlowVolumetricStrain_qp") ||
       _deps.dependsOn(_objects_to_add, "PorousFlowVolumetricStrain_nodal")) &&
      (_coupling_type == CouplingTypeEnum::HydroMechanical ||
       _coupling_type == CouplingTypeEnum::ThermoHydroMechanical))
    addVolumetricStrainMaterial(_coupled_displacements, false);

  if (_deps.dependsOn(_objects_to_add, "PorousFlowRelativePermeability_qp"))
    addRelativePermeabilityCorey(false, 0, 0.0, 0.0, 0.0);
  if (_deps.dependsOn(_objects_to_add, "PorousFlowRelativePermeability_nodal"))
    addRelativePermeabilityCorey(true, 0, 0.0, 0.0, 0.0);
}
