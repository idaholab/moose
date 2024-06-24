//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GeneralizedPlaneStrainUserObjectNOSPD.h"
#include "RankTwoTensor.h"
#include "RankFourTensor.h"
#include "Function.h"

registerMooseObject("PeridynamicsApp", GeneralizedPlaneStrainUserObjectNOSPD);

InputParameters
GeneralizedPlaneStrainUserObjectNOSPD::validParams()
{
  InputParameters params = GeneralizedPlaneStrainUserObjectBasePD::validParams();
  params.addClassDescription("Class for calculating the scalar residual and diagonal Jacobian "
                             "entry of generalized plane strain in the H1NOSPD formulation");

  return params;
}

GeneralizedPlaneStrainUserObjectNOSPD::GeneralizedPlaneStrainUserObjectNOSPD(
    const InputParameters & parameters)
  : GeneralizedPlaneStrainUserObjectBasePD(parameters),
    _stress(getMaterialProperty<RankTwoTensor>("stress"))
{
}

void
GeneralizedPlaneStrainUserObjectNOSPD::execute()
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

  // sum of volumes of material points used in bond-associated deformation gradient calculation
  dof_id_type id_j_in_i = _pdmesh.getNeighborIndex(node_i, node_j);
  dof_id_type id_i_in_j = _pdmesh.getNeighborIndex(node_j, node_i);

  Real dg_vol_frac_i = _pdmesh.getHorizonSubsetVolumeFraction(node_i, id_j_in_i);
  Real dg_vol_frac_j = _pdmesh.getHorizonSubsetVolumeFraction(node_j, id_i_in_j);

  Real bond_status = _bond_status_var->getElementalValue(_current_elem);

  // residual
  _residual += (_stress[0](2, 2) - _pressure.value(_t, coord_i) * _factor) * nv_i * dg_vol_frac_i *
               bond_status;
  _residual += (_stress[1](2, 2) - _pressure.value(_t, coord_j) * _factor) * nv_j * dg_vol_frac_j *
               bond_status;

  // diagonal jacobian
  _jacobian += (_Cijkl[0](2, 2, 2, 2) * nv_i * dg_vol_frac_i +
                _Cijkl[1](2, 2, 2, 2) * nv_j * dg_vol_frac_j) *
               bond_status;
}
