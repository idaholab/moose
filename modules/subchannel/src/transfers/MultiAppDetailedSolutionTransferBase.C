#include "MultiAppDetailedSolutionTransferBase.h"
#include "MultiApp.h"
#include "FEProblemBase.h"
#include "DisplacedProblem.h"
#include "SubChannelMesh.h"

InputParameters
MultiAppDetailedSolutionTransferBase::validParams()
{
  InputParameters params = MultiAppTransfer::validParams();
  params.addRequiredParam<std::vector<AuxVariableName>>("variable",
                                                        "The auxiliary variables to transfer.");
  return params;
}

MultiAppDetailedSolutionTransferBase::MultiAppDetailedSolutionTransferBase(
    const InputParameters & parameters)
  : MultiAppTransfer(parameters), _var_names(getParam<std::vector<AuxVariableName>>("variable"))
{
  if (_directions.contains(Transfer::FROM_MULTIAPP))
    mooseError("This transfer works only into multi-app. Correct the 'direction' parameter.");
}

void
MultiAppDetailedSolutionTransferBase::execute()
{
  TIME_SECTION(
      "MultiAppDetailedSolutionBaseTransfer::execute()", 5, "Transferring subchannel solutions");
  _console << "********** Executing **********" << std::endl;
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
MultiAppDetailedSolutionTransferBase::transferToMultiApps()
{
  _console << "********** Transfer to MultiApps a **********" << std::endl;
  mooseAssert(_from_meshes.size() == 1, "Only one master mesh can be active in this transfer.");
  if (dynamic_cast<SubChannelMesh *>(_from_meshes[0]) == nullptr)
    mooseError("This transfer works only with SubChannelMesh classes.");

  for (unsigned int i = 0; i < getToMultiApp()->numGlobalApps(); i++)
    if (getToMultiApp()->hasLocalApp(i))
      transferVarsToApp(i);
}

void
MultiAppDetailedSolutionTransferBase::transferVarsToApp(unsigned int app_idx)
{
  transferNodalVars(app_idx);
}

void
MultiAppDetailedSolutionTransferBase::transferNodalVars(unsigned int app_idx)
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
