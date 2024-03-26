//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GeneralizedPlaneStrainUserObjectOSPD.h"
#include "RankFourTensor.h"
#include "Function.h"

registerMooseObject("PeridynamicsApp", GeneralizedPlaneStrainUserObjectOSPD);

InputParameters
GeneralizedPlaneStrainUserObjectOSPD::validParams()
{
  InputParameters params = GeneralizedPlaneStrainUserObjectBasePD::validParams();
  params.addClassDescription("Class for calculating the scalar residual and diagonal Jacobian "
                             "entry of generalized plane strain in OSPD formulation");

  params.addCoupledVar("out_of_plane_stress_variable",
                       "Auxiliary variable name for out-of-plane stress in GPS simulation");

  return params;
}

GeneralizedPlaneStrainUserObjectOSPD::GeneralizedPlaneStrainUserObjectOSPD(
    const InputParameters & parameters)
  : GeneralizedPlaneStrainUserObjectBasePD(parameters),
    _out_of_plane_stress_var(getVar("out_of_plane_stress_variable", 0))
{
}

void
GeneralizedPlaneStrainUserObjectOSPD::execute()
{
  // dof_id_type for node i and j
  dof_id_type node_i = _current_elem->node_id(0);
  dof_id_type node_j = _current_elem->node_id(1);

  // coordinates for node i and j
  Point coord_i = _pdmesh.getNodeCoord(node_i);
  Point coord_j = _pdmesh.getNodeCoord(node_j);

  // nodal area for node i and j
  Real nv_i = _pdmesh.getNodeVolume(node_i);
  Real nv_j = _pdmesh.getNodeVolume(node_j);

  // number of neighbors for node i and j, used to avoid repeated accounting nodal stress in
  // element-wise loop

  // calculate number of active neighbors for node i and j
  std::vector<unsigned int> active_neighbors(_nnodes, 0);
  for (unsigned int nd = 0; nd < _nnodes; nd++)
  {
    std::vector<dof_id_type> bonds = _pdmesh.getBonds(_current_elem->node_id(nd));
    for (unsigned int nb = 0; nb < bonds.size(); ++nb)
      if (_bond_status_var->getElementalValue(_pdmesh.elemPtr(bonds[nb])) > 0.5)
        active_neighbors[nd]++;

    if (active_neighbors[nd] == 0) // avoid dividing by zero
      active_neighbors[nd] = 1;
  }

  Real bond_status = _bond_status_var->getElementalValue(_current_elem);

  // residual
  _residual += (_out_of_plane_stress_var->getNodalValue(*_current_elem->node_ptr(0)) -
                _pressure.value(_t, coord_i) * _factor) *
               nv_i / active_neighbors[0] * bond_status;
  _residual += (_out_of_plane_stress_var->getNodalValue(*_current_elem->node_ptr(1)) -
                _pressure.value(_t, coord_j) * _factor) *
               nv_j / active_neighbors[1] * bond_status;

  // diagonal jacobian
  _jacobian += (_Cijkl[0](2, 2, 2, 2) * nv_i / active_neighbors[0] +
                _Cijkl[0](2, 2, 2, 2) * nv_j / active_neighbors[1]) *
               bond_status;
}
