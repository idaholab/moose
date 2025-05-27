//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LinearFVTurbulentAdvection.h"
#include "MooseLinearVariableFV.h"
#include "NSFVUtils.h"
#include "NavierStokesMethods.h"
#include "NS.h"

registerMooseObject("NavierStokesApp", LinearFVTurbulentAdvection);

InputParameters
LinearFVTurbulentAdvection::validParams()
{
  InputParameters params = LinearFVScalarAdvection::validParams();
  params.addClassDescription("Represents the matrix and right hand side contributions of an "
                             "advection term for a turbulent variable.");

  params.addParam<std::vector<BoundaryName>>(
      "walls", {}, "Boundaries that correspond to solid walls.");

  return params;
}

LinearFVTurbulentAdvection::LinearFVTurbulentAdvection(const InputParameters & params)
  : LinearFVScalarAdvection(params),
    _advected_interp_coeffs(std::make_pair<Real, Real>(0, 0)),
    _wall_boundary_names(getParam<std::vector<BoundaryName>>("walls"))
{
  Moose::FV::setInterpolationMethod(*this, _advected_interp_method, "advected_interp_method");
}

void
LinearFVTurbulentAdvection::initialSetup()
{
  LinearFVScalarAdvection::initialSetup();
  NS::getWallBoundedElements(
      _wall_boundary_names, _fe_problem, _subproblem, blockIDs(), _wall_bounded);
}

void
LinearFVTurbulentAdvection::addMatrixContribution()
{
  // Coumputing bounding map
  const Elem * elem = _current_face_info->elemPtr();
  const auto bounded_elem = _wall_bounded.find(elem) != _wall_bounded.end();
  const Elem * neighbor = _current_face_info->neighborPtr();
  const auto bounded_neigh = _wall_bounded.find(neighbor) != _wall_bounded.end();

  // If we are on an internal face, we populate the four entries in the system matrix
  // which touch the face
  if (_current_face_type == FaceInfo::VarFaceNeighbors::BOTH)
  {
    // The dof ids of the variable corresponding to the element and neighbor
    _dof_indices(0) = _current_face_info->elemInfo()->dofIndices()[_sys_num][_var_num];
    _dof_indices(1) = _current_face_info->neighborInfo()->dofIndices()[_sys_num][_var_num];

    // Compute the entries which will go to the neighbor (offdiagonal) and element
    // (diagonal).
    const auto elem_matrix_contribution = computeElemMatrixContribution();
    const auto neighbor_matrix_contribution = computeNeighborMatrixContribution();

    // Populate matrix
    if (hasBlocks(_current_face_info->elemInfo()->subdomain_id()) && !(bounded_elem))
    {
      _matrix_contribution(0, 0) = elem_matrix_contribution;
      _matrix_contribution(0, 1) = neighbor_matrix_contribution;
    }

    if (hasBlocks(_current_face_info->neighborInfo()->subdomain_id()) && !(bounded_neigh))
    {
      _matrix_contribution(1, 0) = -elem_matrix_contribution;
      _matrix_contribution(1, 1) = -neighbor_matrix_contribution;
    }
    (*_linear_system.matrix).add_matrix(_matrix_contribution, _dof_indices.get_values());
  }
  // We are at a block boundary where the variable is not defined on one of the adjacent cells.
  // We check if we have a boundary condition here
  else if (_current_face_type == FaceInfo::VarFaceNeighbors::ELEM ||
           _current_face_type == FaceInfo::VarFaceNeighbors::NEIGHBOR)
  {
    mooseAssert(_current_face_info->boundaryIDs().size() == 1,
                "We should only have one boundary on every face.");

    LinearFVBoundaryCondition * bc_pointer =
        _var.getBoundaryCondition(*_current_face_info->boundaryIDs().begin());

    if (bc_pointer || _force_boundary_execution)
    {
      if (bc_pointer)
        bc_pointer->setupFaceData(_current_face_info, _current_face_type);
      const auto matrix_contribution = computeBoundaryMatrixContribution(*bc_pointer);

      // We allow internal (for the mesh) boundaries too, so we have to check on which side we
      // are on (assuming that this is a boundary for the variable)
      if ((_current_face_type == FaceInfo::VarFaceNeighbors::ELEM) && !(bounded_elem))
      {
        const auto dof_id_elem = _current_face_info->elemInfo()->dofIndices()[_sys_num][_var_num];
        (*_linear_system.matrix).add(dof_id_elem, dof_id_elem, matrix_contribution);
      }
      else if ((_current_face_type == FaceInfo::VarFaceNeighbors::NEIGHBOR) && !(bounded_neigh))
      {
        const auto dof_id_neighbor =
            _current_face_info->neighborInfo()->dofIndices()[_sys_num][_var_num];
        (*_linear_system.matrix).add(dof_id_neighbor, dof_id_neighbor, matrix_contribution);
      }
    }
  }
}

void
LinearFVTurbulentAdvection::addRightHandSideContribution()
{
  // Coumputing bounding map
  const Elem * elem = _current_face_info->elemPtr();
  const auto bounded_elem = _wall_bounded.find(elem) != _wall_bounded.end();
  const Elem * neighbor = _current_face_info->neighborPtr();
  const auto bounded_neigh = _wall_bounded.find(neighbor) != _wall_bounded.end();

  // If we are on an internal face, we populate the two entries in the right hand side
  // which touch the face
  if (_current_face_type == FaceInfo::VarFaceNeighbors::BOTH)
  {
    // The dof ids of the variable corresponding to the element and neighbor
    _dof_indices(0) = _current_face_info->elemInfo()->dofIndices()[_sys_num][_var_num];
    _dof_indices(1) = _current_face_info->neighborInfo()->dofIndices()[_sys_num][_var_num];

    // Compute the entries which will go to the neighbor and element positions.
    const auto elem_rhs_contribution = computeElemRightHandSideContribution();
    const auto neighbor_rhs_contribution = computeNeighborRightHandSideContribution();

    // Populate right hand side
    if (hasBlocks(_current_face_info->elemInfo()->subdomain_id()) && !(bounded_elem))
      _rhs_contribution(0) = elem_rhs_contribution;
    if (hasBlocks(_current_face_info->neighborInfo()->subdomain_id()) && !(bounded_neigh))
      _rhs_contribution(1) = neighbor_rhs_contribution;

    (*_linear_system.rhs)
        .add_vector(_rhs_contribution.get_values().data(), _dof_indices.get_values());
  }
  // We are at a block boundary where the variable is not defined on one of the adjacent cells.
  // We check if we have a boundary condition here
  else if (_current_face_type == FaceInfo::VarFaceNeighbors::ELEM ||
           _current_face_type == FaceInfo::VarFaceNeighbors::NEIGHBOR)
  {
    mooseAssert(_current_face_info->boundaryIDs().size() == 1,
                "We should only have one boundary on every face.");
    LinearFVBoundaryCondition * bc_pointer =
        _var.getBoundaryCondition(*_current_face_info->boundaryIDs().begin());

    if (bc_pointer || _force_boundary_execution)
    {
      if (bc_pointer)
        bc_pointer->setupFaceData(_current_face_info, _current_face_type);

      const auto rhs_contribution = computeBoundaryRHSContribution(*bc_pointer);

      // We allow internal (for the mesh) boundaries too, so we have to check on which side we
      // are on (assuming that this is a boundary for the variable)
      if ((_current_face_type == FaceInfo::VarFaceNeighbors::ELEM) && !(bounded_elem))
      {
        const auto dof_id_elem = _current_face_info->elemInfo()->dofIndices()[_sys_num][_var_num];
        (*_linear_system.rhs).add(dof_id_elem, rhs_contribution);
      }
      else if ((_current_face_type == FaceInfo::VarFaceNeighbors::NEIGHBOR) && !(bounded_neigh))
      {
        const auto dof_id_neighbor =
            _current_face_info->neighborInfo()->dofIndices()[_sys_num][_var_num];
        (*_linear_system.rhs).add(dof_id_neighbor, rhs_contribution);
      }
    }
  }
}
