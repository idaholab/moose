//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LinearFVFluxKernel.h"
#include "LinearFVBoundaryCondition.h"

InputParameters
LinearFVFluxKernel::validParams()
{
  InputParameters params = LinearFVKernel::validParams();
  params.registerSystemAttributeName("LinearFVFluxKernel");
  return params;
}

LinearFVFluxKernel::LinearFVFluxKernel(const InputParameters & params)
  : LinearFVKernel(params),
    FaceArgProducerInterface(),
    _current_face_info(nullptr),
    _current_face_type(FaceInfo::VarFaceNeighbors::NEITHER),
    _cached_matrix_contribution(false),
    _cached_rhs_contribution(false)
{
}

void
LinearFVFluxKernel::addMatrixContribution()
{
  // If we are on an internal face, we populate the four entries in the system matrix
  // which touch the face
  if (_current_face_type == FaceInfo::VarFaceNeighbors::BOTH)
  {
    // The dof ids of the variable corresponding to the element and neighbor
    const auto dof_id_elem =
        _current_face_info->elemInfo()->dofIndices()[_var.sys().number()][_var.number()];
    const auto dof_id_neighbor =
        _current_face_info->neighborInfo()->dofIndices()[_var.sys().number()][_var.number()];

    // Compute the entries which will go to the neighbor (offdiagonal) and element
    // (diagonal).
    const auto elem_matrix_contribution = computeElemMatrixContribution();
    const auto neighbor_matrix_contribution = computeNeighborMatrixContribution();

    // Populate matrix
    (*_linear_system.matrix).add(dof_id_elem, dof_id_elem, elem_matrix_contribution);
    (*_linear_system.matrix).add(dof_id_elem, dof_id_neighbor, neighbor_matrix_contribution);
    (*_linear_system.matrix).add(dof_id_neighbor, dof_id_elem, -elem_matrix_contribution);
    (*_linear_system.matrix).add(dof_id_neighbor, dof_id_neighbor, -neighbor_matrix_contribution);
  }
  else if (auto * bc_pointer =
               _var.getBoundaryCondition(*_current_face_info->boundaryIDs().begin()))
  {
    mooseAssert(_current_face_info->boundaryIDs().size() == 1,
                "We should only have one boundary on every face.");
    bc_pointer->setCurrentFaceInfo(_current_face_info, _current_face_type);
    const auto matrix_contribution = computeBoundaryMatrixContribution(*bc_pointer);

    // We allow internal boundaries too, so we have to check on which side we
    // are on
    if (_current_face_type == FaceInfo::VarFaceNeighbors::ELEM)
    {
      const auto dof_id_elem =
          _current_face_info->elemInfo()->dofIndices()[_var.sys().number()][_var.number()];
      (*_linear_system.matrix).add(dof_id_elem, dof_id_elem, matrix_contribution);
    }
    else if (_current_face_type == FaceInfo::VarFaceNeighbors::NEIGHBOR)
    {
      const auto dof_id_neighbor =
          _current_face_info->neighborInfo()->dofIndices()[_var.sys().number()][_var.number()];
      (*_linear_system.matrix).add(dof_id_neighbor, dof_id_neighbor, matrix_contribution);
    }
  }
}

void
LinearFVFluxKernel::addRightHandSideContribution()
{
  // If we are on an internal face, we populate the two entries in the right hand side
  // which touch the face
  if (_current_face_type == FaceInfo::VarFaceNeighbors::BOTH)
  {
    // The dof ids of the variable corresponding to the element and neighbor
    const auto dof_id_elem =
        _current_face_info->elemInfo()->dofIndices()[_var.sys().number()][_var.number()];
    const auto dof_id_neighbor =
        _current_face_info->neighborInfo()->dofIndices()[_var.sys().number()][_var.number()];

    // Compute the entries which will go to the neighbor and element positions.
    const auto elem_rhs_contribution = computeElemRightHandSideContribution();
    const auto neighbor_rhs_contribution = computeNeighborRightHandSideContribution();

    // Populate right hand side
    (*_linear_system.rhs).add(dof_id_elem, elem_rhs_contribution);
    (*_linear_system.rhs).add(dof_id_neighbor, neighbor_rhs_contribution);
  }
  else if (auto * bc_pointer =
               _var.getBoundaryCondition(*_current_face_info->boundaryIDs().begin()))
  {
    bc_pointer->setCurrentFaceInfo(_current_face_info, _current_face_type);
    const auto rhs_contribution = computeBoundaryRHSContribution(*bc_pointer);

    // We allow internal boundaries too, so we have to check on which side we
    // are on
    if (_current_face_type == FaceInfo::VarFaceNeighbors::ELEM)
    {
      const auto dof_id_elem =
          _current_face_info->elemInfo()->dofIndices()[_var.sys().number()][_var.number()];
      (*_linear_system.rhs).add(dof_id_elem, rhs_contribution);
    }
    else if (_current_face_type == FaceInfo::VarFaceNeighbors::NEIGHBOR)
    {
      const auto dof_id_neighbor =
          _current_face_info->neighborInfo()->dofIndices()[_var.sys().number()][_var.number()];
      (*_linear_system.rhs).add(dof_id_neighbor, rhs_contribution);
    }
  }
}

bool
LinearFVFluxKernel::hasFaceSide(const FaceInfo & fi, bool fi_elem_side) const
{
  const auto ft = fi.faceType(std::make_pair(_var.number(), _var.sys().number()));
  if (fi_elem_side)
    return ft == FaceInfo::VarFaceNeighbors::ELEM || ft == FaceInfo::VarFaceNeighbors::BOTH;
  else
    return ft == FaceInfo::VarFaceNeighbors::NEIGHBOR || ft == FaceInfo::VarFaceNeighbors::BOTH;
}

Moose::FaceArg
LinearFVFluxKernel::singleSidedFaceArg(const FaceInfo * fi,
                                       const Moose::FV::LimiterType limiter_type,
                                       const bool correct_skewness) const
{
  mooseAssert(fi, "FaceInfo should not be null!");
  return makeFace(*fi, limiter_type, true, correct_skewness);
}

void
LinearFVFluxKernel::setCurrentFaceInfo(const FaceInfo * face_info)
{
  _cached_matrix_contribution = false;
  _cached_rhs_contribution = false;
  _current_face_info = face_info;
  _current_face_type =
      _current_face_info->faceType(std::make_pair(_var.number(), _var.sys().number()));
}
