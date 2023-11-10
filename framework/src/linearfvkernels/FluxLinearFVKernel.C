//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FluxLinearFVKernel.h"
#include "Assembly.h"
#include "SubProblem.h"

InputParameters
FluxLinearFVKernel::validParams()
{
  InputParameters params = LinearFVKernel::validParams();
  params.registerSystemAttributeName("FluxLinearFVKernel");
  return params;
}

FluxLinearFVKernel::FluxLinearFVKernel(const InputParameters & params)
  : LinearFVKernel(params), _current_face_info(nullptr)
{
}

void
FluxLinearFVKernel::addMatrixContribution()
{
  if (_current_face_info->faceType(std::make_pair(_var->number(), _var->sys().number())) ==
      FaceInfo::VarFaceNeighbors::BOTH)
  {
    const auto dof_id_elem =
        _current_face_info->elemInfo()->dofIndices()[_var->sys().number()][_var->number()];
    const auto dof_id_neighbor =
        _current_face_info->neighborInfo()->dofIndices()[_var->sys().number()][_var->number()];

    const auto elem_matrix_contribution = computeElemMatrixContribution();
    const auto neighbor_matrix_contribution = computeNeighborMatrixContribution();

    (*_linear_system.matrix).add(dof_id_elem, dof_id_elem, elem_matrix_contribution);
    (*_linear_system.matrix).add(dof_id_elem, dof_id_neighbor, neighbor_matrix_contribution);
  }
}

void
FluxLinearFVKernel::addRightHandSideContribution()
{
  if (_current_face_info->faceType(std::make_pair(_var->number(), _var->sys().number())) ==
      FaceInfo::VarFaceNeighbors::BOTH)
  {
    const auto dof_id_elem =
        _current_face_info->elemInfo()->dofIndices()[_var->sys().number()][_var->number()];
    const auto dof_id_neighbor =
        _current_face_info->neighborInfo()->dofIndices()[_var->sys().number()][_var->number()];

    const auto elem_rhs_contribution = computeElemRightHandSideContribution();
    const auto neighbor_rhs_contribution = computeNeighborRightHandSideContribution();

    (*_linear_system.rhs).add(dof_id_elem, elem_rhs_contribution);
    (*_linear_system.rhs).add(dof_id_neighbor, neighbor_rhs_contribution);
  }
}
