//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "HorizonStabilizedFormIFiniteStrainMechanicsNOSPD.h"
#include "PeridynamicsMesh.h"

registerMooseObject("PeridynamicsApp", HorizonStabilizedFormIFiniteStrainMechanicsNOSPD);

InputParameters
HorizonStabilizedFormIFiniteStrainMechanicsNOSPD::validParams()
{
  InputParameters params = MechanicsFiniteStrainBaseNOSPD::validParams();
  params.addClassDescription(
      "Class for calculating the residual and the Jacobian for Form I "
      "of the horizon-stabilized peridynamic correspondence model under finite strain assumptions");

  params.addRequiredParam<unsigned int>(
      "component",
      "An integer corresponding to the variable this kernel acts on (0 for x, 1 for y, 2 for z)");

  return params;
}

HorizonStabilizedFormIFiniteStrainMechanicsNOSPD::HorizonStabilizedFormIFiniteStrainMechanicsNOSPD(
    const InputParameters & parameters)
  : MechanicsFiniteStrainBaseNOSPD(parameters), _component(getParam<unsigned int>("component"))
{
}

void
HorizonStabilizedFormIFiniteStrainMechanicsNOSPD::computeLocalResidual()
{
  // For finite strain formulation, the _stress tensor gotten from material class is the
  // Cauchy stress (Sigma). the first Piola-Kirchhoff stress (P) is then obtained as
  // P = J * Sigma * inv(F)^T.
  // Nodal force states are based on the first Piola-Kirchhoff stress tensors (P).
  // i.e., T = (J * Sigma * inv(F)^T) * inv(Shape) * xi * multi.
  // Cauchy stress is calculated as Sigma_n+1 = Sigma_n + R * (C * dt * D) * R^T

  for (unsigned int nd = 0; nd < _nnodes; ++nd)
    for (unsigned int i = 0; i < _nnodes; ++i)
      _local_re(i) += (i == 0 ? -1 : 1) * _multi[nd] *
                      ((_dgrad[nd].det() * _stress[nd] * _dgrad[nd].inverse().transpose()) *
                       _shape2[nd].inverse())
                          .row(_component) *
                      _origin_vec * _bond_status;
}

void
HorizonStabilizedFormIFiniteStrainMechanicsNOSPD::computeLocalJacobian()
{
  // excludes dTi/dUj and dTj/dUi contribution which was considered as nonlocal contribution
  std::vector<RankTwoTensor> dPxdUx(_nnodes);
  for (unsigned int nd = 0; nd < _nnodes; ++nd)
    dPxdUx[nd] = computeDJDU(_component, nd) * _stress[nd] * _dgrad[nd].inverse().transpose() +
                 _dgrad[nd].det() * computeDSDU(_component, nd) * _dgrad[nd].inverse().transpose() +
                 _dgrad[nd].det() * _stress[nd] * computeDinvFTDU(_component, nd);

  for (unsigned int i = 0; i < _nnodes; ++i)
    for (unsigned int j = 0; j < _nnodes; ++j)
      _local_ke(i, j) += (i == 0 ? -1 : 1) * _multi[j] *
                         (dPxdUx[j] * _shape2[j].inverse()).row(_component) * _origin_vec *
                         _bond_status;
}

void
HorizonStabilizedFormIFiniteStrainMechanicsNOSPD::computeNonlocalJacobian()
{
  // includes dTi/dUj and dTj/dUi contributions
  for (unsigned int nd = 0; nd < _nnodes; ++nd)
  {
    RankFourTensor dSdFhat = computeDSDFhat(nd);
    RankTwoTensor invF = _dgrad[nd].inverse();
    Real detF = _dgrad[nd].det();
    // calculation of jacobian contribution to current_node's neighbors
    std::vector<dof_id_type> ivardofs(_nnodes);
    ivardofs[0] = _current_elem->node_ptr(nd)->dof_number(_sys.number(), _var.number(), 0);
    std::vector<dof_id_type> neighbors = _pdmesh.getNeighbors(_current_elem->node_id(nd));
    std::vector<dof_id_type> bonds = _pdmesh.getBonds(_current_elem->node_id(nd));

    dof_id_type nb_index =
        std::find(neighbors.begin(), neighbors.end(), _current_elem->node_id(1 - nd)) -
        neighbors.begin();
    std::vector<dof_id_type> dg_neighbors =
        _pdmesh.getBondDeformationGradientNeighbors(_current_elem->node_id(nd), nb_index);

    Real vol_nb, dJdU;
    RealGradient origin_vec_nb;
    RankTwoTensor dFdUk, dPxdUkx, dSdU, dinvFTdU;

    for (unsigned int nb = 0; nb < dg_neighbors.size(); ++nb)
      if (_bond_status_var->getElementalValue(_pdmesh.elemPtr(bonds[dg_neighbors[nb]])) > 0.5)
      {
        ivardofs[1] = _pdmesh.nodePtr(neighbors[dg_neighbors[nb]])
                          ->dof_number(_sys.number(), _var.number(), 0);
        vol_nb = _pdmesh.getNodeVolume(neighbors[dg_neighbors[nb]]);

        origin_vec_nb = *_pdmesh.nodePtr(neighbors[dg_neighbors[nb]]) -
                        *_pdmesh.nodePtr(_current_elem->node_id(nd));

        dFdUk.zero();
        for (unsigned int i = 0; i < _dim; ++i)
          dFdUk(_component, i) =
              _horizon_radius[nd] / origin_vec_nb.norm() * origin_vec_nb(i) * vol_nb;

        dFdUk *= _shape2[nd].inverse();

        // calculate dJ/du
        dJdU = 0.0;
        for (unsigned int i = 0; i < 3; ++i)
          for (unsigned int j = 0; j < 3; ++j)
            dJdU += detF * invF(j, i) * dFdUk(i, j);

        // calculate dS/du
        dSdU = dSdFhat * dFdUk * _dgrad_old[nd].inverse();

        // calculate dinv(F)Tdu
        dinvFTdU.zero();
        for (unsigned int i = 0; i < 3; ++i)
          for (unsigned int J = 0; J < 3; ++J)
            for (unsigned int k = 0; k < 3; ++k)
              for (unsigned int L = 0; L < 3; ++L)
                dinvFTdU(i, J) += -invF(J, k) * invF(L, i) * dFdUk(k, L);

        // calculate the derivative of first Piola-Kirchhoff stress w.r.t displacements
        dPxdUkx = dJdU * _stress[nd] * invF.transpose() + detF * dSdU * invF.transpose() +
                  detF * _stress[nd] * dinvFTdU;

        for (unsigned int i = 0; i < _nnodes; ++i)
          for (unsigned int j = 0; j < _nnodes; ++j)
            _local_ke(i, j) = (i == 0 ? -1 : 1) * (j == 0 ? 0 : 1) * _multi[nd] *
                              (dPxdUkx * _shape2[nd].inverse()).row(_component) * _origin_vec *
                              _bond_status;

        _assembly.cacheJacobianBlock(_local_ke, _ivardofs, ivardofs, _var.scalingFactor());

        if (_has_diag_save_in)
        {
          unsigned int rows = _nnodes;
          DenseVector<Real> diag(rows);
          for (unsigned int i = 0; i < rows; ++i)
            diag(i) = _local_ke(i, i);

          Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
          for (unsigned int i = 0; i < _diag_save_in.size(); ++i)
          {
            std::vector<dof_id_type> diag_save_in_dofs(2);
            diag_save_in_dofs[0] = _current_elem->node_ptr(nd)->dof_number(
                _diag_save_in[i]->sys().number(), _diag_save_in[i]->number(), 0);
            diag_save_in_dofs[1] =
                _pdmesh.nodePtr(neighbors[dg_neighbors[nb]])
                    ->dof_number(_diag_save_in[i]->sys().number(), _diag_save_in[i]->number(), 0);

            _diag_save_in[i]->sys().solution().add_vector(diag, diag_save_in_dofs);
          }
        }
      }
  }
}

void
HorizonStabilizedFormIFiniteStrainMechanicsNOSPD::computeLocalOffDiagJacobian(
    unsigned int jvar_num, unsigned int coupled_component)
{
  _local_ke.zero();
  if (_temp_coupled && jvar_num == _temp_var->number())
  {
    std::vector<RankTwoTensor> dSdT(_nnodes);
    for (unsigned int nd = 0; nd < _nnodes; ++nd)
      for (unsigned int es = 0; es < _deigenstrain_dT.size(); ++es)
        dSdT[nd] = -_dgrad[nd].det() * _Jacobian_mult[nd] * (*_deigenstrain_dT[es])[nd] *
                   _dgrad[nd].inverse().transpose();

    for (unsigned int i = 0; i < _nnodes; ++i)
      for (unsigned int j = 0; j < _nnodes; ++j)
        _local_ke(i, j) += (i == 0 ? -1 : 1) * _multi[j] *
                           (dSdT[j] * _shape2[j].inverse()).row(_component) * _origin_vec *
                           _bond_status;
  }
  else if (_out_of_plane_strain_coupled &&
           jvar_num == _out_of_plane_strain_var
                           ->number()) // weak plane stress case, out_of_plane_strain is coupled
  {
    std::vector<RankTwoTensor> dSdE33(_nnodes);
    for (unsigned int nd = 0; nd < _nnodes; ++nd)
    {
      for (unsigned int i = 0; i < 3; ++i)
        for (unsigned int j = 0; j < 3; ++j)
          dSdE33[nd](i, j) = _Jacobian_mult[nd](i, j, 2, 2);

      dSdE33[nd] = _dgrad[nd].det() * dSdE33[nd] * _dgrad[nd].inverse().transpose();
    }

    for (unsigned int i = 0; i < _nnodes; ++i)
      for (unsigned int j = 0; j < _nnodes; ++j)
        _local_ke(i, j) += (i == 0 ? -1 : 1) * _multi[j] *
                           (dSdE33[j] * _shape2[j].inverse()).row(_component) * _origin_vec *
                           _bond_status;
  }
  else
  {
    std::vector<RankTwoTensor> dPxdUy(_nnodes);
    for (unsigned int nd = 0; nd < _nnodes; ++nd)
      dPxdUy[nd] =
          computeDJDU(coupled_component, nd) * _stress[nd] * _dgrad[nd].inverse().transpose() +
          _dgrad[nd].det() * computeDSDU(coupled_component, nd) * _dgrad[nd].inverse().transpose() +
          _dgrad[nd].det() * _stress[nd] * computeDinvFTDU(coupled_component, nd);

    for (unsigned int i = 0; i < _nnodes; ++i)
      for (unsigned int j = 0; j < _nnodes; ++j)
        _local_ke(i, j) += (i == 0 ? -1 : 1) * _multi[j] *
                           (dPxdUy[j] * _shape2[j].inverse()).row(_component) * _origin_vec *
                           _bond_status;
  }
}

void
HorizonStabilizedFormIFiniteStrainMechanicsNOSPD::computePDNonlocalOffDiagJacobian(
    unsigned int jvar_num, unsigned int coupled_component)
{
  if (_temp_coupled && jvar_num == _temp_var->number())
  {
    // no nonlocal contribution from temperature
  }
  else if (_out_of_plane_strain_coupled && jvar_num == _out_of_plane_strain_var->number())
  {
    // no nonlocal contribution from out of plane strain
  }
  else
  {
    for (unsigned int nd = 0; nd < _nnodes; ++nd)
    {
      RankFourTensor dSdFhat = computeDSDFhat(nd);
      RankTwoTensor invF = _dgrad[nd].inverse();
      Real detF = _dgrad[nd].det();
      // calculation of jacobian contribution to current_node's neighbors
      std::vector<dof_id_type> jvardofs(_nnodes);
      jvardofs[0] = _current_elem->node_ptr(nd)->dof_number(_sys.number(), jvar_num, 0);
      std::vector<dof_id_type> neighbors = _pdmesh.getNeighbors(_current_elem->node_id(nd));
      std::vector<dof_id_type> bonds = _pdmesh.getBonds(_current_elem->node_id(nd));

      dof_id_type nb_index =
          std::find(neighbors.begin(), neighbors.end(), _current_elem->node_id(1 - nd)) -
          neighbors.begin();
      std::vector<dof_id_type> dg_neighbors =
          _pdmesh.getBondDeformationGradientNeighbors(_current_elem->node_id(nd), nb_index);

      Real vol_nb, dJdU;
      RealGradient origin_vec_nb;
      RankTwoTensor dFdUk, dPxdUky, dSdU, dinvFTdU;

      for (unsigned int nb = 0; nb < dg_neighbors.size(); ++nb)
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

          // calculate dJ/du
          dJdU = 0.0;
          for (unsigned int i = 0; i < 3; ++i)
            for (unsigned int j = 0; j < 3; ++j)
              dJdU += detF * invF(j, i) * dFdUk(i, j);

          // calculate dS/du
          dSdU = dSdFhat * dFdUk * _dgrad_old[nd].inverse();

          // calculate dinv(F)Tdu
          dinvFTdU.zero();
          for (unsigned int i = 0; i < 3; ++i)
            for (unsigned int J = 0; J < 3; ++J)
              for (unsigned int k = 0; k < 3; ++k)
                for (unsigned int L = 0; L < 3; ++L)
                  dinvFTdU(i, J) += -invF(J, k) * invF(L, i) * dFdUk(k, L);

          // calculate the derivative of first Piola-Kirchhoff stress w.r.t displacements
          dPxdUky = dJdU * _stress[nd] * invF.transpose() + detF * dSdU * invF.transpose() +
                    detF * _stress[nd] * dinvFTdU;

          for (unsigned int i = 0; i < _nnodes; ++i)
            for (unsigned int j = 0; j < _nnodes; ++j)
              _local_ke(i, j) = (i == 0 ? -1 : 1) * (j == 0 ? 0 : 1) * _multi[nd] *
                                (dPxdUky * _shape2[nd].inverse()).row(_component) * _origin_vec *
                                _bond_status;

          _assembly.cacheJacobianBlock(_local_ke, _ivardofs, jvardofs, _var.scalingFactor());
        }
    }
  }
}
