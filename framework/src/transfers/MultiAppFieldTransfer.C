//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "MultiAppFieldTransfer.h"
#include "FEProblemBase.h"
#include "MooseVariableFEBase.h"
#include "MooseMesh.h"
#include "SystemBase.h"

#include "libmesh/system.h"
#include "libmesh/id_types.h"
#include "libmesh/string_to_enum.h"

defineLegacyParams(MultiAppFieldTransfer);

InputParameters
MultiAppFieldTransfer::validParams()
{
  InputParameters params = MultiAppTransfer::validParams();
  params.addParam<Real>(
      "shrink_gap_width",
      0,
      "gap width with which we want to temporarily shrink mesh in transfering solution");

  MooseEnum shrink_type("SOURCE TARGET", "SOURCE");
  params.addParam<MooseEnum>("shrink_mesh", shrink_type, "Which mesh we want to shrink");
  return params;
}

MultiAppFieldTransfer::MultiAppFieldTransfer(const InputParameters & parameters)
  : MultiAppTransfer(parameters),
    _shrink_gap_width(getParam<Real>("shrink_gap_width")),
    _shrink_mesh(getParam<MooseEnum>("shrink_mesh"))
{
}

void
MultiAppFieldTransfer::initialSetup()
{
  if (_current_direction == TO_MULTIAPP)
    for (auto & to_var : getToVarNames())
      variableIntegrityCheck(to_var);
  else
    for (auto & from_var : getFromVarNames())
      variableIntegrityCheck(from_var);
}

void
MultiAppFieldTransfer::computeTransformation(
    MooseMesh & mesh, std::unordered_map<dof_id_type, Point> & transformation)
{
  auto & libmesh_mesh = mesh.getMesh();

  auto & subdomainids = mesh.meshSubdomains();

  int max_subdomain_id = 0;

  for (auto subdomain_id : subdomainids)
  {
    max_subdomain_id = max_subdomain_id > subdomain_id ? max_subdomain_id : subdomain_id;
  }

  max_subdomain_id += 1;

  std::unordered_map<dof_id_type, Point> subdomain_centers;
  std::unordered_map<dof_id_type, dof_id_type> nelems;

  for (auto & elem :
       as_range(libmesh_mesh.local_elements_begin(), libmesh_mesh.local_elements_end()))
  {
    subdomain_centers[max_subdomain_id] += elem->centroid();
    nelems[max_subdomain_id] += 1;

    auto subdomain = elem->subdomain_id();

    if (subdomain == Moose::INVALID_BLOCK_ID)
      mooseError("block is invalid");

    subdomain_centers[subdomain] += elem->centroid();

    nelems[subdomain] += 1;
  }

  comm().sum(subdomain_centers);

  comm().sum(nelems);

  subdomain_centers[max_subdomain_id] /= nelems[max_subdomain_id];

  for (auto subdomain_id : subdomainids)
  {
    subdomain_centers[subdomain_id] /= nelems[subdomain_id];
  }

  transformation.clear();
  for (auto subdomain_id : subdomainids)
  {
    transformation[subdomain_id] =
        subdomain_centers[max_subdomain_id] - subdomain_centers[subdomain_id];

    auto norm = transformation[subdomain_id].norm();

    if (norm > 1e-10)
      transformation[subdomain_id] /= norm;

    transformation[subdomain_id] *= _shrink_gap_width;
  }
}

void
MultiAppFieldTransfer::transferDofObject(libMesh::DofObject * to_object,
                                         libMesh::DofObject * from_object,
                                         MooseVariableFEBase & to_var,
                                         MooseVariableFEBase & from_var)
{
  for (unsigned int vc = 0; vc < to_var.count(); ++vc)
    if (to_object->n_dofs(to_var.sys().number(), to_var.number() + vc) >
        0) // If this variable has dofs at this node
      for (unsigned int comp = 0;
           comp < to_object->n_comp(to_var.sys().number(), to_var.number() + vc);
           ++comp)
      {
        dof_id_type dof = to_object->dof_number(to_var.sys().number(), to_var.number() + vc, comp);
        dof_id_type from_dof =
            from_object->dof_number(from_var.sys().number(), from_var.number() + vc, comp);
        Real from_value = from_var.sys().solution()(from_dof);
        to_var.sys().solution().set(dof, from_value);
      }
}

void
MultiAppFieldTransfer::transfer(FEProblemBase & to_problem, FEProblemBase & from_problem)
{
  // Perform error checking
  if (getToVarNames().size() != getFromVarNames().size())
    mooseError("Number of variables transfered must be same in both systems.");
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

    // Check integrity
    if (to_var.feType() != from_var.feType())
      paramError("variable",
                 "Corresponding 'variable' and 'source_variable' inputs must be the same type "
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
      mooseError("The meshes must be identical to utilize MultiAppCopyTransfer.");

    // Transfer node dofs
    for (const auto & node : as_range(to_mesh.local_nodes_begin(), to_mesh.local_nodes_end()))
      transferDofObject(node, from_mesh.node_ptr(node->id()), to_var, from_var);

    // Transfer elem dofs
    for (auto & to_elem : as_range(to_mesh.local_elements_begin(), to_mesh.local_elements_end()))
    {
      Elem * from_elem = from_mesh.elem_ptr(to_elem->id());
      mooseAssert(to_elem->type() == from_elem->type(), "The elements must be the same type.");
      transferDofObject(to_elem, from_elem, to_var, from_var);
    }

    to_var.sys().solution().close();
    to_var.sys().update();
  }
}
