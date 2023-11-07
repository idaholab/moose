//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TestElemInfo.h"

// MOOSE includes
#include "MooseMesh.h"
#include "SystemBase.h"

registerMooseObject("MooseTestApp", TestElemInfo);

InputParameters
TestElemInfo::validParams()
{
  InputParameters params = GeneralVectorPostprocessor::validParams();
  params.addParam<std::vector<VariableName>>("vars", "Variable names.");
  params.addClassDescription("Computes element quantities like area, neighbors, normals, etc.");
  return params;
}

TestElemInfo::TestElemInfo(const InputParameters & parameters)
  : GeneralVectorPostprocessor(parameters),
    _elem_id(declareVector("id")),
    _elem_volume(declareVector("volume")),
    _cx(declareVector("cx")),
    _cy(declareVector("cy")),
    _cz(declareVector("cz"))
{
  if (isParamValid("vars"))
  {
    _vars = getParam<std::vector<VariableName>>("vars");
    for (auto & v : _vars)
      _var_dof_indices.push_back(&declareVector(v + "_dof_indices"));
  }
}

void
TestElemInfo::execute()
{
  // We need this because on the mesh it is stored in an unordered map and the hashing can be
  // dependent on the std library of the specific architecture
  std::map<dof_id_type, const ElemInfo *> ordered_elem_info;

  for (const auto & ei : _fe_problem.mesh().elemInfoVector())
    ordered_elem_info[ei->elem()->id()] = ei;

  for (const auto & ei_pair : ordered_elem_info)
  {
    const auto & elem_info = ei_pair.second;
    _elem_id.push_back(elem_info->elem()->id());
    _elem_volume.push_back(elem_info->volume() * elem_info->coordFactor());

    const Point & centroid = elem_info->centroid();
    _cx.push_back(centroid(0));
    _cy.push_back(centroid(1));
    _cz.push_back(centroid(2));

    for (const auto var_i : index_range(_vars))
    {
      const auto & var = _subproblem.getVariable(0, _vars[var_i]);
      const auto system_number = var.sys().number();
      const auto variable_number = var.number();
      const auto & dof_indices = elem_info->dofIndices();

      // We need this trick because the invalid_dof_id can depend on the the
      // compiler. For example, with the minimum clang version it is O(1E9) whereas with
      // other versions it is O(1E19). This would result in CSV diffs.
      if (dof_indices[system_number][variable_number] != libMesh::DofObject::invalid_id)
        _var_dof_indices[var_i]->push_back(dof_indices[system_number][variable_number]);
      else
        _var_dof_indices[var_i]->push_back(-1);
    }
  }
}
