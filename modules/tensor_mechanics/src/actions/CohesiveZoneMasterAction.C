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
      "displacements", "The nonlinear displacement variables for the problem");
  MooseEnum strainType("SMALL FINITE", "SMALL");
  params.addParam<MooseEnum>("strain", strainType, "Strain formulation");

  // Advanced
  params.addParam<std::string>("base_name", "Material property base name");
  params.addParam<std::vector<AuxVariableName>>("save_in_master",
                                                "The displacement residuals on the  master side");
  params.addParam<std::vector<AuxVariableName>>(
      "diag_save_in_master", "The displacement diagonal preconditioner terms on the  master side");
  params.addParam<std::vector<AuxVariableName>>("save_in_slave",
                                                "The displacement residuals on the  slave side");
  params.addParam<std::vector<AuxVariableName>>(
      "diag_save_in_slave", "The displacement diagonal preconditioner terms on the  slave side");
  params.addParamNamesToGroup("save_in_master diag_save_in_master save_in_slave diag_save_in_slave",
                              "Advanced");

  return params;
}

CohesiveZoneMasterAction::CohesiveZoneMasterAction(const InputParameters & params)
  : Action(params),
    _displacements(getParam<std::vector<VariableName>>("displacements")),
    _ndisp(_displacements.size()),
    _base_name(isParamValid("base_name") && !getParam<std::string>("base_name").empty()
                   ? getParam<std::string>("base_name")
                   : ""),
    _strain(getParam<MooseEnum>("strain").getEnum<Strain>()),
    _save_in_master(getParam<std::vector<AuxVariableName>>("save_in_master")),
    _diag_save_in_master(getParam<std::vector<AuxVariableName>>("diag_save_in_master")),
    _save_in_slave(getParam<std::vector<AuxVariableName>>("save_in_slave")),
    _diag_save_in_slave(getParam<std::vector<AuxVariableName>>("diag_save_in_slave"))
{
  // We can't enforce consistency between the number of displacement variables and the mesh
  // dimension. Hence we only check we have a reasonable number of displacement variables
  if (_ndisp > 3 || _ndisp < 1)
    mooseError("the CZM Action requires 1, 2 or 3 displacement variables.");

  switch (_strain)
  {
    case Strain::Small:
    {
      _czm_kernel_name = "CZMInterfaceKernelSmallStrain";
      _disp_jump_provider_name = "CZMComputeDisplacementJumpSmallStrain";
      _equilibrium_traction_calculator_name = "CZMComputeGlobalTractionSmallStrain";
      break;
    }
    case Strain::Finite:
    {
      _czm_kernel_name = "CZMInterfaceKernelTotalLagrangian";
      _disp_jump_provider_name = "CZMComputeDisplacementJumpTotalLagrangian";
      _equilibrium_traction_calculator_name = "CZMComputeGlobalTractionTotalLagrangian";
      break;
    }
    default:
      mooseError("CohesiveZoneMasterAction Error: Invalid kinematic parameter. Allowed values are: "
                 "SmallStrain or TotalLagrangian");
  }

  if (_save_in_master.size() != 0 && _save_in_master.size() != _ndisp)
    mooseError(
        "Number of save_in_master variables should equal to the number of displacement variables ",
        _ndisp);
  if (_diag_save_in_master.size() != 0 && _diag_save_in_master.size() != _ndisp)
    mooseError("Number of diag_save_in_master variables should equal to the number of displacement "
               "variables ",
               _ndisp);
  if (_save_in_slave.size() != 0 && _save_in_slave.size() != _ndisp)
    mooseError(
        "Number of save_in_slave variables should equal to the number of displacement variables ",
        _ndisp);

  if (_diag_save_in_slave.size() != 0 && _diag_save_in_slave.size() != _ndisp)
    mooseError("Number of diag_save_in_slave variables should equal to the number of displacement "
               "variables ",
               _ndisp);
}

void
CohesiveZoneMasterAction::act()
{
  // Enforce consistency
  if (_ndisp != _mesh->dimension())
    paramError("displacements", "Number of displacements must match problem dimension.");

  if (_current_task == "add_interface_kernel")
  {
    for (unsigned int i = 0; i < _ndisp; ++i)
    {
      // Create unique kernel name for each displacement component
      std::string unique_kernel_name = _czm_kernel_name + "_" + _name + "_" + Moose::stringify(i);

      InputParameters paramsk = _factory.getValidParams(_czm_kernel_name);

      paramsk.set<unsigned int>("component") = i;
      paramsk.set<NonlinearVariableName>("variable") = _displacements[i];
      paramsk.set<std::vector<VariableName>>("neighbor_var") = {_displacements[i]};
      paramsk.set<std::vector<VariableName>>("displacements") = _displacements;
      paramsk.set<std::vector<BoundaryName>>("boundary") =
          getParam<std::vector<BoundaryName>>("boundary");
      paramsk.set<std::string>("base_name") = _base_name;

      std::string save_in_side;
      std::vector<AuxVariableName> save_in_var_names;
      if (_save_in_master.size() == _ndisp || _save_in_slave.size() == _ndisp)
      {
        prepareSaveInInputs(save_in_var_names, save_in_side, _save_in_master, _save_in_slave, i);
        paramsk.set<std::vector<AuxVariableName>>("save_in") = save_in_var_names;
        paramsk.set<MultiMooseEnum>("save_in_var_side") = save_in_side;
      }
      if (_diag_save_in_master.size() == _ndisp || _diag_save_in_slave.size() == _ndisp)
      {
        prepareSaveInInputs(
            save_in_var_names, save_in_side, _diag_save_in_master, _diag_save_in_slave, i);
        paramsk.set<std::vector<AuxVariableName>>("diag_save_in") = save_in_var_names;
        paramsk.set<MultiMooseEnum>("diag_save_in_var_side") = save_in_side;
      }
      _problem->addInterfaceKernel(_czm_kernel_name, unique_kernel_name, paramsk);
    }
  }
  else if (_current_task == "add_material")
  {
    // Create unique material name for the "CZMComputeDisplacementJump" object
    std::string unique_material_name = _disp_jump_provider_name + "_" + _name;
    InputParameters paramsm = _factory.getValidParams(_disp_jump_provider_name);
    paramsm.set<std::vector<BoundaryName>>("boundary") =
        getParam<std::vector<BoundaryName>>("boundary");
    paramsm.set<std::vector<VariableName>>("displacements") = _displacements;
    paramsm.set<std::string>("base_name") = _base_name;
    _problem->addInterfaceMaterial(_disp_jump_provider_name, unique_material_name, paramsm);

    // Create unique material name for the "CZMComputeGlobalTraction" object
    unique_material_name = _equilibrium_traction_calculator_name + "_" + _name;
    paramsm = _factory.getValidParams(_equilibrium_traction_calculator_name);
    paramsm.set<std::vector<BoundaryName>>("boundary") =
        getParam<std::vector<BoundaryName>>("boundary");
    paramsm.set<std::string>("base_name") = _base_name;
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

void
CohesiveZoneMasterAction::prepareSaveInInputs(std::vector<AuxVariableName> & save_in_names,
                                              std::string & save_in_side,
                                              const std::vector<AuxVariableName> & var_name_master,
                                              const std::vector<AuxVariableName> & var_name_slave,
                                              const int & i) const
{
  save_in_names.clear();
  save_in_side.clear();
  if (var_name_master.size() == _ndisp)
  {
    save_in_names.push_back(var_name_master[i]);
    save_in_side += "m";
    if (var_name_slave.size() == _ndisp)
      save_in_side += " ";
  }
  if (var_name_slave.size() == _ndisp)
  {
    save_in_names.push_back(var_name_slave[i]);
    save_in_side += "s";
  }
}
