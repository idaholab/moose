//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MechanicsFiniteStrainBaseNOSPD.h"

InputParameters
MechanicsFiniteStrainBaseNOSPD::validParams()
{
  InputParameters params = MechanicsBaseNOSPD::validParams();
  params.addClassDescription("Base class for kernels of the stabilized non-ordinary "
                             "state-based peridynamic correspondence models");

  params.addParam<std::vector<MaterialPropertyName>>(
      "eigenstrain_names",
      "List of eigenstrains to be coupled in non-ordinary state-based mechanics kernels");

  return params;
}

MechanicsFiniteStrainBaseNOSPD::MechanicsFiniteStrainBaseNOSPD(const InputParameters & parameters)
  : MechanicsBaseNOSPD(parameters),
    _dgrad_old(getMaterialPropertyOld<RankTwoTensor>("deformation_gradient")),
    _E_inc(getMaterialProperty<RankTwoTensor>("strain_increment")),
    _R_inc(getMaterialProperty<RankTwoTensor>("rotation_increment"))
{
}

RankTwoTensor
MechanicsFiniteStrainBaseNOSPD::computeDSDU(unsigned int component, unsigned int nd)
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
MechanicsFiniteStrainBaseNOSPD::computeDSDFhat(unsigned int nd)
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
MechanicsFiniteStrainBaseNOSPD::computeDJDU(unsigned int component, unsigned int nd)
{
  // for finite formulation, compute the derivative of determinant of deformation gradient w.r.t the
  // solution components
  // dJ / du = dJ / dF_ij * dF_ij / du = J * inv(F)_ji * dF_ij / du

  Real dJdU = 0.0;
  RankTwoTensor invF = _dgrad[nd].inverse();
  Real detF = _dgrad[nd].det();
  for (unsigned int i = 0; i < 3; ++i)
    for (unsigned int j = 0; j < 3; ++j)
    {
      if (component == 0)
        dJdU += detF * invF(j, i) * _ddgraddu[nd](i, j);
      else if (component == 1)
        dJdU += detF * invF(j, i) * _ddgraddv[nd](i, j);
      else if (component == 2)
        dJdU += detF * invF(j, i) * _ddgraddw[nd](i, j);
    }

  return dJdU;
}

RankTwoTensor
MechanicsFiniteStrainBaseNOSPD::computeDinvFTDU(unsigned int component, unsigned int nd)
{
  // for finite formulation, compute the derivative of transpose of inverse of deformation gradient
  // w.r.t the solution components
  // d(inv(F)_ji)/du = d(inv(F)_ji)/dF_kl * dF_kl/du = - inv(F)_jk * inv(F)_li * dF_kl/du
  // the bases are gi, gj, gk, gl, indictates the inverse transpose rather than the inverse

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
    for (unsigned int j = 0; j < 3; ++j)
      for (unsigned int k = 0; k < 3; ++k)
        for (unsigned int l = 0; l < 3; ++l)
        {
          if (component == 0)
            dinvFTdU(i, j) -= invF(i, j) * invF(l, k) * _ddgraddu[nd](k, l);
          else if (component == 1)
            dinvFTdU(i, j) -= invF(i, j) * invF(l, k) * _ddgraddv[nd](k, l);
          else if (component == 2)
            dinvFTdU(i, j) -= invF(i, j) * invF(l, k) * _ddgraddw[nd](k, l);
        }

  return dinvFTdU;
}
