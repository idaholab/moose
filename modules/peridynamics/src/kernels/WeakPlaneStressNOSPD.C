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

registerMooseObject("PeridynamicsApp", WeakPlaneStressNOSPD);

InputParameters
WeakPlaneStressNOSPD::validParams()
{
  InputParameters params = MechanicsBaseNOSPD::validParams();
  params.addClassDescription(
      "Class for calculating the residual and the Jacobian for the peridynamic plane "
      "stress model using weak formulation based on peridynamic correspondence models");

  return params;
}

WeakPlaneStressNOSPD::WeakPlaneStressNOSPD(const InputParameters & parameters)
  : MechanicsBaseNOSPD(parameters)
{
}

void
WeakPlaneStressNOSPD::computeLocalResidual()
{
  _local_re(0) = _stress[0](2, 2) * _dg_vol_frac[0] * _node_vol[0] * _bond_status;
  _local_re(1) = _stress[1](2, 2) * _dg_vol_frac[1] * _node_vol[1] * _bond_status;
}

void
WeakPlaneStressNOSPD::computeLocalJacobian()
{
  for (unsigned int i = 0; i < _nnodes; ++i)
    for (unsigned int j = 0; j < _nnodes; ++j)
      _local_ke(i, j) += (i == j ? 1 : 0) * _Jacobian_mult[i](2, 2, 2, 2) * _dg_vol_frac[i] *
                         _node_vol[i] * _bond_status;
}

void
WeakPlaneStressNOSPD::computeLocalOffDiagJacobian(unsigned int jvar_num,
                                                  unsigned int coupled_component)
{
  _local_ke.zero();
  if (_temp_coupled && jvar_num == _temp_var->number()) // for coupled temperature variable
  {
    std::vector<RankTwoTensor> dSdT(_nnodes);
    for (unsigned int nd = 0; nd < _nnodes; nd++)
      for (unsigned int es = 0; es < _deigenstrain_dT.size(); es++)
        dSdT[nd] = -_Jacobian_mult[nd] * (*_deigenstrain_dT[es])[nd];

    for (unsigned int i = 0; i < _nnodes; ++i)
      for (unsigned int j = 0; j < _nnodes; ++j)
        _local_ke(i, j) +=
            (i == j ? 1 : 0) * dSdT[i](2, 2) * _dg_vol_frac[i] * _node_vol[i] * _bond_status;
  }
  else // for coupled displacement variables
  {
    // dSidUi and dSjdUj are considered as local off-diagonal Jacobian
    // contributions to their neighbors are considered as nonlocal off-diagonal
    for (unsigned int i = 0; i < _nnodes; ++i)
      for (unsigned int j = 0; j < _nnodes; ++j)
        _local_ke(i, j) += (i == j ? 1 : 0) * computeDSDU(coupled_component, j)(2, 2) *
                           _dg_vol_frac[j] * _node_vol[j] * _bond_status;
  }
}

void
WeakPlaneStressNOSPD::computePDNonlocalOffDiagJacobian(unsigned int jvar_num,
                                                       unsigned int coupled_component)
{
  if (_temp_coupled && jvar_num == _temp_var->number())
  {
    // no nonlocal contribution from temperature
  }
  else
  {
    for (unsigned int nd = 0; nd < _nnodes; nd++)
    {
      // calculation of jacobian contribution to current_node's neighbors
      // NOT including the contribution to nodes i and j, which is considered as local
      // off-diagonal
      std::vector<dof_id_type> jvardofs(_nnodes);
      jvardofs[0] = _current_elem->node_ptr(nd)->dof_number(_sys.number(), jvar_num, 0);
      std::vector<dof_id_type> neighbors = _pdmesh.getNeighbors(_current_elem->node_id(nd));
      std::vector<dof_id_type> bonds = _pdmesh.getBonds(_current_elem->node_id(nd));

      dof_id_type nb_index =
          std::find(neighbors.begin(), neighbors.end(), _current_elem->node_id(1 - nd)) -
          neighbors.begin();
      std::vector<dof_id_type> dg_neighbors =
          _pdmesh.getBondDeformationGradientNeighbors(_current_elem->node_id(nd), nb_index);

      Real vol_nb;
      RealGradient origin_vec_nb;
      RankTwoTensor dFdUk, dPdUk;

      for (unsigned int nb = 0; nb < dg_neighbors.size(); nb++)
        if (_bond_status_var->getElementalValue(_pdmesh.elemPtr(bonds[dg_neighbors[nb]])) > 0.5)
        {
          jvardofs[1] =
              _pdmesh.nodePtr(neighbors[dg_neighbors[nb]])->dof_number(_sys.number(), jvar_num, 0);
          vol_nb = _pdmesh.getNodeVolume(neighbors[dg_neighbors[nb]]);

          origin_vec_nb = *_pdmesh.nodePtr(neighbors[dg_neighbors[nb]]) -
                          *_pdmesh.nodePtr(_current_elem->node_id(nd));

          dFdUk.zero();
          for (unsigned int i = 0; i < _dim; ++i)
            dFdUk(coupled_component, i) =
                _horizon_radius[nd] / origin_vec_nb.norm() * origin_vec_nb(i) * vol_nb;

          dFdUk *= _shape2[nd].inverse();

          dPdUk = _Jacobian_mult[nd] * 0.5 *
                  (dFdUk.transpose() * _dgrad[nd] + _dgrad[nd].transpose() * dFdUk);

          _local_ke.zero();
          _local_ke(nd, 1) = dPdUk(2, 2) * _dg_vol_frac[nd] * _node_vol[nd] * _bond_status;

          _assembly.cacheJacobianBlock(_local_ke, _ivardofs, jvardofs, _var.scalingFactor());
        }
    }
  }
}
