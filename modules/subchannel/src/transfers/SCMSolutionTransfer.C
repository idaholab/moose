//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SCMSolutionTransfer.h"
#include "MultiApp.h"
#include "FEProblemBase.h"
#include "DisplacedProblem.h"
#include "SubChannelMesh.h"

registerMooseObject("SubChannelApp", SCMSolutionTransfer);
registerMooseObjectRenamed("SubChannelApp",
                           SolutionTransfer,
                           "06/30/2027 24:00",
                           SCMSolutionTransfer);
registerMooseObjectRenamed("SubChannelApp",
                           SCMPinSolutionTransfer,
                           "06/30/2027 24:00",
                           SCMSolutionTransfer);
registerMooseObjectRenamed("SubChannelApp",
                           PinSolutionTransfer,
                           "06/30/2027 24:00",
                           SCMSolutionTransfer);

InputParameters
SCMSolutionTransfer::validParams()
{
  InputParameters params = MultiAppTransfer::validParams();
  params.addRequiredParam<std::vector<AuxVariableName>>("variable",
                                                        "The auxiliary variables to transfer.");
  MooseEnum transfer_type("subchannel pin", "subchannel");
  params.addParam<MooseEnum>("transfer_type",
                             transfer_type,
                             "Whether to transfer subchannel-centered or pin-centered fields.");
  params.addClassDescription(
      "Transfers subchannel or pin solutions from a SubChannel mesh onto a visualization mesh.");
  return params;
}

SCMSolutionTransfer::SCMSolutionTransfer(const InputParameters & parameters)
  : MultiAppTransfer(parameters),
    _var_names(getParam<std::vector<AuxVariableName>>("variable")),
    _pin_transfer(getParam<MooseEnum>("transfer_type") == "pin")
{
  if (_directions.contains(Transfer::FROM_MULTIAPP))
    paramError("from_multiapp", "This transfer works only into multi-app.");
}

void
SCMSolutionTransfer::initialSetup()
{
  MultiAppTransfer::initialSetup();
  for (std::size_t var_index = 0; var_index < _var_names.size(); ++var_index)
  {
    if (_to_problems.empty())
      continue;

    MooseVariableFieldBase & from_var = _subproblem.getVariable(
        0, _var_names[var_index], Moose::VarKindType::VAR_ANY, Moose::VarFieldType::VAR_FIELD_ANY);
    System & from_sys = from_var.sys().system();
    const auto & fe_type = from_sys.variable_type(from_var.number());

    if (fe_type.family != LAGRANGE || fe_type.order != FIRST)
      paramError("variable",
                 "This transfer requires a first order Lagrange variable for the source variable");

    MooseVariableFieldBase & to_var = _to_problems[0]->getVariable(
        0, _var_names[var_index], Moose::VarKindType::VAR_ANY, Moose::VarFieldType::VAR_FIELD_ANY);

    System & to_sys = to_var.sys().system();
    const auto & fe_type_target = to_sys.variable_type(to_var.number());

    if (fe_type_target.family != LAGRANGE || fe_type_target.order != FIRST)
      paramError("variable",
                 "This transfer requires a first order Lagrange variable for the source variable");
  }
}

void
SCMSolutionTransfer::execute()
{
  TIME_SECTION(
      "MultiAppDetailedSolutionBaseTransfer::execute()", 5, "Transferring subchannel solutions");
  getAppInfo();

  switch (_current_direction)
  {
    case TO_MULTIAPP:
      transferToMultiApps();
      break;

    default:
      break;
  }
}

void
SCMSolutionTransfer::transferToMultiApps()
{
  mooseAssert(_from_meshes.size() == 1, "Only one source mesh can be active in this transfer.");
  auto * from_mesh = dynamic_cast<SubChannelMesh *>(_from_meshes[0]);
  if (from_mesh == nullptr)
    mooseError("This transfer works only with SubChannelMesh classes.");
  if (_pin_transfer && !from_mesh->pinMeshExist())
    mooseError(
        "This transfer was configured for pin variables, but the source mesh has no pin mesh.");

  for (unsigned int i = 0; i < getToMultiApp()->numGlobalApps(); i++)
    if (getToMultiApp()->hasLocalApp(i))
      transferVarsToApp(i);
}

void
SCMSolutionTransfer::transferVarsToApp(unsigned int app_idx)
{
  transferNodalVars(app_idx);
}

void
SCMSolutionTransfer::transferNodalVars(unsigned int app_idx)
{
  Moose::ScopedCommSwapper swapper(getToMultiApp()->comm());

  FEProblemBase & to_problem = getToMultiApp()->appProblemBase(app_idx);
  MooseMesh * mesh = NULL;
  if (_displaced_target_mesh && to_problem.getDisplacedProblem())
    mesh = &to_problem.getDisplacedProblem()->mesh();
  else
    mesh = &to_problem.mesh();

  const SubChannelMesh & from_mesh = dynamic_cast<SubChannelMesh &>(*_from_meshes[0]);
  FEProblemBase & from_problem = *_from_problems[0];

  for (auto & node : mesh->getMesh().local_node_ptr_range())
  {
    if (processor_id() != 0)
      continue;
    Node * from_node = getFromNode(from_mesh, *node);

    for (auto & var_name : _var_names)
    {
      System * to_sys = find_sys(to_problem.es(), var_name);
      unsigned int to_sys_num = to_sys->number();
      unsigned int to_var_num = to_sys->variable_number(var_name);

      if (node->n_dofs(to_sys_num, to_var_num) > 0)
      {
        System * from_sys = find_sys(from_problem.es(), var_name);
        unsigned int from_sys_num = from_sys->number();
        unsigned int from_var_num = from_sys->variable_number(var_name);

        swapper.forceSwap();
        NumericVector<Real> * from_solution = from_sys->solution.get();
        dof_id_type from_dof = from_node->dof_number(from_sys_num, from_var_num, 0);
        Real from_value = (*from_solution)(from_dof);
        swapper.forceSwap();

        NumericVector<Real> & to_solution = getToMultiApp()->appTransferVector(app_idx, var_name);
        dof_id_type to_dof = node->dof_number(to_sys_num, to_var_num, 0);
        to_solution.set(to_dof, from_value);
      }
    }
  }

  for (auto & var_name : _var_names)
  {
    getToMultiApp()->appTransferVector(app_idx, var_name).close();
    find_sys(to_problem.es(), var_name)->update();
  }
}

Node *
SCMSolutionTransfer::getFromNode(const SubChannelMesh & from_mesh, const Point & src_node)
{
  unsigned int sch_idx =
      _pin_transfer ? from_mesh.pinIndex(src_node) : from_mesh.channelIndex(src_node);
  unsigned iz = from_mesh.getZIndex(src_node);
  return _pin_transfer ? from_mesh.getPinNode(sch_idx, iz) : from_mesh.getChannelNode(sch_idx, iz);
}
