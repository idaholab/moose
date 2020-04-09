//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "HorizonStabilizedFiniteStrainMechanicsNOSPD.h"
#include "PeridynamicsMesh.h"

registerMooseObject("PeridynamicsApp", HorizonStabilizedFiniteStrainMechanicsNOSPD);

InputParameters
HorizonStabilizedFiniteStrainMechanicsNOSPD::validParams()
{
  InputParameters params = MechanicsBaseNOSPD::validParams();
  params.addClassDescription(
      "Class for calculating the residual and Jacobian for the horizon-stabilized "
      "non-ordinary state-based peridynamic correspondence model under finite "
      "strain assumptions");

  params.addRequiredParam<unsigned int>(
      "component",
      "An integer corresponding to the variable this kernel acts on (0 for x, 1 for y, 2 for z)");

  return params;
}

HorizonStabilizedFiniteStrainMechanicsNOSPD::HorizonStabilizedFiniteStrainMechanicsNOSPD(
    const InputParameters & parameters)
  : MechanicsBaseNOSPD(parameters),
    _dgrad_old(getMaterialPropertyOld<RankTwoTensor>("deformation_gradient")),
    _E_inc(getMaterialProperty<RankTwoTensor>("strain_increment")),
    _R_inc(getMaterialProperty<RankTwoTensor>("rotation_increment")),
    _component(getParam<unsigned int>("component"))
{
}

void
HorizonStabilizedFiniteStrainMechanicsNOSPD::computeLocalResidual()
{
  // For finite strain formulation, the _stress tensor gotten from material class is the
  // Cauchy stress (Sigma). the first Piola-Kirchhoff stress (P) is then obtained as
  // P = J * Sigma * inv(F)^T.
  // Nodal force states are based on the first Piola-Kirchhoff stress tensors (P).
  // i.e., T = (J * Sigma * inv(F)^T) * inv(Shape) * xi * multi.
  // Cauchy stress is calculated as Sigma_n+1 = Sigma_n + R * (C * dt * D) * R^T

  std::vector<Real> nodal_force_comp(_nnodes);
  for (unsigned int nd = 0; nd < _nnodes; ++nd)
    nodal_force_comp[nd] = _multi[nd] *
                           ((_dgrad[nd].det() * _stress[nd] * _dgrad[nd].inverse().transpose()) *
                            _shape2[nd].inverse())
                               .row(_component) *
                           (nd == 0 ? 1 : -1) * _origin_vec;

  _local_re(0) = -(nodal_force_comp[0] - nodal_force_comp[1]) * _bond_status;
  _local_re(1) = -_local_re(0);
}

void
HorizonStabilizedFiniteStrainMechanicsNOSPD::computeLocalJacobian()
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
                           (dPxdUx[_j] * _shape2[_j].inverse()).row(_component) * _origin_vec *
                           _bond_status;
}

void
HorizonStabilizedFiniteStrainMechanicsNOSPD::computeNonlocalJacobian()
{
  // includes dTi/dUj and dTj/dUi contributions
  for (unsigned int nd = 0; nd < _nnodes; ++nd)
  {
    RankFourTensor dSdFhat = computeDSDFhat(nd);
    RankTwoTensor invF = _dgrad[nd].inverse();
    Real detF = _dgrad[nd].det();
    // calculation of jacobian contribution to current_node's neighbors
    std::vector<dof_id_type> dof(_nnodes);
    dof[0] = _current_elem->node_ptr(nd)->dof_number(_sys.number(), _var.number(), 0);
    std::vector<dof_id_type> neighbors = _pdmesh.getNeighbors(_current_elem->node_id(nd));
    unsigned int nb_index =
        std::find(neighbors.begin(), neighbors.end(), _current_elem->node_id(1 - nd)) -
        neighbors.begin();
    std::vector<dof_id_type> dg_neighbors =
        _pdmesh.getDefGradNeighbors(_current_elem->node_id(nd), nb_index);
    std::vector<dof_id_type> bonds = _pdmesh.getBonds(_current_elem->node_id(nd));

    Real vol_nb, dJdU;
    RealGradient origin_vec_nb;
    RankTwoTensor dFdUk, dPxdUkx, dSdU, dinvFTdU;

    for (unsigned int nb = 0; nb < dg_neighbors.size(); ++nb)
      if (_bond_status_var->getElementalValue(_pdmesh.elemPtr(bonds[dg_neighbors[nb]])) > 0.5)
      {
        Node * dgneighbor_nb = _pdmesh.nodePtr(neighbors[dg_neighbors[nb]]);
        dof[1] = dgneighbor_nb->dof_number(_sys.number(), _var.number(), 0);
        vol_nb = _pdmesh.getPDNodeVolume(neighbors[dg_neighbors[nb]]);

        // obtain bond ndnb's origin vector
        origin_vec_nb = *dgneighbor_nb - *_pdmesh.nodePtr(_current_elem->node_id(nd));

        dFdUk.zero();
        for (unsigned int k = 0; k < _dim; ++k)
          dFdUk(_component, k) = _horiz_rad[nd] / origin_vec_nb.norm() * origin_vec_nb(k) * vol_nb;

        dFdUk *= _shape2[nd].inverse();

        // calculate dJ/du
        dJdU = 0.0;
        for (unsigned int i = 0; i < 3; ++i)
          for (unsigned int J = 0; J < 3; ++J)
            dJdU += detF * invF(J, i) * dFdUk(i, J);

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

        _local_ke.resize(_test.size(), _phi.size());
        _local_ke.zero();
        for (_i = 0; _i < _test.size(); ++_i)
          for (_j = 0; _j < _phi.size(); ++_j)
            _local_ke(_i, _j) = (_i == 0 ? -1 : 1) * (_j == 0 ? 0 : 1) * _multi[nd] *
                                (dPxdUkx * _shape2[nd].inverse()).row(_component) * _origin_vec *
                                _bond_status;

        _assembly.cacheJacobianBlock(_local_ke, _ivardofs, dof, _var.scalingFactor());

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
HorizonStabilizedFiniteStrainMechanicsNOSPD::computeLocalOffDiagJacobian(
    unsigned int coupled_component)
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
                             (dSdT[_j] * _shape2[_j].inverse()).row(_component) * _origin_vec *
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

    for (_i = 0; _i < _test.size(); ++_i)
      for (_j = 0; _j < _phi.size(); ++_j)
        _local_ke(_i, _j) += (_i == 0 ? -1 : 1) * _multi[_j] *
                             (dPxdUy[_j] * _shape2[_j].inverse()).row(_component) * _origin_vec *
                             _bond_status;
  }
}

void
HorizonStabilizedFiniteStrainMechanicsNOSPD::computePDNonlocalOffDiagJacobian(
    unsigned int jvar_num, unsigned int coupled_component)
{
  if (coupled_component != 3 && coupled_component != 4)
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
      unsigned int nb_index =
          std::find(neighbors.begin(), neighbors.end(), _current_elem->node_id(1 - nd)) -
          neighbors.begin();
      std::vector<dof_id_type> dg_neighbors =
          _pdmesh.getDefGradNeighbors(_current_elem->node_id(nd), nb_index);
      std::vector<dof_id_type> bonds = _pdmesh.getBonds(_current_elem->node_id(nd));

      Real vol_nb, dJdU;
      RealGradient origin_vec_nb;
      RankTwoTensor dFdUk, dPxdUky, dSdU, dinvFTdU;

      for (unsigned int nb = 0; nb < dg_neighbors.size(); ++nb)
        if (_bond_status_var->getElementalValue(_pdmesh.elemPtr(bonds[dg_neighbors[nb]])) > 0.5)
        {
          Node * dgneighbor_nb = _pdmesh.nodePtr(neighbors[dg_neighbors[nb]]);
          jvardofs[1] = dgneighbor_nb->dof_number(_sys.number(), jvar_num, 0);
          vol_nb = _pdmesh.getPDNodeVolume(neighbors[dg_neighbors[nb]]);

          // obtain bond ndnb's origin vector
          origin_vec_nb = *dgneighbor_nb - *_pdmesh.nodePtr(_current_elem->node_id(nd));

          dFdUk.zero();
          for (unsigned int k = 0; k < _dim; ++k)
            dFdUk(coupled_component, k) =
                _horiz_rad[nd] / origin_vec_nb.norm() * origin_vec_nb(k) * vol_nb;

          dFdUk *= _shape2[nd].inverse();

          // calculate dJ/du
          dJdU = 0.0;
          for (unsigned int i = 0; i < 3; ++i)
            for (unsigned int J = 0; J < 3; ++J)
              dJdU += detF * invF(J, i) * dFdUk(i, J);

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

          _local_ke.zero();
          for (_i = 0; _i < _test.size(); ++_i)
            for (_j = 0; _j < _phi.size(); ++_j)
              _local_ke(_i, _j) = (_i == 0 ? -1 : 1) * (_j == 0 ? 0 : 1) * _multi[nd] *
                                  (dPxdUky * _shape2[nd].inverse()).row(_component) * _origin_vec *
                                  _bond_status;

          _assembly.cacheJacobianBlock(_local_ke, _ivardofs, jvardofs, _var.scalingFactor());
        }
    }
  }
}

RankTwoTensor
HorizonStabilizedFiniteStrainMechanicsNOSPD::computeDSDU(unsigned int component, unsigned int nd)
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
HorizonStabilizedFiniteStrainMechanicsNOSPD::computeDSDFhat(unsigned int nd)
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
HorizonStabilizedFiniteStrainMechanicsNOSPD::computeDJDU(unsigned int component, unsigned int nd)
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
HorizonStabilizedFiniteStrainMechanicsNOSPD::computeDinvFTDU(unsigned int component,
                                                             unsigned int nd)
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
