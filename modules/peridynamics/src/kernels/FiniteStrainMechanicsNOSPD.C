//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FiniteStrainMechanicsNOSPD.h"
#include "PeridynamicsMesh.h"
#include "RankTwoTensor.h"
#include "RankFourTensor.h"

registerMooseObject("PeridynamicsApp", FiniteStrainMechanicsNOSPD);

template <>
InputParameters
validParams<FiniteStrainMechanicsNOSPD>()
{
  InputParameters params = validParams<MechanicsBaseNOSPD>();
  params.addClassDescription(
      "Class for calculating residual and Jacobian for the Self-stabilized "
      "Non-Ordinary State-based PeriDynamic (SNOSPD) formulation under finite "
      "strain assumptions");

  params.addRequiredParam<unsigned int>(
      "component",
      "An integer corresponding to the variable this kernel acts on (0 for x, 1 for y, 2 for z)");

  return params;
}

FiniteStrainMechanicsNOSPD::FiniteStrainMechanicsNOSPD(const InputParameters & parameters)
  : MechanicsBaseNOSPD(parameters),
    _dgrad_old(getMaterialPropertyOld<RankTwoTensor>("deformation_gradient")),
    _E_inc(getMaterialProperty<RankTwoTensor>("strain_increment")),
    _R_inc(getMaterialProperty<RankTwoTensor>("rotation_increment")),
    _component(getParam<unsigned int>("component"))
{
}

void
FiniteStrainMechanicsNOSPD::computeLocalResidual()
{
  // For finite strain formulation, the _stress tensor gotten from material class is the
  // Cauchy stress (Sigma). the first Piola-Kirchhoff stress (P) is then obtained as
  // P = J * Sigma * inv(F)^T.
  // Nodal force states are based on the first Piola-Kirchhoff stress tensors (P).
  // i.e., T = (J * Sigma * inv(F)^T) * inv(Shape) * xi * multi.
  // Cauchy stress is calculated as Sigma_n+1 = Sigma_n + R * (C * dt * D) * R^T

  std::vector<RankTwoTensor> nodal_force(_nnodes);
  for (unsigned int nd = 0; nd < _nnodes; ++nd)
    nodal_force[nd] = (_dgrad[nd].det() * _stress[nd] * _dgrad[nd].inverse().transpose()) *
                      _shape[nd].inverse() * _multi[nd];

  _local_re(0) = -(nodal_force[0].row(_component) + nodal_force[1].row(_component)) *
                 _origin_vec_ij * _bond_status_ij;
  _local_re(1) = -_local_re(0);
}

void
FiniteStrainMechanicsNOSPD::computeLocalJacobian()
{
  // excludes dTi/dUj and dTj/dUi contribution which was considered as nonlocal contribution
  std::vector<RankTwoTensor> dPxdUx(_nnodes);
  for (unsigned int nd = 0; nd < _nnodes; ++nd)
    dPxdUx[nd] = computeDJDU(_component, nd) * _stress[nd] * _dgrad[nd].inverse().transpose() +
                 _dgrad[nd].det() * computeDSDU(_component, nd) * _dgrad[nd].inverse().transpose() +
                 _dgrad[nd].det() * _stress[nd] * computeDinvFTDU(_component, nd);

  for (_i = 0; _i < _test.size(); ++_i)
    for (_j = 0; _j < _phi.size(); ++_j)
      _local_ke(_i, _j) += (_i == 0 ? -1 : 1) * _multi[_j] *
                           (dPxdUx[_j] * _shape[_j].inverse()).row(_component) * _origin_vec_ij *
                           _bond_status_ij;
}

void
FiniteStrainMechanicsNOSPD::computeNonlocalJacobian()
{
  // includes dTi/dUj and dTj/dUi contributions
  for (unsigned int cur_nd = 0; cur_nd < _nnodes; ++cur_nd)
  {
    RankFourTensor dSdFhat = computeDSDFhat(cur_nd);
    RankTwoTensor invF = _dgrad[cur_nd].inverse();
    Real detF = _dgrad[cur_nd].det();
    // calculation of jacobian contribution to current_node's neighbors
    std::vector<dof_id_type> dof(_nnodes);
    dof[0] = _current_elem->node_ptr(cur_nd)->dof_number(_sys.number(), _var.number(), 0);
    std::vector<dof_id_type> neighbors = _pdmesh.getNeighbors(_current_elem->node_id(cur_nd));
    unsigned int nb =
        std::find(neighbors.begin(), neighbors.end(), _current_elem->node_id(1 - cur_nd)) -
        neighbors.begin();
    std::vector<unsigned int> dg_neighbors =
        _pdmesh.getDefGradNeighbors(_current_elem->node_id(cur_nd), nb);
    std::vector<dof_id_type> bonds = _pdmesh.getBonds(_current_elem->node_id(cur_nd));
    for (unsigned int k = 0; k < dg_neighbors.size(); ++k)
    {
      Node * node_k = _pdmesh.nodePtr(neighbors[dg_neighbors[k]]);
      dof[1] = node_k->dof_number(_sys.number(), _var.number(), 0);
      Real vol_k = _pdmesh.getPDNodeVolume(neighbors[dg_neighbors[k]]);

      // obtain bond ik's origin vector
      RealGradient origin_vec_ijk = *node_k - *_pdmesh.nodePtr(_current_elem->node_id(cur_nd));

      RankTwoTensor dFdUk;
      dFdUk.zero();
      for (unsigned int j = 0; j < _dim; ++j)
        dFdUk(_component, j) =
            _horiz_rad[cur_nd] / origin_vec_ijk.norm() * origin_vec_ijk(j) * vol_k;

      dFdUk *= _shape[cur_nd].inverse();

      RankTwoTensor dPxdUkx;
      // calculate dJ/du
      Real dJdU = 0.0;
      for (unsigned int i = 0; i < 3; ++i)
        for (unsigned int J = 0; J < 3; ++J)
          dJdU += detF * invF(J, i) * dFdUk(i, J);

      // calculate dS/du
      RankTwoTensor dSdU = dSdFhat * dFdUk * _dgrad_old[cur_nd].inverse();

      // calculate dinv(F)Tdu
      RankTwoTensor dinvFTdU;
      dinvFTdU.zero();
      for (unsigned int i = 0; i < 3; ++i)
        for (unsigned int J = 0; J < 3; ++J)
          for (unsigned int k = 0; k < 3; ++k)
            for (unsigned int L = 0; L < 3; ++L)
              dinvFTdU(i, J) += -invF(J, k) * invF(L, i) * dFdUk(k, L);

      // calculate the derivative of first Piola-Kirchhoff stress w.r.t displacements
      dPxdUkx = dJdU * _stress[cur_nd] * invF.transpose() + detF * dSdU * invF.transpose() +
                detF * _stress[cur_nd] * dinvFTdU;

      // bond status for bond k
      Real bond_status_ijk =
          _bond_status_var.getElementalValue(_pdmesh.elemPtr(bonds[dg_neighbors[k]]));

      _local_ke.resize(_test.size(), _phi.size());
      _local_ke.zero();
      for (_i = 0; _i < _test.size(); ++_i)
        for (_j = 0; _j < _phi.size(); ++_j)
          _local_ke(_i, _j) = (_i == 0 ? -1 : 1) * (_j == 0 ? 0 : 1) * _multi[cur_nd] *
                              (dPxdUkx * _shape[cur_nd].inverse()).row(_component) *
                              _origin_vec_ij * _bond_status_ij * bond_status_ijk;

      _assembly.cacheJacobianBlock(_local_ke, _ivardofs_ij, dof, _var.scalingFactor());

      if (_has_diag_save_in)
      {
        unsigned int rows = _test.size();
        DenseVector<Real> diag(rows);
        for (unsigned int i = 0; i < rows; ++i)
          diag(i) = _local_ke(i, i);

        Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
        for (unsigned int i = 0; i < _diag_save_in.size(); ++i)
          _diag_save_in[i]->sys().solution().add_vector(diag, _diag_save_in[i]->dofIndices());
      }
    }
  }
}

void
FiniteStrainMechanicsNOSPD::computeLocalOffDiagJacobian(unsigned int coupled_component)
{
  _local_ke.zero();
  if (coupled_component == 3)
  {
    std::vector<RankTwoTensor> dSdT(_nnodes);
    for (unsigned int nd = 0; nd < _nnodes; ++nd)
      for (unsigned int es = 0; es < _deigenstrain_dT.size(); ++es)
        dSdT[nd] = -_dgrad[nd].det() * _Jacobian_mult[nd] * (*_deigenstrain_dT[es])[nd] *
                   _dgrad[nd].inverse().transpose();

    for (_i = 0; _i < _test.size(); ++_i)
      for (_j = 0; _j < _phi.size(); ++_j)
        _local_ke(_i, _j) += (_i == 0 ? -1 : 1) * _multi[_j] *
                             (dSdT[_j] * _shape[_j].inverse()).row(_component) * _origin_vec_ij *
                             _bond_status_ij;
  }
  else
  {
    std::vector<RankTwoTensor> dPxdUy(_nnodes);
    for (unsigned int nd = 0; nd < _nnodes; ++nd)
      dPxdUy[nd] =
          computeDJDU(coupled_component, nd) * _stress[nd] * _dgrad[nd].inverse().transpose() +
          _dgrad[nd].det() * computeDSDU(coupled_component, nd) * _dgrad[nd].inverse().transpose() +
          _dgrad[nd].det() * _stress[nd] * computeDinvFTDU(coupled_component, nd);

    for (_i = 0; _i < _test.size(); ++_i)
      for (_j = 0; _j < _phi.size(); ++_j)
        _local_ke(_i, _j) += (_i == 0 ? -1 : 1) * _multi[_j] *
                             (dPxdUy[_j] * _shape[_j].inverse()).row(_component) * _origin_vec_ij *
                             _bond_status_ij;
  }
}

void
FiniteStrainMechanicsNOSPD::computePDNonlocalOffDiagJacobian(unsigned int jvar_num,
                                                             unsigned int coupled_component)
{
  if (coupled_component != 3 && coupled_component != 4)
  {
    for (unsigned int cur_nd = 0; cur_nd < _nnodes; ++cur_nd)
    {
      RankFourTensor dSdFhat = computeDSDFhat(cur_nd);
      RankTwoTensor invF = _dgrad[cur_nd].inverse();
      Real detF = _dgrad[cur_nd].det();
      // calculation of jacobian contribution to current_node's neighbors
      std::vector<dof_id_type> jvardofs_ijk(_nnodes);
      jvardofs_ijk[0] = _current_elem->node_ptr(cur_nd)->dof_number(_sys.number(), jvar_num, 0);
      std::vector<dof_id_type> neighbors = _pdmesh.getNeighbors(_current_elem->node_id(cur_nd));
      unsigned int nb =
          std::find(neighbors.begin(), neighbors.end(), _current_elem->node_id(1 - cur_nd)) -
          neighbors.begin();
      std::vector<unsigned int> dg_neighbors =
          _pdmesh.getDefGradNeighbors(_current_elem->node_id(cur_nd), nb);
      std::vector<dof_id_type> bonds = _pdmesh.getBonds(_current_elem->node_id(cur_nd));
      for (unsigned int k = 0; k < dg_neighbors.size(); ++k)
      {
        Node * node_k = _pdmesh.nodePtr(neighbors[dg_neighbors[k]]);
        jvardofs_ijk[1] = node_k->dof_number(_sys.number(), jvar_num, 0);
        Real vol_k = _pdmesh.getPDNodeVolume(neighbors[dg_neighbors[k]]);

        // obtain bond k's origin vector
        RealGradient origin_vec_ijk = *node_k - *_pdmesh.nodePtr(_current_elem->node_id(cur_nd));

        RankTwoTensor dFdUk;
        dFdUk.zero();
        for (unsigned int j = 0; j < _dim; ++j)
          dFdUk(coupled_component, j) =
              _horiz_rad[cur_nd] / origin_vec_ijk.norm() * origin_vec_ijk(j) * vol_k;

        dFdUk *= _shape[cur_nd].inverse();

        RankTwoTensor dPxdUky;
        // calculate dJ/du
        Real dJdU = 0.0;
        for (unsigned int i = 0; i < 3; ++i)
          for (unsigned int J = 0; J < 3; ++J)
            dJdU += detF * invF(J, i) * dFdUk(i, J);

        // calculate dS/du
        RankTwoTensor dSdU = dSdFhat * dFdUk * _dgrad_old[cur_nd].inverse();

        // calculate dinv(F)Tdu
        RankTwoTensor dinvFTdU;
        dinvFTdU.zero();
        for (unsigned int i = 0; i < 3; ++i)
          for (unsigned int J = 0; J < 3; ++J)
            for (unsigned int k = 0; k < 3; ++k)
              for (unsigned int L = 0; L < 3; ++L)
                dinvFTdU(i, J) += -invF(J, k) * invF(L, i) * dFdUk(k, L);

        // calculate the derivative of first Piola-Kirchhoff stress w.r.t displacements
        dPxdUky = dJdU * _stress[cur_nd] * invF.transpose() + detF * dSdU * invF.transpose() +
                  detF * _stress[cur_nd] * dinvFTdU;

        // bond status for bond k
        Real bond_status_ijk =
            _bond_status_var.getElementalValue(_pdmesh.elemPtr(bonds[dg_neighbors[k]]));

        _local_ke.zero();
        for (_i = 0; _i < _test.size(); ++_i)
          for (_j = 0; _j < _phi.size(); ++_j)
            _local_ke(_i, _j) = (_i == 0 ? -1 : 1) * (_j == 0 ? 0 : 1) * _multi[cur_nd] *
                                (dPxdUky * _shape[cur_nd].inverse()).row(_component) *
                                _origin_vec_ij * _bond_status_ij * bond_status_ijk;

        _assembly.cacheJacobianBlock(_local_ke, _ivardofs_ij, jvardofs_ijk, _var.scalingFactor());
      }
    }
  }
}

RankTwoTensor
FiniteStrainMechanicsNOSPD::computeDSDU(unsigned int component, unsigned int nd)
{
  // compute the derivative of stress w.r.t the solution components for finite strain
  RankTwoTensor dSdU;

  // fetch the derivative of stress w.r.t the Fhat
  RankFourTensor DSDFhat = computeDSDFhat(nd);

  // third calculate derivative of Fhat w.r.t solution components
  RankTwoTensor Tp3;
  if (component == 0)
    Tp3 = _dgrad_old[nd].inverse() * _ddgraddu[nd];
  else if (component == 1)
    Tp3 = _dgrad_old[nd].inverse() * _ddgraddv[nd];
  else if (component == 2)
    Tp3 = _dgrad_old[nd].inverse() * _ddgraddw[nd];

  // assemble the fetched and calculated quantities to form the derivative of Cauchy stress w.r.t
  // solution components
  dSdU = DSDFhat * Tp3;

  return dSdU;
}

RankFourTensor
FiniteStrainMechanicsNOSPD::computeDSDFhat(unsigned int nd)
{
  // compute the derivative of stress w.r.t the Fhat for finite strain
  RankTwoTensor I(RankTwoTensor::initIdentity);
  RankFourTensor dSdFhat;
  dSdFhat.zero();

  // first calculate the derivative of incremental Cauchy stress w.r.t the inverse of Fhat
  // Reference: M. M. Rashid (1993), Incremental Kinematics for finite element applications, IJNME
  RankTwoTensor S_inc = _Jacobian_mult[nd] * _E_inc[nd];
  RankFourTensor Tp1;
  Tp1.zero();
  for (unsigned int i = 0; i < 3; ++i)
    for (unsigned int j = 0; j < 3; ++j)
      for (unsigned int k = 0; k < 3; ++k)
        for (unsigned int l = 0; l < 3; ++l)
          for (unsigned int m = 0; m < 3; ++m)
            for (unsigned int n = 0; n < 3; ++n)
              for (unsigned int r = 0; r < 3; ++r)
                Tp1(i, j, k, l) +=
                    S_inc(m, n) *
                        (_R_inc[nd](j, n) * (0.5 * I(k, m) * I(i, l) - I(m, l) * _R_inc[nd](i, k) +
                                             0.5 * _R_inc[nd](i, k) * _R_inc[nd](m, l)) +
                         _R_inc[nd](i, m) * (0.5 * I(k, n) * I(j, l) - I(n, l) * _R_inc[nd](j, k) +
                                             0.5 * _R_inc[nd](j, k) * _R_inc[nd](n, l))) -
                    _R_inc[nd](l, m) * _R_inc[nd](i, n) * _R_inc[nd](j, r) *
                        _Jacobian_mult[nd](n, r, m, k);

  // second calculate derivative of inverse of Fhat w.r.t Fhat
  // d(inv(Fhat)_kl)/dFhat_mn = - inv(Fhat)_km * inv(Fhat)_nl
  // the bases are gk, gl, gm, gn, indictates the inverse rather than the inverse transpose

  RankFourTensor Tp2;
  Tp2.zero();
  RankTwoTensor invFhat = (_dgrad[nd] * _dgrad_old[nd].inverse()).inverse();
  for (unsigned int k = 0; k < 3; ++k)
    for (unsigned int l = 0; l < 3; ++l)
      for (unsigned int m = 0; m < 3; ++m)
        for (unsigned int n = 0; n < 3; ++n)
          Tp2(k, l, m, n) += -invFhat(k, m) * invFhat(n, l);

  // assemble two calculated quantities to form the derivative of Cauchy stress w.r.t
  // Fhat
  dSdFhat = Tp1 * Tp2;

  return dSdFhat;
}

Real
FiniteStrainMechanicsNOSPD::computeDJDU(unsigned int component, unsigned int nd)
{
  // for finite formulation, compute the derivative of determinant of deformation gradient w.r.t the
  // solution components
  // dJ / du = dJ / dF_iJ * dF_iJ / du = J * inv(F)_Ji * dF_iJ / du

  Real dJdU = 0.0;
  RankTwoTensor invF = _dgrad[nd].inverse();
  Real detF = _dgrad[nd].det();
  for (unsigned int i = 0; i < 3; ++i)
    for (unsigned int J = 0; J < 3; ++J)
    {
      if (component == 0)
        dJdU += detF * invF(J, i) * _ddgraddu[nd](i, J);
      else if (component == 1)
        dJdU += detF * invF(J, i) * _ddgraddv[nd](i, J);
      else if (component == 2)
        dJdU += detF * invF(J, i) * _ddgraddw[nd](i, J);
    }

  return dJdU;
}

RankTwoTensor
FiniteStrainMechanicsNOSPD::computeDinvFTDU(unsigned int component, unsigned int nd)
{
  // for finite formulation, compute the derivative of transpose of inverse of deformation gradient
  // w.r.t the solution components
  // d(inv(F)_Ji)/du = d(inv(F)_Ji)/dF_kL * dF_kL/du = - inv(F)_Jk * inv(F)_Li * dF_kL/du
  // the bases are gi, GJ, gk, GL, indictates the inverse transpose rather than the inverse

  // RankTwoTensor dinvFTdU;
  // dinvFTdU.zero();
  // RankTwoTensor invF = _dgrad[nd].inverse();
  // for (unsigned int i = 0; i < 3; ++i)
  //   for (unsigned int J = 0; J < 3; ++J)
  //     for (unsigned int k = 0; k < 3; ++k)
  //       for (unsigned int L = 0; L < 3; ++L)
  //       {
  //         if (component == 0)
  //           dinvFTdU(i, J) += -invF(J, k) * invF(L, i) * _ddgraddu[nd](k, L);
  //         else if (component == 1)
  //           dinvFTdU(i, J) += -invF(J, k) * invF(L, i) * _ddgraddv[nd](k, L);
  //         else if (component == 2)
  //           dinvFTdU(i, J) += -invF(J, k) * invF(L, i) * _ddgraddw[nd](k, L);
  //       }

  RankTwoTensor dinvFTdU;
  dinvFTdU.zero();
  RankTwoTensor invF = _dgrad[nd].inverse();
  if (component == 0)
  {
    dinvFTdU(0, 1) =
        _ddgraddu[nd](0, 2) * _dgrad[nd](2, 1) - _ddgraddu[nd](0, 1) * _dgrad[nd](2, 2);
    dinvFTdU(0, 2) =
        _ddgraddu[nd](0, 1) * _dgrad[nd](1, 2) - _ddgraddu[nd](0, 2) * _dgrad[nd](1, 1);
    dinvFTdU(1, 1) =
        _ddgraddu[nd](0, 0) * _dgrad[nd](2, 2) - _ddgraddu[nd](0, 2) * _dgrad[nd](2, 0);
    dinvFTdU(1, 2) =
        _ddgraddu[nd](0, 2) * _dgrad[nd](1, 0) - _ddgraddu[nd](0, 0) * _dgrad[nd](1, 2);
    dinvFTdU(2, 1) =
        _ddgraddu[nd](0, 1) * _dgrad[nd](2, 0) - _ddgraddu[nd](0, 0) * _dgrad[nd](2, 1);
    dinvFTdU(2, 2) =
        _ddgraddu[nd](0, 0) * _dgrad[nd](1, 1) - _ddgraddu[nd](0, 1) * _dgrad[nd](1, 0);
  }
  else if (component == 1)
  {
    dinvFTdU(0, 0) =
        _ddgraddv[nd](1, 1) * _dgrad[nd](2, 2) - _ddgraddv[nd](1, 2) * _dgrad[nd](2, 1);
    dinvFTdU(0, 2) =
        _ddgraddv[nd](1, 2) * _dgrad[nd](0, 1) - _ddgraddv[nd](0, 2) * _dgrad[nd](1, 1);
    dinvFTdU(1, 0) =
        _ddgraddv[nd](1, 2) * _dgrad[nd](2, 0) - _ddgraddv[nd](1, 0) * _dgrad[nd](2, 2);
    dinvFTdU(1, 2) =
        _ddgraddv[nd](1, 0) * _dgrad[nd](0, 2) - _ddgraddv[nd](1, 2) * _dgrad[nd](0, 0);
    dinvFTdU(2, 0) =
        _ddgraddv[nd](1, 0) * _dgrad[nd](2, 1) - _ddgraddv[nd](1, 1) * _dgrad[nd](2, 0);
    dinvFTdU(2, 2) =
        _ddgraddv[nd](1, 1) * _dgrad[nd](0, 0) - _ddgraddv[nd](1, 0) * _dgrad[nd](0, 1);
  }
  else if (component == 2)
  {
    dinvFTdU(0, 0) =
        _ddgraddw[nd](2, 2) * _dgrad[nd](1, 1) - _ddgraddw[nd](2, 1) * _dgrad[nd](1, 2);
    dinvFTdU(0, 1) =
        _ddgraddw[nd](2, 1) * _dgrad[nd](0, 2) - _ddgraddw[nd](2, 2) * _dgrad[nd](0, 1);
    dinvFTdU(1, 0) =
        _ddgraddw[nd](2, 0) * _dgrad[nd](1, 2) - _ddgraddw[nd](2, 2) * _dgrad[nd](1, 0);
    dinvFTdU(1, 1) =
        _ddgraddw[nd](2, 2) * _dgrad[nd](0, 0) - _ddgraddw[nd](2, 0) * _dgrad[nd](0, 2);
    dinvFTdU(2, 0) =
        _ddgraddw[nd](2, 1) * _dgrad[nd](1, 0) - _ddgraddw[nd](2, 0) * _dgrad[nd](1, 1);
    dinvFTdU(2, 1) =
        _ddgraddw[nd](2, 0) * _dgrad[nd](0, 1) - _ddgraddw[nd](2, 1) * _dgrad[nd](0, 0);
  }
  dinvFTdU /= _dgrad[nd].det();
  for (unsigned int i = 0; i < 3; ++i)
    for (unsigned int J = 0; J < 3; ++J)
      for (unsigned int k = 0; k < 3; ++k)
        for (unsigned int L = 0; L < 3; ++L)
        {
          if (component == 0)
            dinvFTdU(i, J) -= invF(i, J) * invF(L, k) * _ddgraddu[nd](k, L);
          else if (component == 1)
            dinvFTdU(i, J) -= invF(i, J) * invF(L, k) * _ddgraddv[nd](k, L);
          else if (component == 2)
            dinvFTdU(i, J) -= invF(i, J) * invF(L, k) * _ddgraddw[nd](k, L);
        }

  return dinvFTdU;
}
