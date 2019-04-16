//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "MultiAppFieldTransferInterface.h"
#include "MooseTypes.h"
#include "FEProblem.h"
#include "MultiApp.h"
#include "MooseMesh.h"
#include "UserObject.h"

template <>
InputParameters
validParams<MultiAppFieldTransferInterface>()
{
  InputParameters params = validParams<MultiAppTransfer>();
  params.addRequiredParam<std::vector<AuxVariableName>>(
      "variable", "The auxiliary variable to store the transferred values in.");
  params.addRequiredParam<std::vector<VariableName>>("source_variable",
                                                     "The variable to transfer from.");
  params.addParam<bool>("preserve_transfer",
                        false,
                        "Whether or not to conserve the transfered field, "
                        " if true, the transfered variables will be adjusted "
                        "according to the pps value");

  std::vector<PostprocessorName> from_postprocessor = {"from_postprocessor"};
  params.addParam<std::vector<PostprocessorName>>(
      "from_postprocessor_to_be_preserved",
      from_postprocessor,
      "The name of the Postprocessor in the from-app to evaluate an adjusting factor.");

  std::vector<PostprocessorName> to_postprocessor = {"to_postprocessor"};
  params.addParam<std::vector<PostprocessorName>>(
      "to_postprocessor_to_be_preserved",
      to_postprocessor,
      "The name of the Postprocessor in the to-app to evaluate an adjusting factor.");
  return params;
}

MultiAppFieldTransferInterface::MultiAppFieldTransferInterface(const InputParameters & parameters)
  : MultiAppTransfer(parameters),
    _from_var_name(getParam<std::vector<VariableName>>("source_variable")),
    _to_var_name(getParam<std::vector<AuxVariableName>>("variable")),
    _preserve_transfer(parameters.get<bool>("preserve_transfer")),
    _from_postprocessor_to_be_preserved(
        parameters.get<std::vector<PostprocessorName>>("from_postprocessor_to_be_preserved")),
    _to_postprocessor_to_be_preserved(
        parameters.get<std::vector<PostprocessorName>>("to_postprocessor_to_be_preserved"))
{
  if (_preserve_transfer)
  {
    if (_direction == TO_MULTIAPP)
    {
      mooseAssert(_from_postprocessor_to_be_preserved.size() == _multi_app->numGlobalApps(),
                  "Number of from Postprocessors should equal to the number of subapps");
      mooseAssert(_to_postprocessor_to_be_preserved.size() == 1,
                  "Number of to Postprocessors should equal to 1");
    }
    else if (_direction == FROM_MULTIAPP)
    {
      mooseAssert(_from_postprocessor_to_be_preserved.size() == 1,
                  "Number of from Postprocessors should equal to 1");
      mooseAssert(_to_postprocessor_to_be_preserved.size() == _multi_app->numGlobalApps(),
                  "Number of to Postprocessors should equal to the number of subapps ");
    }
  }
}

void
MultiAppFieldTransferInterface::postExecute()
{
  if (_preserve_transfer)
  {
    _console << "Beginning Conservative transfers " << name() << std::endl;

    if (_direction == TO_MULTIAPP)
    {
      FEProblemBase & from_problem = _multi_app->problemBase();
      for (unsigned int i = 0; i < _multi_app->numGlobalApps(); i++)
        if (_multi_app->hasLocalApp(i))
          adjustTransferedSolution(&from_problem,
                                   _from_postprocessor_to_be_preserved[i],
                                   _multi_app->appProblemBase(i),
                                   _to_postprocessor_to_be_preserved[0]);
    }

    else if (_direction == FROM_MULTIAPP)
    {
      FEProblemBase & to_problem = _multi_app->problemBase();
      for (unsigned int i = 0; i < _multi_app->numGlobalApps(); i++)
      {
        adjustTransferedSolution(_multi_app->hasLocalApp(i) ? &_multi_app->appProblemBase(i)
                                                            : nullptr,
                                 _from_postprocessor_to_be_preserved[0],
                                 to_problem,
                                 _to_postprocessor_to_be_preserved[i]);
      }
    }

    _console << "Finished Conservative transfers " << name() << std::endl;
  }
}

void
MultiAppFieldTransferInterface::adjustTransferedSolution(FEProblemBase * from_problem,
                                                         PostprocessorName & from_postprocessor,
                                                         FEProblemBase & to_problem,
                                                         PostprocessorName & to_postprocessor)
{
  PostprocessorValue from_adjuster = 0;
  if (from_problem)
  {
    from_adjuster = from_problem->getPostprocessorValue(from_postprocessor);
  }
  else
  {
    from_adjuster = 0;
  }
  /* Everyone on master side should know this value, and use it to scale the solution */
  if (_direction == FROM_MULTIAPP)
  {
    comm().max(from_adjuster);
  }

  // Compute to-postproessor to have the adjuster
  to_problem.computeUserObjectByName(EXEC_TRANSFER, to_postprocessor);

  // Now we should have the right adjuster based on the transfered solution
  PostprocessorValue & to_adjuster = to_problem.getPostprocessorValue(to_postprocessor);

  auto & to_var = to_problem.getVariable(
      0, _to_var_name[0], Moose::VarKindType::VAR_ANY, Moose::VarFieldType::VAR_FIELD_STANDARD);
  auto & to_sys = to_var.sys().system();
  auto var_num = to_sys.variable_number(_to_var_name[0]);
  auto sys_num = to_sys.number();
  auto * pps =
      dynamic_cast<const BlockRestrictable *>(&(to_problem.getUserObjectBase(to_postprocessor)));
  auto & to_solution = to_var.sys().solution();
  auto & to_mesh = to_problem.mesh().getMesh();
  auto & moose_mesh = to_problem.mesh();
  bool is_nodal = to_sys.variable_type(var_num).family == LAGRANGE;
  if (is_nodal)
  {
    for (const auto & node : to_mesh.local_node_ptr_range())
    {
      // Skip this node if the variable has no dofs at it.
      if (node->n_dofs(sys_num, var_num) < 1)
        continue;

      bool scale_current_node = false;
      /* If we care about block IDs */
      if (pps)
      {
        auto & blockids = pps->blockIDs();
        auto & node_to_elem_map = moose_mesh.nodeToElemMap();
        auto neighbor_elements = node_to_elem_map.find(node->id());
        for (auto element : neighbor_elements->second)
        {
          auto & elem = to_mesh.elem_ref(element);
          if (blockids.find(elem.subdomain_id()) != blockids.end() ||
              blockids.find(Moose::ANY_BLOCK_ID) != blockids.end())
          {
            scale_current_node = true;
            break;
          }
        }
      }
      else
      {
        scale_current_node = true;
      }
      /* Need to scale this node */
      if (scale_current_node)
      {
        dof_id_type dof = node->dof_number(sys_num, var_num, 0);
        to_solution.set(dof, (from_adjuster / to_adjuster) * to_solution(dof));
      }
    }
  }
  else
  {
    for (auto & elem : as_range(to_mesh.local_elements_begin(), to_mesh.local_elements_end()))
    {
      // Skip this element if the variable has no dofs at it.
      if (elem->n_dofs(sys_num, var_num) < 1)
        continue;

      bool scale_current_element = false;
      if (pps)
      {
        auto & blockids = pps->blockIDs();
        if (blockids.find(elem->subdomain_id()) != blockids.end() ||
            blockids.find(Moose::ANY_BLOCK_ID) != blockids.end())
        {
          scale_current_element = true;
        }
      }
      else
      {
        scale_current_element = true;
      }
      if (scale_current_element)
      {
        dof_id_type dof = elem->dof_number(sys_num, var_num, 0);
        to_solution.set(dof, (from_adjuster / to_adjuster) * to_solution(dof));
      }
    }
  }

  to_solution.close();
  to_sys.update();
}
