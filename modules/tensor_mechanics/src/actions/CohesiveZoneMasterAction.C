//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CohesiveZoneMasterAction.h"
#include "Factory.h"
#include "FEProblem.h"
#include "Conversion.h"

registerMooseAction("TensorMechanicsApp", CohesiveZoneMasterAction, "add_interface_kernel");

registerMooseAction("TensorMechanicsApp", CohesiveZoneMasterAction, "add_material");

InputParameters
CohesiveZoneMasterAction::validParams()
{
  InputParameters params = Action::validParams();
  params.addClassDescription("Action to create an instance of the cohesive zone model kernel for "
                             "each displacement component");
  params.addRequiredParam<std::vector<BoundaryName>>(
      "boundary", "The list of boundary IDs from the mesh where the cohesive zone will be applied");
  params.addRequiredParam<std::vector<VariableName>>(
      "displacements",
      "The displacements appropriate for the simulation geometry and coordinate system");
  MooseEnum kinematicType("SmallStrain TotalLagrangian", "SmallStrain");
  params.addParam<MooseEnum>("kinematic", kinematicType, "Kinematic formulation");

  return params;
}

CohesiveZoneMasterAction::CohesiveZoneMasterAction(const InputParameters & params)
  : Action(params), _kinematic(getParam<MooseEnum>("kinematic").getEnum<Kinematic>())
{
  if (_kinematic == Kinematic::SmallStrain)
  {
    _czm_kernel_name = "CZMInterfaceKernelSmallStrain";
    _disp_jump_provider_name = "CZMDisplacementJumpProviderSmallStrain";
    _equilibrium_traction_calculator_name = "CZMEquilibriumTractionCalculatorSmallStrain";
  }
  else if (_kinematic == Kinematic::TotalLagrangian)
  {
    _czm_kernel_name = "CZMInterfaceKernelTotalLagrangian";
    _disp_jump_provider_name = "CZMDisplacementJumpProviderIncrementalTotalLagrangian";
    _equilibrium_traction_calculator_name = "CZMEquilibriumTractionCalculatorTotalLagrangian";
  }
  else
    mooseError("Internal error");
}

void
CohesiveZoneMasterAction::act()
{
  std::vector<VariableName> displacements;
  if (isParamValid("displacements"))
    displacements = getParam<std::vector<VariableName>>("displacements");

  if (_current_task == "add_interface_kernel")
  {
    for (unsigned int i = 0; i < displacements.size(); ++i)
    {
      // Create unique kernel name for each of the components
      std::string unique_kernel_name = _czm_kernel_name + "_" + _name + "_" + Moose::stringify(i);

      InputParameters paramsk = _factory.getValidParams(_czm_kernel_name);

      paramsk.set<unsigned int>("component") = i;
      paramsk.set<NonlinearVariableName>("variable") = displacements[i];
      paramsk.set<std::vector<VariableName>>("neighbor_var") = {displacements[i]};
      paramsk.set<std::vector<VariableName>>("displacements") = displacements;
      paramsk.set<std::vector<BoundaryName>>("boundary") =
          getParam<std::vector<BoundaryName>>("boundary");

      _problem->addInterfaceKernel(_czm_kernel_name, unique_kernel_name, paramsk);
    }
  }
  else if (_current_task == "add_material")
  {
    // Create unique material name for the displacement jump provider
    std::string unique_material_name = _disp_jump_provider_name + "_" + _name;
    InputParameters paramsm = _factory.getValidParams(_disp_jump_provider_name);
    paramsm.set<std::vector<BoundaryName>>("boundary") =
        getParam<std::vector<BoundaryName>>("boundary");
    paramsm.set<std::vector<VariableName>>("displacements") = displacements;
    _problem->addInterfaceMaterial(_disp_jump_provider_name, unique_material_name, paramsm);

    // Create unique material name for the equilibrium traction calculator
    unique_material_name = _equilibrium_traction_calculator_name + "_" + _name;
    paramsm = _factory.getValidParams(_equilibrium_traction_calculator_name);
    paramsm.set<std::vector<BoundaryName>>("boundary") =
        getParam<std::vector<BoundaryName>>("boundary");
    _problem->addInterfaceMaterial(
        _equilibrium_traction_calculator_name, unique_material_name, paramsm);
  }
}

void
CohesiveZoneMasterAction::addRelationshipManagers(Moose::RelationshipManagerType input_rm_type)
{
  InputParameters ips = _factory.getValidParams(_czm_kernel_name);
  addRelationshipManagers(input_rm_type, ips);
}
