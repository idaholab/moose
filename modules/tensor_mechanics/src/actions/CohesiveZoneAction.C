//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CohesiveZoneAction.h"
#include "AddAuxVariableAction.h"
#include "Factory.h"
#include "FEProblem.h"
#include "Conversion.h"

registerMooseAction("TensorMechanicsApp", CohesiveZoneAction, "add_interface_kernel");
registerMooseAction("TensorMechanicsApp", CohesiveZoneAction, "add_material");
registerMooseAction("TensorMechanicsApp", CohesiveZoneAction, "add_master_action_material");
registerMooseAction("TensorMechanicsApp", CohesiveZoneAction, "add_aux_variable");
registerMooseAction("TensorMechanicsApp", CohesiveZoneAction, "add_aux_kernel");
registerMooseAction("TensorMechanicsApp", CohesiveZoneAction, "add_kernel");
registerMooseAction("TensorMechanicsApp", CohesiveZoneAction, "validate_coordinate_systems");

InputParameters
CohesiveZoneAction::validParams()
{
  InputParameters params = CohesiveZoneActionBase::validParams();
  params.addClassDescription("Action to create an instance of the cohesive zone model kernel for "
                             "each displacement component");
  params.addRequiredParam<std::vector<BoundaryName>>(
      "boundary", "The list of boundary IDs from the mesh where the cohesive zone will be applied");
  return params;
}

CohesiveZoneAction::CohesiveZoneAction(const InputParameters & params)
  : CohesiveZoneActionBase(params),
    _displacements(getParam<std::vector<VariableName>>("displacements")),
    _ndisp(_displacements.size()),
    _use_AD(getParam<bool>("use_automatic_differentiation")),
    _base_name(isParamValid("base_name") && !getParam<std::string>("base_name").empty()
                   ? getParam<std::string>("base_name")
                   : ""),
    _boundary(getParam<std::vector<BoundaryName>>("boundary")),
    _strain(getParam<MooseEnum>("strain").getEnum<Strain>()),
    _save_in_master(getParam<std::vector<AuxVariableName>>("save_in_master")),
    _diag_save_in_master(getParam<std::vector<AuxVariableName>>("diag_save_in_master")),
    _save_in_slave(getParam<std::vector<AuxVariableName>>("save_in_slave")),
    _diag_save_in_slave(getParam<std::vector<AuxVariableName>>("diag_save_in_slave")),
    _material_output_order(getParam<MultiMooseEnum>("material_output_order")),
    _material_output_family(getParam<MultiMooseEnum>("material_output_family")),
    _verbose(getParam<bool>("verbose"))
{
  // We can't enforce consistency between the number of displacement variables and the mesh
  // dimension. Hence we only check we have a reasonable number of displacement variables
  if (_ndisp > 3 || _ndisp < 1)
    mooseError("the CZM Action requires 1, 2 or 3 displacement variables.");

  switch (_strain)
  {
    case Strain::Small:
    {
      _czm_kernel_name =
          _use_AD ? "ADCZMInterfaceKernelSmallStrain" : "CZMInterfaceKernelSmallStrain";
      _disp_jump_provider_name = _use_AD ? "ADCZMComputeDisplacementJumpSmallStrain"
                                         : "CZMComputeDisplacementJumpSmallStrain";
      _equilibrium_traction_calculator_name =
          _use_AD ? "ADCZMComputeGlobalTractionSmallStrain" : "CZMComputeGlobalTractionSmallStrain";
      break;
    }
    case Strain::Finite:
    {
      _czm_kernel_name =
          _use_AD ? "ADCZMInterfaceKernelTotalLagrangian" : "CZMInterfaceKernelTotalLagrangian";
      _disp_jump_provider_name = _use_AD ? "ADCZMComputeDisplacementJumpTotalLagrangian"
                                         : "CZMComputeDisplacementJumpTotalLagrangian";
      _equilibrium_traction_calculator_name = _use_AD ? "ADCZMComputeGlobalTractionTotalLagrangian"
                                                      : "CZMComputeGlobalTractionTotalLagrangian";
      break;
    }
    default:
      mooseError("CohesiveZoneAction Error: Invalid kinematic parameter. Allowed values are: "
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

  // convert output variable names to lower case
  for (const auto & out : getParam<MultiMooseEnum>("generate_output"))
  {
    std::string lower(out);
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
    _generate_output.push_back(lower);
  }

  if (!_generate_output.empty())
    verifyOrderAndFamilyOutputs();
}

void
CohesiveZoneAction::addRequiredCZMInterfaceKernels()
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
    paramsk.set<std::vector<BoundaryName>>("boundary") = _boundary;
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

void
CohesiveZoneAction::addRequiredCZMInterfaceMaterials()
{
  // Create unique material name for the "CZMComputeDisplacementJump" object
  std::string unique_material_name = _disp_jump_provider_name + "_" + _name;
  InputParameters paramsm = _factory.getValidParams(_disp_jump_provider_name);
  paramsm.set<std::vector<BoundaryName>>("boundary") = _boundary;
  ;
  paramsm.set<std::vector<VariableName>>("displacements") = _displacements;
  paramsm.set<std::string>("base_name") = _base_name;
  _problem->addInterfaceMaterial(_disp_jump_provider_name, unique_material_name, paramsm);

  // Create unique material name for the "CZMComputeGlobalTraction" object
  unique_material_name = _equilibrium_traction_calculator_name + "_" + _name;
  paramsm = _factory.getValidParams(_equilibrium_traction_calculator_name);
  paramsm.set<std::vector<BoundaryName>>("boundary") = _boundary;
  ;
  paramsm.set<std::string>("base_name") = _base_name;
  _problem->addInterfaceMaterial(
      _equilibrium_traction_calculator_name, unique_material_name, paramsm);
}

void
CohesiveZoneAction::act()
{
  // Enforce consistency
  if (_ndisp != _mesh->dimension())
    paramError("displacements", "Number of displacements must match problem dimension.");

  chekMultipleActionParameters();

  if (_current_task == "add_interface_kernel")
    addRequiredCZMInterfaceKernels();
  else if (_current_task == "add_master_action_material")
    addRequiredCZMInterfaceMaterials();

  // optional, add required outputs
  actOutputGeneration();
}

void
CohesiveZoneAction::addRelationshipManagers(Moose::RelationshipManagerType input_rm_type)
{
  InputParameters ips = _factory.getValidParams(_czm_kernel_name);
  addRelationshipManagers(input_rm_type, ips);
}

void
CohesiveZoneAction::prepareSaveInInputs(std::vector<AuxVariableName> & save_in_names,
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

void
CohesiveZoneAction::verifyOrderAndFamilyOutputs()
{
  // Ensure material output order and family vectors are same size as generate output

  // check number of supplied orders and families
  if (_material_output_order.size() > 1 && _material_output_order.size() < _generate_output.size())
    paramError("material_output_order",
               "The number of orders assigned to material outputs must be: 0 to be assigned "
               "CONSTANT; 1 to assign all outputs the same value, or the same size as the number "
               "of generate outputs listed.");

  if (_material_output_family.size() > 1 &&
      _material_output_family.size() < _generate_output.size())
    paramError("material_output_family",
               "The number of families assigned to material outputs must be: 0 to be assigned "
               "MONOMIAL; 1 to assign all outputs the same value, or the same size as the number "
               "of generate outputs listed.");

  // if no value was provided, chose the default CONSTANT
  if (_material_output_order.size() == 0)
    _material_output_order.push_back("CONSTANT");

  // For only one order, make all orders the same magnitude
  if (_material_output_order.size() == 1)
    _material_output_order =
        std::vector<std::string>(_generate_output.size(), _material_output_order[0]);

  if (_verbose)
    Moose::out << COLOR_CYAN << "*** Automatic applied material output orders ***"
               << "\n"
               << _name << ": " << Moose::stringify(_material_output_order) << "\n"
               << COLOR_DEFAULT;

  // if no value was provided, chose the default MONOMIAL
  if (_material_output_family.size() == 0)
    _material_output_family.push_back("MONOMIAL");

  // For only one family, make all families that value
  if (_material_output_family.size() == 1)
    _material_output_family =
        std::vector<std::string>(_generate_output.size(), _material_output_family[0]);

  if (_verbose)
    Moose::out << COLOR_CYAN << "*** Automatic applied material output families ***"
               << "\n"
               << _name << ": " << Moose::stringify(_material_output_family) << "\n"
               << COLOR_DEFAULT;
}

void
CohesiveZoneAction::actOutputGeneration()
{
  if (_current_task == "add_material")
    actOutputMatProp();

  // Add variables (optional)
  if (_current_task == "add_aux_variable")
  {
    unsigned int index = 0;
    for (auto out : _generate_output)
    {
      const auto & order = _material_output_order[index];
      const auto & family = _material_output_family[index];

      std::string type = (order == "CONSTANT" && family == "MONOMIAL")
                             ? "MooseVariableConstMonomial"
                             : "MooseVariable";

      // Create output helper aux variables
      auto params = _factory.getValidParams(type);
      params.set<MooseEnum>("order") = order;
      params.set<MooseEnum>("family") = family;
      if (family == "MONOMIAL")
        _problem->addAuxVariable(type, addBaseName(out), params);
      else
        _problem->addVariable(type, addBaseName(out), params);

      index++;
    }
  }

  // Add output AuxKernels
  else if (_current_task == "add_aux_kernel")
  {
    const std::string material_output_aux_name = _use_AD ? "ADMaterialRealAux" : "MaterialRealAux";
    // Loop through output aux variables
    unsigned int index = 0;
    for (auto out : _generate_output)
    {
      if (_material_output_family[index] == "MONOMIAL")
      {
        InputParameters params = _factory.getValidParams(material_output_aux_name);
        params.set<MaterialPropertyName>("property") = addBaseName(out);
        params.set<AuxVariableName>("variable") = addBaseName(out);
        params.set<ExecFlagEnum>("execute_on") = EXEC_TIMESTEP_END;
        params.set<std::vector<BoundaryName>>("boundary") = _boundary;
        params.set<bool>("check_boundary_restricted") = false;
        _problem->addAuxKernel(material_output_aux_name, addBaseName(out) + '_' + name(), params);
      }
      index++;
    }
  }
}

void
CohesiveZoneAction::actOutputMatProp()
{
  if (_current_task == "add_material")
  {
    // Add output Materials
    for (auto out : _generate_output)
    {
      InputParameters params = emptyInputParameters();

      // RealVectorCartesianComponent
      if (
          [&]()
          {
            for (const auto & vq : _real_vector_cartesian_component_table)
              for (unsigned int a = 0; a < 3; ++a)
                if (vq.first + '_' + _component_table[a] == out)
                {
                  auto type = _use_AD ? "ADCZMRealVectorCartesianComponent"
                                      : "CZMRealVectorCartesianComponent";
                  params = _factory.getValidParams(type);
                  params.set<std::string>("real_vector_value") = vq.second;
                  params.set<unsigned int>("index") = a;
                  params.set<std::vector<BoundaryName>>("boundary") = _boundary;
                  params.set<MaterialPropertyName>("property_name") = addBaseName(out);
                  params.set<std::string>("base_name") = _base_name;
                  _problem->addInterfaceMaterial(type, addBaseName(out) + '_' + name(), params);
                  return true;
                }
            return false;
          }())
        continue;

      // CZMRealVectorScalar
      if (setupOutput(out,
                      _vector_direction_table,
                      [&](std::string prop_name, std::string direction)
                      {
                        auto type = _use_AD ? "ADCZMRealVectorScalar" : "CZMRealVectorScalar";
                        params = _factory.getValidParams(type);
                        params.set<std::string>("real_vector_value") = prop_name;
                        params.set<MooseEnum>("direction") = direction;
                        params.set<MaterialPropertyName>("property_name") = addBaseName(out);
                        params.set<std::vector<BoundaryName>>("boundary") = _boundary;
                        params.set<std::string>("base_name") = _base_name;
                        _problem->addInterfaceMaterial(
                            type, addBaseName(out) + '_' + name(), params);
                      }))
        continue;

      mooseError("CZM Master: unable to add output Material");
    }
  }
}

void
CohesiveZoneAction::chekMultipleActionParameters()
{

  // Gather info about all other master actions when we add variables
  if (_current_task == "validate_coordinate_systems")
  {
    auto actions = _awh.getActions<CohesiveZoneAction>();
    for (const auto & action : actions)
    {
      const auto size_before = _boundary_name_union.size();
      const auto added_size = action->_boundary.size();
      _boundary_name_union.insert(action->_boundary.begin(), action->_boundary.end());
      const auto size_after = _boundary_name_union.size();
      if (size_after != size_before + added_size)
        mooseError("The boundary restrictions in the CohesiveZoneAction actions must be "
                   "non-overlapping.");
    }

    for (const auto & action : actions)
    {
      // check for different strain definitions
      _strain_formulation_union.insert(action->_strain);
      const auto size_after = _strain_formulation_union.size();

      if (size_after != 1)
        mooseError("All blocks of the CohesiveZoneAction should have the same strain formulation");
    }
  }
}
