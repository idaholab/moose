//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeFiniteStrain.h"
#include "Assembly.h"

#include "libmesh/quadrature.h"
#include "libmesh/utility.h"

MooseEnum
ComputeFiniteStrain::decompositionType()
{
  return MooseEnum("TaylorExpansion EigenSolution HughesWinget", "TaylorExpansion");
}

registerMooseObject("TensorMechanicsApp", ComputeFiniteStrain);

InputParameters
ComputeFiniteStrain::validParams()
{
  InputParameters params = ComputeIncrementalStrainBase::validParams();
  params.addClassDescription(
      "Compute a strain increment and rotation increment for finite strains.");
  params.addParam<MooseEnum>("decomposition_method",
                             ComputeFiniteStrain::decompositionType(),
                             "Methods to calculate the strain and rotation increments");
  return params;
}

ComputeFiniteStrain::ComputeFiniteStrain(const InputParameters & parameters)
  : ComputeIncrementalStrainBase(parameters),
    _Fhat(_fe_problem.getMaxQps()),
    _decomposition_method(getParam<MooseEnum>("decomposition_method").getEnum<DecompMethod>()),
    _use_hw(_decomposition_method == DecompMethod::HughesWinget),
    _def_grad_mid(_use_hw ? &declareProperty<RankTwoTensor>(_base_name + "def_grad_mid") : nullptr),
    _f_bar(_use_hw ? &declareProperty<RankTwoTensor>(_base_name + "f_bar") : nullptr)
{
}

void
ComputeFiniteStrain::computeProperties()
{
  RankTwoTensor ave_Fhat;
  Real ave_dfgrd_det = 0.0;
  for (_qp = 0; _qp < _qrule->n_points(); ++_qp)
  {
    // Deformation gradient
    auto A = RankTwoTensor::initializeFromRows(
        (*_grad_disp[0])[_qp], (*_grad_disp[1])[_qp], (*_grad_disp[2])[_qp]);

    // Old Deformation gradient
    auto Fbar = RankTwoTensor::initializeFromRows(
        (*_grad_disp_old[0])[_qp], (*_grad_disp_old[1])[_qp], (*_grad_disp_old[2])[_qp]);

    // Gauss point deformation gradient
    _deformation_gradient[_qp] = A;
    _deformation_gradient[_qp].addIa(1.0);

    // deformation gradient midpoint (for Hughes-Winget kinematics)
    if (_use_hw)
    {
      (*_def_grad_mid)[_qp].setToIdentity();
      (*_def_grad_mid)[_qp] += 0.5 * (A + Fbar);
    }

    // A = gradU - gradUold
    A -= Fbar;

    //_f_bar = dDu/Dx_o (for Hughes-Winget kinematics)
    if (_use_hw)
      (*_f_bar)[_qp] = A;

    // Fbar = ( I + gradUold)
    Fbar.addIa(1.0);

    // Incremental deformation gradient _Fhat = I + A Fbar^-1
    _Fhat[_qp] = A * Fbar.inverse();
    _Fhat[_qp].addIa(1.0);

    if (_volumetric_locking_correction)
    {
      // Calculate average _Fhat for volumetric locking correction
      ave_Fhat += _Fhat[_qp] * _JxW[_qp] * _coord[_qp];

      // Average deformation gradient
      ave_dfgrd_det += _deformation_gradient[_qp].det() * _JxW[_qp] * _coord[_qp];
    }
  }

  if (_volumetric_locking_correction)
  {
    // needed for volumetric locking correction
    ave_Fhat /= _current_elem_volume;
    // average deformation gradient
    ave_dfgrd_det /= _current_elem_volume;
  }

  for (_qp = 0; _qp < _qrule->n_points(); ++_qp)
  {
    if (_volumetric_locking_correction)
    {
      // Finalize volumetric locking correction
      _Fhat[_qp] *= std::cbrt(ave_Fhat.det() / _Fhat[_qp].det());
      _deformation_gradient[_qp] *= std::cbrt(ave_dfgrd_det / _deformation_gradient[_qp].det());
    }

    computeQpStrain();
  }
}

void
ComputeFiniteStrain::computeQpStrain()
{
  RankTwoTensor total_strain_increment;

  // three ways to calculate these increments: TaylorExpansion(default), EigenSolution, or
  // HughesWinget
  computeQpIncrements(total_strain_increment, _rotation_increment[_qp]);

  _strain_increment[_qp] = total_strain_increment;

  // Remove the eigenstrain increment
  subtractEigenstrainIncrementFromStrain(_strain_increment[_qp]);

  if (_dt > 0)
    _strain_rate[_qp] = _strain_increment[_qp] / _dt;
  else
    _strain_rate[_qp].zero();

  // if HughesWinget, rotate old strains here
  RankTwoTensor mechanical_strain_old = _mechanical_strain_old[_qp];
  RankTwoTensor total_strain_old = _total_strain_old[_qp];
  if (_use_hw)
  {
    mechanical_strain_old = _rotation_increment[_qp] * _mechanical_strain_old[_qp] *
                            _rotation_increment[_qp].transpose();
    total_strain_old =
        _rotation_increment[_qp] * _total_strain_old[_qp] * _rotation_increment[_qp].transpose();
  }

  // Update strain in intermediate configuration
  _mechanical_strain[_qp] = mechanical_strain_old + _strain_increment[_qp];
  _total_strain[_qp] = total_strain_old + total_strain_increment;

  // Rotate strain to current configuration, unless HughesWinget
  if (!_use_hw)
  {
    _mechanical_strain[_qp] =
        _rotation_increment[_qp] * _mechanical_strain[_qp] * _rotation_increment[_qp].transpose();
    _total_strain[_qp] =
        _rotation_increment[_qp] * _total_strain[_qp] * _rotation_increment[_qp].transpose();
  }

  if (_global_strain)
    _total_strain[_qp] += (*_global_strain)[_qp];
}

void
ComputeFiniteStrain::computeQpIncrements(RankTwoTensor & total_strain_increment,
                                         RankTwoTensor & rotation_increment)
{
  switch (_decomposition_method)
  {
    case DecompMethod::TaylorExpansion:
    {
      // inverse of _Fhat
      const RankTwoTensor invFhat = _Fhat[_qp].inverse();

      // A = I - _Fhat^-1
      RankTwoTensor A(RankTwoTensor::initIdentity);
      A -= invFhat;

      // Cinv - I = A A^T - A - A^T;
      RankTwoTensor Cinv_I = A * A.transpose() - A - A.transpose();

      // strain rate D from Taylor expansion, Chat = (-1/2(Chat^-1 - I) + 1/4*(Chat^-1 - I)^2 + ...
      total_strain_increment = -Cinv_I * 0.5 + Cinv_I * Cinv_I * 0.25;

      const Real a[3] = {invFhat(1, 2) - invFhat(2, 1),
                         invFhat(2, 0) - invFhat(0, 2),
                         invFhat(0, 1) - invFhat(1, 0)};

      Real q = (a[0] * a[0] + a[1] * a[1] + a[2] * a[2]) / 4.0;
      Real trFhatinv_1 = invFhat.trace() - 1.0;
      const Real p = trFhatinv_1 * trFhatinv_1 / 4.0;

      // cos theta_a
      const Real C1_squared = p +
                              3.0 * Utility::pow<2>(p) * (1.0 - (p + q)) / Utility::pow<2>(p + q) -
                              2.0 * Utility::pow<3>(p) * (1.0 - (p + q)) / Utility::pow<3>(p + q);
      if (C1_squared <= 0.0)
        mooseException(
            "Cannot take square root of a number less than or equal to zero in the calculation of "
            "C1 for the Rashid approximation for the rotation tensor. This zero or negative number "
            "may occur when elements become heavily distorted.");

      const Real C1 = std::sqrt(C1_squared);

      Real C2;
      if (q > 0.01)
        // (1-cos theta_a)/4q
        C2 = (1.0 - C1) / (4.0 * q);
      else
        // alternate form for small q
        C2 = 0.125 + q * 0.03125 * (Utility::pow<2>(p) - 12.0 * (p - 1.0)) / Utility::pow<2>(p) +
             Utility::pow<2>(q) * (p - 2.0) * (Utility::pow<2>(p) - 10.0 * p + 32.0) /
                 Utility::pow<3>(p) +
             Utility::pow<3>(q) *
                 (1104.0 - 992.0 * p + 376.0 * Utility::pow<2>(p) - 72.0 * Utility::pow<3>(p) +
                  5.0 * Utility::pow<4>(p)) /
                 (512.0 * Utility::pow<4>(p));

      const Real C3_test =
          (p * q * (3.0 - q) + Utility::pow<3>(p) + Utility::pow<2>(q)) / Utility::pow<3>(p + q);

      if (C3_test <= 0.0)
        mooseException(
            "Cannot take square root of a number less than or equal to zero in the calculation of "
            "C3_test for the Rashid approximation for the rotation tensor. This zero or negative "
            "number may occur when elements become heavily distorted.");

      const Real C3 = 0.5 * std::sqrt(C3_test); // sin theta_a/(2 sqrt(q))

      // Calculate incremental rotation. Note that this value is the transpose of that from Rashid,
      // 93, so we transpose it before storing
      RankTwoTensor R_incr;
      R_incr.addIa(C1);
      for (unsigned int i = 0; i < 3; ++i)
        for (unsigned int j = 0; j < 3; ++j)
          R_incr(i, j) += C2 * a[i] * a[j];

      R_incr(0, 1) += C3 * a[2];
      R_incr(0, 2) -= C3 * a[1];
      R_incr(1, 0) -= C3 * a[2];
      R_incr(1, 2) += C3 * a[0];
      R_incr(2, 0) += C3 * a[1];
      R_incr(2, 1) -= C3 * a[0];

      rotation_increment = R_incr.transpose();
      break;
    }

    case DecompMethod::EigenSolution:
    {
      FactorizedRankTwoTensor Chat = RankTwoTensor::transposeTimes(_Fhat[_qp]);
      FactorizedRankTwoTensor Uhat = MathUtils::sqrt(Chat);
      rotation_increment = _Fhat[_qp] * Uhat.inverse().get();
      total_strain_increment = MathUtils::log(Uhat).get();
      break;
    }

    case DecompMethod::HughesWinget:
    {
      const RankTwoTensor G = (*_f_bar)[_qp] * (*_def_grad_mid)[_qp].inverse();

      total_strain_increment = 0.5 * (G + G.transpose());
      const RankTwoTensor W = 0.5 * (G - G.transpose());

      RankTwoTensor Q_1(RankTwoTensor::initIdentity);
      RankTwoTensor Q_2(RankTwoTensor::initIdentity);

      Q_1 -= 0.5 * W;
      Q_2 += 0.5 * W;

      rotation_increment = Q_1.inverse() * Q_2;

      break;
    }

    default:
      mooseError("ComputeFiniteStrain Error: Pass valid decomposition type: TaylorExpansion, "
                 "EigenSolution, or HughesWinget.");
  }
}
