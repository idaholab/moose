//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "MultiAppConservativeTransfer.h"
#include "MooseTypes.h"
#include "FEProblem.h"
#include "MultiApp.h"
#include "MooseMesh.h"
#include "UserObject.h"
#include "NearestPointIntegralVariablePostprocessor.h"
#include "SystemBase.h"

defineLegacyParams(MultiAppConservativeTransfer);

InputParameters
MultiAppConservativeTransfer::validParams()
{
  InputParameters params = MultiAppFieldTransfer::validParams();
  params.addRequiredParam<std::vector<AuxVariableName>>(
      "variable", "The auxiliary variable to store the transferred values in.");
  params.addRequiredParam<std::vector<VariableName>>("source_variable",
                                                     "The variable to transfer from.");

  params.addParam<std::vector<PostprocessorName>>(
      "from_postprocessors_to_be_preserved",
      "The name of the Postprocessor in the from-app to evaluate an adjusting factor.");

  params.addParam<std::vector<PostprocessorName>>(
      "to_postprocessors_to_be_preserved",
      "The name of the Postprocessor in the to-app to evaluate an adjusting factor.");
  return params;
}

MultiAppConservativeTransfer::MultiAppConservativeTransfer(const InputParameters & parameters)
  : MultiAppFieldTransfer(parameters),
    _from_var_names(getParam<std::vector<VariableName>>("source_variable")),
    _to_var_names(getParam<std::vector<AuxVariableName>>("variable")),
    _preserve_transfer(isParamValid("from_postprocessors_to_be_preserved")),
    _from_postprocessors_to_be_preserved(
        getParam<std::vector<PostprocessorName>>("from_postprocessors_to_be_preserved")),
    _to_postprocessors_to_be_preserved(
        getParam<std::vector<PostprocessorName>>("to_postprocessors_to_be_preserved")),
    _use_nearestpoint_pps(false)
{
  if (_directions.size() != 1)
    paramError("direction", "This transfer is only unidirectional");

  if (_preserve_transfer)
  {
    /*
     * Not sure how important to support multi variables
     * Let us handle the single variable case only right now if the conservative capability is on
     */
    if (_to_var_names.size() != 1)
      paramError("variable",
                 " Support single variable only when the conservative capability is on ");

    if (_current_direction == TO_MULTIAPP)
    {
      if (_from_postprocessors_to_be_preserved.size() != _multi_app->numGlobalApps() &&
          _from_postprocessors_to_be_preserved.size() != 1)
        paramError("from_postprocessors_to_be_preserved",
                   "Number of from-postprocessors should equal to the number of subapps, or use "
                   "NearestPointIntegralVariablePostprocessor");
      if (_to_postprocessors_to_be_preserved.size() != 1)
        paramError("to_postprocessors_to_be_preserved",
                   "Number of to-postprocessors should equal to 1");
    }
    else if (_current_direction == FROM_MULTIAPP)
    {
      if (_from_postprocessors_to_be_preserved.size() != 1)
        paramError("from_postprocessors_to_be_preserved",
                   "Number of from Postprocessors should equal to 1");

      if (_to_postprocessors_to_be_preserved.size() != _multi_app->numGlobalApps() &&
          _to_postprocessors_to_be_preserved.size() != 1)
        paramError("to_postprocessors_to_be_preserved",
                   "_to_postprocessors_to_be_preserved",
                   "Number of to Postprocessors should equal to the number of subapps, or use "
                   "NearestPointIntegralVariablePostprocessor ");
    }
  }

  /* Have to specify at least one to-variable */
  if (_to_var_names.size() == 0)
    paramError("variable", "You need to specify at least one variable");

  /* Right now, most of transfers support one variable only */
  if (_to_var_names.size() == 1)
    _to_var_name = _to_var_names[0];

  if (_from_var_names.size() == 1)
    _from_var_name = _from_var_names[0];
}

void
MultiAppConservativeTransfer::initialSetup()
{
  MultiAppFieldTransfer::initialSetup();
  if (_preserve_transfer)
  {
    if (_from_postprocessors_to_be_preserved.size() == 1 && _current_direction == TO_MULTIAPP)
    {
      FEProblemBase & from_problem = _multi_app->problemBase();
      auto * pps = dynamic_cast<const NearestPointIntegralVariablePostprocessor *>(
          &(from_problem.getUserObjectBase(_from_postprocessors_to_be_preserved[0])));
      if (pps)
        _use_nearestpoint_pps = true;
      else
      {
        _use_nearestpoint_pps = false;
        if (_multi_app->numGlobalApps() > 1)
          mooseError(
              " You have to specify ",
              _multi_app->numGlobalApps(),
              " regular from-postprocessors, or use NearestPointIntegralVariablePostprocessor ");
      }
    }

    if (_to_postprocessors_to_be_preserved.size() == 1 && _current_direction == FROM_MULTIAPP)
    {
      FEProblemBase & to_problem = _multi_app->problemBase();
      auto * pps = dynamic_cast<const NearestPointIntegralVariablePostprocessor *>(
          &(to_problem.getUserObjectBase(_to_postprocessors_to_be_preserved[0])));
      if (pps)
        _use_nearestpoint_pps = true;
      else
      {
        _use_nearestpoint_pps = false;
        if (_multi_app->numGlobalApps() > 1)
          mooseError(
              " You have to specify ",
              _multi_app->numGlobalApps(),
              " regular to-postprocessors, or use NearestPointIntegralVariablePostprocessor ");
      }
    }
  }
}

void
MultiAppConservativeTransfer::postExecute()
{
  if (_preserve_transfer)
  {
    _console << "Beginning Conservative transfers " << name() << std::endl;

    if (_current_direction == TO_MULTIAPP)
    {
      FEProblemBase & from_problem = _multi_app->problemBase();
      if (_use_nearestpoint_pps)
        from_problem.computeUserObjectByName(EXEC_TRANSFER,
                                             _from_postprocessors_to_be_preserved[0]);

      for (unsigned int i = 0; i < _multi_app->numGlobalApps(); i++)
        if (_multi_app->hasLocalApp(i))
        {
          if (_use_nearestpoint_pps)
            adjustTransferedSolutionNearestPoint(i,
                                                 &from_problem,
                                                 _from_postprocessors_to_be_preserved[0],
                                                 _multi_app->appProblemBase(i),
                                                 _to_postprocessors_to_be_preserved[0]);
          else
            adjustTransferedSolution(&from_problem,
                                     _from_postprocessors_to_be_preserved[i],
                                     _multi_app->appProblemBase(i),
                                     _to_postprocessors_to_be_preserved[0]);
        }
    }

    else if (_current_direction == FROM_MULTIAPP)
    {
      FEProblemBase & to_problem = _multi_app->problemBase();
      if (_use_nearestpoint_pps)
        to_problem.computeUserObjectByName(EXEC_TRANSFER, _to_postprocessors_to_be_preserved[0]);

      for (unsigned int i = 0; i < _multi_app->numGlobalApps(); i++)
      {
        if (_use_nearestpoint_pps)
          adjustTransferedSolutionNearestPoint(
              i,
              _multi_app->hasLocalApp(i) ? &_multi_app->appProblemBase(i) : nullptr,
              _from_postprocessors_to_be_preserved[0],
              to_problem,
              _to_postprocessors_to_be_preserved[0]);
        else
          adjustTransferedSolution(_multi_app->hasLocalApp(i) ? &_multi_app->appProblemBase(i)
                                                              : nullptr,
                                   _from_postprocessors_to_be_preserved[0],
                                   to_problem,
                                   _to_postprocessors_to_be_preserved[i]);
      }

      // Compute the to-postproessor again so that it has the right value with the updated solution
      if (_use_nearestpoint_pps)
        to_problem.computeUserObjectByName(EXEC_TRANSFER, _to_postprocessors_to_be_preserved[0]);
    }

    _console << "Finished Conservative transfers " << name() << std::endl;
  }
}

void
MultiAppConservativeTransfer::adjustTransferedSolutionNearestPoint(
    unsigned int i,
    FEProblemBase * from_problem,
    PostprocessorName & from_postprocessor,
    FEProblemBase & to_problem,
    PostprocessorName & to_postprocessor)
{
  PostprocessorValue from_adjuster = 0;
  if (from_problem && _current_direction == FROM_MULTIAPP)
    from_adjuster = from_problem->getPostprocessorValue(from_postprocessor);
  else
    from_adjuster = 0;

  /* Everyone on master side should know this value, and use it to scale the solution */
  if (_current_direction == FROM_MULTIAPP)
  {
    /* In this case, only one subapp has value, and other subapps' must be zero.
     *  We should see the maximum value.
     */
    PostprocessorValue from_adjuster_tmp = from_adjuster;
    comm().max(from_adjuster);

    /* We may have a negative value */
    if (MooseUtils::absoluteFuzzyLessEqual(from_adjuster, 0.))
    {
      comm().min(from_adjuster_tmp);
      from_adjuster = from_adjuster_tmp;
    }

    /* We should not have a zero value */
    if (MooseUtils::absoluteFuzzyLessEqual(from_adjuster, 0.))
      mooseError("from_adjuster must be nonzero");
  }

  PostprocessorValue to_adjuster = 0;
  // Compute to-postproessor to have the adjuster
  if (_current_direction == TO_MULTIAPP)
  {
    to_problem.computeUserObjectByName(EXEC_TRANSFER, to_postprocessor);
    to_adjuster = to_problem.getPostprocessorValue(to_postprocessor);
  }

  auto & to_var = to_problem.getVariable(
      0, _to_var_name, Moose::VarKindType::VAR_ANY, Moose::VarFieldType::VAR_FIELD_STANDARD);
  auto & to_sys = to_var.sys().system();
  auto var_num = to_sys.variable_number(_to_var_name);
  auto sys_num = to_sys.number();
  auto & pps = static_cast<const NearestPointIntegralVariablePostprocessor &>(
      _current_direction == FROM_MULTIAPP ? (to_problem.getUserObjectBase(to_postprocessor))
                                          : (from_problem->getUserObjectBase(from_postprocessor)));
  auto & to_solution = to_var.sys().solution();
  auto & to_mesh = to_problem.mesh().getMesh();
  bool is_nodal = to_sys.variable_type(var_num).family == LAGRANGE;
  if (is_nodal)
  {
    for (const auto & node : to_mesh.local_node_ptr_range())
    {
      // Skip this node if the variable has no dofs at it.
      if (node->n_dofs(sys_num, var_num) < 1)
        continue;

      Real scale = 1;
      if (_current_direction == FROM_MULTIAPP)
      {
        auto ii = pps.nearestPointIndex(*node);
        if (ii != i)
          continue;
        if (MooseUtils::absoluteFuzzyLessEqual(pps.userObjectValue(i), 0.))
          mooseError("to_adjuster must be nonzero");

        scale = from_adjuster / pps.userObjectValue(i);
      }
      else
      {
        if (MooseUtils::absoluteFuzzyLessEqual(to_adjuster, 0.))
          mooseError("to_adjuster must be nonzero");

        scale = pps.userObjectValue(i) / to_adjuster;
      }

      /* Need to scale this node */
      dof_id_type dof = node->dof_number(sys_num, var_num, 0);
      to_solution.set(dof, scale * to_solution(dof));
    }
  }
  else
  {
    for (auto & elem : as_range(to_mesh.local_elements_begin(), to_mesh.local_elements_end()))
    {
      // Skip this element if the variable has no dofs at it.
      if (elem->n_dofs(sys_num, var_num) < 1)
        continue;

      Real scale = 1;
      if (_current_direction == FROM_MULTIAPP)
      {
        unsigned int ii = pps.nearestPointIndex(elem->centroid());
        if (ii != i)
          continue;

        if (MooseUtils::absoluteFuzzyLessEqual(pps.userObjectValue(i), 0.))
          mooseError("to_adjuster must be nonzero");

        scale = from_adjuster / pps.userObjectValue(i);
      }
      else
      {
        if (MooseUtils::absoluteFuzzyLessEqual(to_adjuster, 0.))
          mooseError("to_adjuster must be nonzero");

        scale = pps.userObjectValue(i) / to_adjuster;
      }

      dof_id_type dof = elem->dof_number(sys_num, var_num, 0);
      to_solution.set(dof, scale * to_solution(dof));
    }
  }

  to_solution.close();
  to_sys.update();
}

void
MultiAppConservativeTransfer::adjustTransferedSolution(FEProblemBase * from_problem,
                                                       PostprocessorName & from_postprocessor,
                                                       FEProblemBase & to_problem,
                                                       PostprocessorName & to_postprocessor)
{
  PostprocessorValue from_adjuster = 0;
  if (from_problem)
    from_adjuster = from_problem->getPostprocessorValue(from_postprocessor);
  else
    from_adjuster = 0;

  /* Everyone on master side should know this value, and use it to scale the solution */
  if (_current_direction == FROM_MULTIAPP)
  {
    /* In this case, only one subapp has value, and other subapps' must be zero.
     *  We should see the maximum value.
     */
    PostprocessorValue from_adjuster_tmp = from_adjuster;
    comm().max(from_adjuster);

    /* We may have a negative value, and let us try it again  */
    if (MooseUtils::absoluteFuzzyLessEqual(from_adjuster, 0.))
    {
      comm().min(from_adjuster_tmp);
      from_adjuster = from_adjuster_tmp;
    }

    /* We should not have a zero value */
    if (MooseUtils::absoluteFuzzyLessEqual(from_adjuster, 0.))
      mooseError("from_adjuster must be nonzero");
  }

  // Compute to-postproessor to have the adjuster
  to_problem.computeUserObjectByName(EXEC_TRANSFER, to_postprocessor);

  // Now we should have the right adjuster based on the transfered solution
  PostprocessorValue & to_adjuster = to_problem.getPostprocessorValue(to_postprocessor);
  if (MooseUtils::absoluteFuzzyLessEqual(to_adjuster, 0.))
    mooseError(" To postproessor has a zero value ");

  auto & to_var = to_problem.getVariable(
      0, _to_var_name, Moose::VarKindType::VAR_ANY, Moose::VarFieldType::VAR_FIELD_STANDARD);
  auto & to_sys = to_var.sys().system();
  auto var_num = to_sys.variable_number(_to_var_name);
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
        auto & node_to_elem_map = moose_mesh.nodeToElemPtrMap();
        auto neighbor_elements = node_to_elem_map.find(node->id());
        for (const auto & elem : neighbor_elements->second)
        {
          if (blockids.find(elem->subdomain_id()) != blockids.end() ||
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
        unsigned int n_comp = elem->n_comp(sys_num, var_num);

        for (unsigned int offset = 0; offset < n_comp; offset++)
        {
          dof_id_type dof = elem->dof_number(sys_num, var_num, offset);
          to_solution.set(dof, (from_adjuster / to_adjuster) * to_solution(dof));
        }
      }
    }
  }

  to_solution.close();
  to_sys.update();

  // Compute again so that the post-processor has the value with the updated solution
  to_problem.computeUserObjectByName(EXEC_TRANSFER, to_postprocessor);
}
