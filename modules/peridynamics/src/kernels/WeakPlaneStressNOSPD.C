//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "WeakPlaneStressNOSPD.h"
#include "MooseVariable.h"
#include "PeridynamicsMesh.h"
#include "RankTwoTensor.h"
#include "RankFourTensor.h"

registerMooseObject("PeridynamicsApp", WeakPlaneStressNOSPD);

template <>
InputParameters
validParams<WeakPlaneStressNOSPD>()
{
  InputParameters params = validParams<MechanicsBaseNOSPD>();
  params.addClassDescription("Class for calculating residual and Jacobian for peridynamic plane "
                             "stress model using weak formulation");

  return params;
}

WeakPlaneStressNOSPD::WeakPlaneStressNOSPD(const InputParameters & parameters)
  : MechanicsBaseNOSPD(parameters)
{
}

void
WeakPlaneStressNOSPD::computeLocalResidual()
{
  _local_re(0) =
      _stress[0](2, 2) * _dg_bond_vsum_ij[0] / _dg_node_vsum_ij[0] * _vols_ij[0] * _bond_status_ij;
  _local_re(1) =
      _stress[1](2, 2) * _dg_bond_vsum_ij[1] / _dg_node_vsum_ij[1] * _vols_ij[1] * _bond_status_ij;
}

void
WeakPlaneStressNOSPD::computeLocalJacobian()
{
  for (_i = 0; _i < _test.size(); _i++)
    for (_j = 0; _j < _phi.size(); _j++)
      _local_ke(_i, _j) += (_i == _j ? 1 : 0) * _Jacobian_mult[_i](2, 2, 2, 2) *
                           _dg_bond_vsum_ij[_i] / _dg_node_vsum_ij[_i] * _vols_ij[_i] *
                           _bond_status_ij;
}

void
WeakPlaneStressNOSPD::computeLocalOffDiagJacobian(unsigned int coupled_component)
{
  _local_ke.zero();
  if (coupled_component == 3) // off-diagonal Jacobian with respect to coupled temperature variable
  {
    std::vector<RankTwoTensor> dSdT(_nnodes);
    for (unsigned int nd = 0; nd < _nnodes; nd++)
      for (unsigned int es = 0; es < _deigenstrain_dT.size(); es++)
        dSdT[nd] = -_Jacobian_mult[nd] * (*_deigenstrain_dT[es])[nd];

    for (_i = 0; _i < _test.size(); _i++)
      for (_j = 0; _j < _phi.size(); _j++)
        _local_ke(_i, _j) += (_i == _j ? 1 : 0) * dSdT[_i](2, 2) * _dg_bond_vsum_ij[_i] /
                             _dg_node_vsum_ij[_i] * _vols_ij[_i] * _bond_status_ij;
  }
  else // off-diagonal Jacobian with respect to coupled displacement variables
  {
    // dSidUi and dSjdUj are considered as local off-diagonal Jacobian
    // contributions to their neighbors are considered as nonlocal off-diagonal
    for (_i = 0; _i < _test.size(); _i++)
      for (_j = 0; _j < _phi.size(); _j++)
        _local_ke(_i, _j) += (_i == _j ? 1 : 0) * computeDSDU(coupled_component, _j)(2, 2) *
                             _dg_bond_vsum_ij[_j] / _dg_node_vsum_ij[_j] * _vols_ij[_j] *
                             _bond_status_ij;
  }
}

void
WeakPlaneStressNOSPD::computePDNonlocalOffDiagJacobian(unsigned int jvar_num,
                                                       unsigned int coupled_component)
{
  if (coupled_component != 3)
  {
    for (unsigned int cur_nd = 0; cur_nd < _nnodes; cur_nd++)
    {
      // calculation of jacobian contribution to current_node's neighbors
      // NOT including the contribution to nodes i and j, which is considered as local off-diagonal
      std::vector<dof_id_type> jvardofs_ijk(_nnodes);
      jvardofs_ijk[0] = _current_elem->node_ptr(cur_nd)->dof_number(_sys.number(), jvar_num, 0);
      std::vector<dof_id_type> neighbors = _pdmesh.getNeighbors(_current_elem->node_id(cur_nd));
      unsigned int nb =
          std::find(neighbors.begin(), neighbors.end(), _current_elem->node_id(1 - cur_nd)) -
          neighbors.begin();
      std::vector<unsigned int> BAneighbors =
          _pdmesh.getBondAssocHorizonNeighbors(_current_elem->node_id(cur_nd), nb);
      std::vector<dof_id_type> bonds = _pdmesh.getAssocBonds(_current_elem->node_id(cur_nd));
      for (unsigned int k = 0; k < BAneighbors.size(); k++)
      {
        const Node * node_k = _pdmesh.nodePtr(neighbors[BAneighbors[k]]);
        jvardofs_ijk[1] = node_k->dof_number(_sys.number(), jvar_num, 0);
        const Real vol_k = _pdmesh.getVolume(neighbors[BAneighbors[k]]);

        // obtain bond k's origin vector
        const RealGradient origin_vec_ijk =
            *node_k - *_pdmesh.nodePtr(_current_elem->node_id(cur_nd));

        RankTwoTensor dFdUk;
        dFdUk.zero();
        for (unsigned int j = 0; j < _dim; j++)
          dFdUk(coupled_component, j) =
              _horizons_ij[cur_nd] / origin_vec_ijk.norm() * origin_vec_ijk(j) * vol_k;

        dFdUk *= _shape[cur_nd].inverse();

        RankTwoTensor dPdUk =
            _Jacobian_mult[cur_nd] * 0.5 *
            (dFdUk.transpose() * _dgrad[cur_nd] + _dgrad[cur_nd].transpose() * dFdUk);

        // bond status for bond k
        Real bond_status_ijk =
            _bond_status_var.getElementalValue(_pdmesh.elemPtr(bonds[BAneighbors[k]]));

        _local_ke.zero();
        _local_ke(cur_nd, 1) = dPdUk(2, 2) * _dg_bond_vsum_ij[cur_nd] / _dg_node_vsum_ij[cur_nd] *
                               _vols_ij[cur_nd] * _bond_status_ij * bond_status_ijk;

        _assembly.cacheJacobianBlock(_local_ke, _ivardofs_ij, jvardofs_ijk, _var.scalingFactor());
      }
    }
  }
}
