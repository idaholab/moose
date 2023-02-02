//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "MultiAppDofCopyTransfer.h"
#include "FEProblemBase.h"
#include "MultiApp.h"
#include "SystemBase.h"

#include "libmesh/id_types.h"
#include "libmesh/string_to_enum.h"

InputParameters
MultiAppDofCopyTransfer::validParams()
{
  InputParameters params = MultiAppFieldTransfer::validParams();

  params.addParam<TagName>(
      "from_solution_tag",
      "The tag of the solution vector to be transferred (default to the solution)");
  params.addParam<TagName>(
      "to_solution_tag",
      "The tag of the solution vector to be transferred to (default to the solution)");

  // Block restrictions
  params.addParam<std::vector<SubdomainName>>(
      "from_blocks",
      "Subdomain restriction to transfer from (defaults to all the origin app domain)");
  params.addParam<std::vector<SubdomainName>>(
      "to_blocks", "Subdomain restriction to transfer to, (defaults to all the target app domain)");

  params.addClassDescription(
      "Base class for copying degrees-of-freedom values (nonlinear and auxiliary) between apps "
      "that have identical meshes.");
  return params;
}

MultiAppDofCopyTransfer::MultiAppDofCopyTransfer(const InputParameters & parameters)
  : MultiAppFieldTransfer(parameters),
    _has_block_restrictions(isParamValid("from_blocks") || isParamValid("to_blocks"))
{
}

void
MultiAppDofCopyTransfer::initialSetup()
{
  MultiAppFieldTransfer::initialSetup();

  const FEProblemBase * from_problem;
  const FEProblemBase * to_problem;

  if (_current_direction == FROM_MULTIAPP)
  {
    // Subdomain and variable type information is shared on all subapps
    from_problem = &getFromMultiApp()->appProblemBase(getFromMultiApp()->firstLocalApp());
    to_problem = &getFromMultiApp()->problemBase();
  }
  else if (_current_direction == TO_MULTIAPP)
  {
    from_problem = &getToMultiApp()->problemBase();
    to_problem = &getToMultiApp()->appProblemBase(getToMultiApp()->firstLocalApp());
  }
  else
  {
    from_problem = &getFromMultiApp()->appProblemBase(getFromMultiApp()->firstLocalApp());
    to_problem = &getToMultiApp()->appProblemBase(getToMultiApp()->firstLocalApp());
  }

  // Convert block names to block IDs, fill with ANY_BLOCK_ID if unspecified
  if (_has_block_restrictions)
  {
    const auto & from_block_names = getParam<std::vector<SubdomainName>>("from_blocks");
    for (const auto & b : from_block_names)
      if (!MooseMeshUtils::hasSubdomainName(const_cast<MeshBase &>(from_problem->mesh().getMesh()),
                                            b))
        paramError("from_blocks", "The block '", b, "' was not found in the mesh");

    if (from_block_names.size())
    {
      if (from_problem)
      {
        const auto block_vec = from_problem->mesh().getSubdomainIDs(from_block_names);
        _from_blocks = std::set<SubdomainID>(block_vec.begin(), block_vec.end());
      }
      // We dont even own any of these subapps
      else
        _from_blocks = {Moose::INVALID_BLOCK_ID};
    }
    else
      _from_blocks = {Moose::ANY_BLOCK_ID};

    const auto & to_block_names = getParam<std::vector<SubdomainName>>("to_blocks");
    for (const auto & b : to_block_names)
      if (!MooseMeshUtils::hasSubdomainName(const_cast<MeshBase &>(to_problem->mesh().getMesh()),
                                            b))
        paramError("to_blocks", "The block '", b, "' was not found in the mesh");

    if (to_block_names.size())
    {
      if (to_problem)
      {
        const auto block_vec = to_problem->mesh().getSubdomainIDs(to_block_names);
        _to_blocks = std::set<SubdomainID>(block_vec.begin(), block_vec.end());
      }
      // We dont even own any of these subapps
      else
        _from_blocks = {Moose::INVALID_BLOCK_ID};
    }
    else
      _to_blocks = {Moose::ANY_BLOCK_ID};
  }

  // Forbid block restriction on nodal variables as currently not supported
  if (_from_blocks.size())
    for (auto & from_var : getFromVarNames())
      if (from_problem
              ->getVariable(
                  0, from_var, Moose::VarKindType::VAR_ANY, Moose::VarFieldType::VAR_FIELD_ANY)
              .hasDoFsOnNodes())
        paramError("from_blocks", "Block restriction is not implemented for nodal variables");
  if (_to_blocks.size())
    for (auto & to_var : getToVarNames())
      if (to_problem
              ->getVariable(
                  0, to_var, Moose::VarKindType::VAR_ANY, Moose::VarFieldType::VAR_FIELD_ANY)
              .hasDoFsOnNodes())
        paramError("to_blocks", "Block restriction is not implemented for nodal variables");
}

void
MultiAppDofCopyTransfer::transfer(FEProblemBase & to_problem, FEProblemBase & from_problem)
{
  // Perform error checking
  if (!getToVarNames().size())
    mooseError("No transferred variables were specified, neither programmatically or through the "
               "'source_variable' parameter");
  if (getToVarNames().size() != getFromVarNames().size())
    mooseError("Number of variables transferred must be same in both systems.");
  for (auto & to_var : getToVarNames())
    checkVariable(to_problem, to_var);
  for (auto & from_var : getFromVarNames())
    checkVariable(from_problem, from_var);

  for (unsigned int v = 0; v < getToVarNames().size(); ++v)
  {
    // Populate the to/from variables needed to perform the transfer
    MooseVariableFEBase & to_var = to_problem.getVariable(
        0, getToVarNames()[v], Moose::VarKindType::VAR_ANY, Moose::VarFieldType::VAR_FIELD_ANY);
    MeshBase & to_mesh = to_problem.mesh().getMesh();

    MooseVariableFEBase & from_var = from_problem.getVariable(
        0, getFromVarNames()[v], Moose::VarKindType::VAR_ANY, Moose::VarFieldType::VAR_FIELD_ANY);
    MeshBase & from_mesh = from_problem.mesh().getMesh();

    auto & to_solution = isParamValid("to_solution_tag")
                             ? to_var.sys().getVector(
                                   to_problem.getVectorTagID(getParam<TagName>("to_solution_tag")))
                             : to_var.sys().solution();
    auto & from_solution = isParamValid("from_solution_tag")
                               ? from_var.sys().getVector(from_problem.getVectorTagID(
                                     getParam<TagName>("from_solution_tag")))
                               : from_var.sys().solution();

    // Check integrity
    if (to_var.feType() != from_var.feType())
      mooseError("MultiAppFieldTransfer '",
                 name(),
                 "'requires that the target variable '",
                 to_var.name(),
                 "' and the source variable'",
                 from_var.name(),
                 "' must be the same type "
                 "(order and family): ",
                 libMesh::Utility::enum_to_string<FEFamily>(to_var.feType().family),
                 moose::internal::incompatVarMsg(to_var, from_var));
    if (to_var.fieldType() != from_var.fieldType())
      mooseError(
          "Corresponding transfer variables must be same field type (STANDARD | VECTOR | ARRAY).");
    if (to_var.fieldType() == Moose::VarFieldType::VAR_FIELD_VECTOR)
      mooseError("Unable to transfer vector variables.");
    if (to_var.count() != from_var.count())
      mooseError("Corresponding transfer variables must have same number of components.");

    if ((to_mesh.n_nodes() != from_mesh.n_nodes()) || (to_mesh.n_elem() != from_mesh.n_elem()))
      mooseError("The meshes must be identical to utilize MultiAppDofCopyTransfer::transfer.");

    // Transfer node dofs. Block restriction is not supported, forbidden in initialSetup
    for (const auto & node : as_range(to_mesh.local_nodes_begin(), to_mesh.local_nodes_end()))
      transferDofObject(
          node, from_mesh.node_ptr(node->id()), to_var, from_var, to_solution, from_solution);

    // Transfer elem dofs
    for (auto & to_elem : as_range(to_mesh.local_elements_begin(), to_mesh.local_elements_end()))
    {
      Elem * from_elem = from_mesh.elem_ptr(to_elem->id());
      mooseAssert(to_elem->type() == from_elem->type(), "The elements must be the same type.");

      // Examine block restriction
      if (_has_block_restrictions)
      {
        if (std::find(_from_blocks.begin(), _from_blocks.end(), Moose::ANY_BLOCK_ID) ==
            _from_blocks.end())
        {
          SubdomainID from_block = from_elem->subdomain_id();
          if (std::find(_from_blocks.begin(), _from_blocks.end(), from_block) == _from_blocks.end())
            continue;
        }

        if (std::find(_to_blocks.begin(), _to_blocks.end(), Moose::ANY_BLOCK_ID) ==
            _to_blocks.end())
        {
          SubdomainID to_block = to_elem->subdomain_id();
          if (std::find(_to_blocks.begin(), _to_blocks.end(), to_block) == _to_blocks.end())
            continue;
        }
      }

      transferDofObject(to_elem, from_elem, to_var, from_var, to_solution, from_solution);
    }

    to_solution.close();
    to_var.sys().update();
  }
}

void
MultiAppDofCopyTransfer::transferDofObject(libMesh::DofObject * to_object,
                                           libMesh::DofObject * from_object,
                                           MooseVariableFEBase & to_var,
                                           MooseVariableFEBase & from_var,
                                           NumericVector<Number> & to_solution,
                                           NumericVector<Number> & from_solution)
{
  for (unsigned int vc = 0; vc < to_var.count(); ++vc)
    // Transfer from one solution vector to another
    if (to_object->n_dofs(to_var.sys().number(), to_var.number() + vc) >
        0) // If this variable has dofs at this node
      for (unsigned int comp = 0;
           comp < to_object->n_comp(to_var.sys().number(), to_var.number() + vc);
           ++comp)
      {
        dof_id_type dof = to_object->dof_number(to_var.sys().number(), to_var.number() + vc, comp);
        dof_id_type from_dof =
            from_object->dof_number(from_var.sys().number(), from_var.number() + vc, comp);
        Real from_value = from_solution(from_dof);
        to_solution.set(dof, from_value);
      }
}
