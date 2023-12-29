//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADComputeFiniteStrain.h"
#include "RankTwoTensor.h"
#include "RankFourTensor.h"
#include "SymmetricRankTwoTensor.h"
#include "SymmetricRankFourTensor.h"

#include "libmesh/quadrature.h"
#include "libmesh/utility.h"

registerMooseObject("TensorMechanicsApp", ADComputeFiniteStrain);
registerMooseObject("TensorMechanicsApp", ADSymmetricFiniteStrain);

template <typename R2, typename R4>
MooseEnum
ADComputeFiniteStrainTempl<R2, R4>::decompositionType()
{
  return MooseEnum("TaylorExpansion EigenSolution", "TaylorExpansion");
}

template <typename R2, typename R4>
InputParameters
ADComputeFiniteStrainTempl<R2, R4>::validParams()
{
  InputParameters params = ADComputeIncrementalStrainBase::validParams();
  params.addClassDescription(
      "Compute a strain increment and rotation increment for finite strains.");
  params.addParam<MooseEnum>("decomposition_method",
                             ADComputeFiniteStrainTempl<R2, R4>::decompositionType(),
                             "Methods to calculate the strain and rotation increments");
  return params;
}

template <typename R2, typename R4>
ADComputeFiniteStrainTempl<R2, R4>::ADComputeFiniteStrainTempl(const InputParameters & parameters)
  : ADComputeIncrementalStrainBaseTempl<R2>(parameters),
    _Fhat(this->_fe_problem.getMaxQps()),
    _decomposition_method(
        this->template getParam<MooseEnum>("decomposition_method").template getEnum<DecompMethod>())
{
}

template <typename R2, typename R4>
void
ADComputeFiniteStrainTempl<R2, R4>::computeProperties()
{
  ADRankTwoTensor ave_Fhat;
  for (_qp = 0; _qp < _qrule->n_points(); ++_qp)
  {
    // Deformation gradient
    auto A = ADRankTwoTensor::initializeFromRows(
        (*_grad_disp[0])[_qp], (*_grad_disp[1])[_qp], (*_grad_disp[2])[_qp]);

    // Old Deformation gradient
    auto Fbar = ADRankTwoTensor::initializeFromRows(
        (*_grad_disp_old[0])[_qp], (*_grad_disp_old[1])[_qp], (*_grad_disp_old[2])[_qp]);

    // A = gradU - gradUold
    A -= Fbar;

    // Fbar = ( I + gradUold)
    Fbar.addIa(1.0);

    // Incremental deformation gradient _Fhat = I + A Fbar^-1
    _Fhat[_qp] = A * Fbar.inverse();
    _Fhat[_qp].addIa(1.0);

    // Calculate average _Fhat for volumetric locking correction
    if (_volumetric_locking_correction)
      ave_Fhat += _Fhat[_qp] * _JxW[_qp] * _coord[_qp];
  }

  if (_volumetric_locking_correction)
    ave_Fhat /= _current_elem_volume;

  const auto ave_Fhat_det = ave_Fhat.det();
  for (_qp = 0; _qp < _qrule->n_points(); ++_qp)
  {
    // Finalize volumetric locking correction
    if (_volumetric_locking_correction)
      _Fhat[_qp] *= std::cbrt(ave_Fhat_det / _Fhat[_qp].det());

    computeQpStrain();
  }
}

template <typename R2, typename R4>
void
ADComputeFiniteStrainTempl<R2, R4>::computeQpStrain()
{
  ADR2 total_strain_increment;

  // two ways to calculate these increments: TaylorExpansion(default) or EigenSolution
  computeQpIncrements(total_strain_increment, _rotation_increment[_qp]);

  _strain_increment[_qp] = total_strain_increment;

  // Remove the eigenstrain increment
  this->subtractEigenstrainIncrementFromStrain(_strain_increment[_qp]);

  if (_dt > 0)
    _strain_rate[_qp] = _strain_increment[_qp] / _dt;
  else
    _strain_rate[_qp].zero();

  // Update strain in intermediate configuration
  _mechanical_strain[_qp] = _mechanical_strain_old[_qp] + _strain_increment[_qp];
  _total_strain[_qp] = _total_strain_old[_qp] + total_strain_increment;

  // Rotate strain to current configuration
  _mechanical_strain[_qp].rotate(_rotation_increment[_qp]);
  _total_strain[_qp].rotate(_rotation_increment[_qp]);

  if (_global_strain)
    _total_strain[_qp] += (*_global_strain)[_qp];
}

template <typename R2, typename R4>
void
ADComputeFiniteStrainTempl<R2, R4>::computeQpIncrements(ADR2 & total_strain_increment,
                                                        ADRankTwoTensor & rotation_increment)
{
  switch (_decomposition_method)
  {
    case DecompMethod::TaylorExpansion:
    {
      // inverse of _Fhat
      const ADRankTwoTensor invFhat = _Fhat[_qp].inverse();

      // A = I - _Fhat^-1
      ADRankTwoTensor A(ADRankTwoTensor::initIdentity);
      A -= invFhat;

      // Cinv - I = A A^T - (A + A^T);
      ADR2 Cinv_I = ADR2::timesTranspose(A) - ADR2::plusTranspose(A);

      // strain rate D from Taylor expansion, Chat = (-1/2(Chat^-1 - I) + 1/4*(Chat^-1 - I)^2 + ...
      total_strain_increment = -Cinv_I * 0.5 + Cinv_I.square() * 0.25;

      const ADReal a[3] = {invFhat(1, 2) - invFhat(2, 1),
                           invFhat(2, 0) - invFhat(0, 2),
                           invFhat(0, 1) - invFhat(1, 0)};

      const auto q = (a[0] * a[0] + a[1] * a[1] + a[2] * a[2]) / 4.0;
      const auto trFhatinv_1 = invFhat.trace() - 1.0;
      const auto p = trFhatinv_1 * trFhatinv_1 / 4.0;

      // cos theta_a
      const ADReal C1_squared =
          p + 3.0 * Utility::pow<2>(p) * (1.0 - (p + q)) / Utility::pow<2>(p + q) -
          2.0 * Utility::pow<3>(p) * (1.0 - (p + q)) / Utility::pow<3>(p + q);
      if (C1_squared <= 0.0)
        mooseException(
            "Cannot take square root of a number less than or equal to zero in the calculation of "
            "C1 for the Rashid approximation for the rotation tensor. This zero or negative number "
            "may occur when elements become heavily distorted.");

      const ADReal C1 = std::sqrt(C1_squared);

      ADReal C2;
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

      const ADReal C3_test =
          (p * q * (3.0 - q) + Utility::pow<3>(p) + Utility::pow<2>(q)) / Utility::pow<3>(p + q);
      if (C3_test <= 0.0)
        mooseException(
            "Cannot take square root of a number less than or equal to zero in the calculation of "
            "C3_test for the Rashid approximation for the rotation tensor. This zero or negative "
            "number may occur when elements become heavily distorted.");
      const ADReal C3 = 0.5 * std::sqrt(C3_test); // sin theta_a/(2 sqrt(q))

      // Calculate incremental rotation. Note that this value is the transpose of that from Rashid,
      // 93, so we transpose it before storing
      ADRankTwoTensor R_incr;
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
      FADR2 Chat = ADR2::transposeTimes(_Fhat[_qp]);
      FADR2 Uhat = MathUtils::sqrt(Chat);
      rotation_increment = _Fhat[_qp] * Uhat.inverse().template get<ADRankTwoTensor>();
      total_strain_increment = MathUtils::log(Uhat).template get<ADR2>();
      break;
    }

    default:
      mooseError("ADComputeFiniteStrain Error: Pass valid decomposition type: TaylorExpansion or "
                 "EigenSolution.");
  }
}

template class ADComputeFiniteStrainTempl<RankTwoTensor, RankFourTensor>;
template class ADComputeFiniteStrainTempl<SymmetricRankTwoTensor, SymmetricRankFourTensor>;
