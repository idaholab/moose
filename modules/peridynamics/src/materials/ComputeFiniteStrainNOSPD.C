//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeFiniteStrainNOSPD.h"

#include "libmesh/utility.h"

registerMooseObject("PeridynamicsApp", ComputeFiniteStrainNOSPD);

MooseEnum
ComputeFiniteStrainNOSPD::decompositionType()
{
  return MooseEnum("TaylorExpansion EigenSolution", "TaylorExpansion");
}

InputParameters
ComputeFiniteStrainNOSPD::validParams()
{
  InputParameters params = ComputeStrainBaseNOSPD::validParams();
  params.addClassDescription(
      "Class for computing nodal quantities for residual and jacobian calculation "
      "for peridynamic correspondence models under finite strain assumptions");

  params.addParam<MooseEnum>("decomposition_method",
                             ComputeFiniteStrainNOSPD::decompositionType(),
                             "Methods to calculate the strain and rotation increments");

  return params;
}

ComputeFiniteStrainNOSPD::ComputeFiniteStrainNOSPD(const InputParameters & parameters)
  : ComputeStrainBaseNOSPD(parameters),
    _strain_rate(declareProperty<RankTwoTensor>("strain_rate")),
    _strain_increment(declareProperty<RankTwoTensor>("strain_increment")),
    _rotation_increment(declareProperty<RankTwoTensor>("rotation_increment")),
    _deformation_gradient_old(getMaterialPropertyOld<RankTwoTensor>("deformation_gradient")),
    _mechanical_strain_old(getMaterialPropertyOld<RankTwoTensor>("mechanical_strain")),
    _total_strain_old(getMaterialPropertyOld<RankTwoTensor>("total_strain")),
    _eigenstrains_old(_eigenstrain_names.size()),
    _Fhat(2),
    _decomposition_method(getParam<MooseEnum>("decomposition_method").getEnum<DecompMethod>())
{
  for (unsigned int i = 0; i < _eigenstrains_old.size(); ++i)
    _eigenstrains_old[i] = &getMaterialPropertyOld<RankTwoTensor>(_eigenstrain_names[i]);
}

void
ComputeFiniteStrainNOSPD::computeQpStrain()
{
  computeQpDeformationGradient();

  computeQpFhat();

  RankTwoTensor total_strain_increment;

  // Two ways to calculate these increments: TaylorExpansion(default) or EigenSolution
  computeQpStrainRotationIncrements(total_strain_increment, _rotation_increment[_qp]);

  _strain_increment[_qp] = total_strain_increment;

  // Remove the eigenstrain increment
  subtractEigenstrainIncrementFromStrain(_strain_increment[_qp]);

  if (_dt > 0)
    _strain_rate[_qp] = _strain_increment[_qp] / _dt;
  else
    _strain_rate[_qp].zero();

  // Update strain in intermediate configuration
  _mechanical_strain[_qp] = _mechanical_strain_old[_qp] + _strain_increment[_qp];
  _total_strain[_qp] = _total_strain_old[_qp] + total_strain_increment;

  // Rotate strain to current configuration
  _mechanical_strain[_qp] =
      _rotation_increment[_qp] * _mechanical_strain[_qp] * _rotation_increment[_qp].transpose();
  _total_strain[_qp] =
      _rotation_increment[_qp] * _total_strain[_qp] * _rotation_increment[_qp].transpose();

  // zero out all strain measures for broken bond
  if (_bond_status_var->getElementalValue(_current_elem) < 0.5)
  {
    _strain_rate[_qp].zero();
    _strain_increment[_qp].zero();
    _rotation_increment[_qp].zero();
    _mechanical_strain[_qp].zero();
    _total_strain[_qp].zero();
  }
}

void
ComputeFiniteStrainNOSPD::computeQpFhat()
{
  // Incremental deformation gradient of current step w.r.t previous step:
  // _Fhat = deformation_gradient * inv(deformation_gradient_old)
  _Fhat[_qp] = _deformation_gradient[_qp] * _deformation_gradient_old[_qp].inverse();
}

void
ComputeFiniteStrainNOSPD::computeQpStrainRotationIncrements(RankTwoTensor & total_strain_increment,
                                                            RankTwoTensor & rotation_increment)
{
  switch (_decomposition_method)
  {
    case DecompMethod::TaylorExpansion:
    {
      // inverse of _Fhat
      RankTwoTensor invFhat(_Fhat[_qp].inverse());

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
      const Real C1 =
          std::sqrt(p + 3.0 * Utility::pow<2>(p) * (1.0 - (p + q)) / Utility::pow<2>(p + q) -
                    2.0 * Utility::pow<3>(p) * (1.0 - (p + q)) / Utility::pow<3>(p + q));

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
      const Real C3 =
          0.5 * std::sqrt((p * q * (3.0 - q) + Utility::pow<3>(p) + Utility::pow<2>(q)) /
                          Utility::pow<3>(p + q)); // sin theta_a/(2 sqrt(q))

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
      std::vector<Real> e_value(3);
      RankTwoTensor e_vector;

      RankTwoTensor Chat = _Fhat[_qp].transpose() * _Fhat[_qp];
      Chat.symmetricEigenvaluesEigenvectors(e_value, e_vector);

      const Real lambda1 = std::sqrt(e_value[0]);
      const Real lambda2 = std::sqrt(e_value[1]);
      const Real lambda3 = std::sqrt(e_value[2]);

      const auto N1 = RankTwoTensor::selfOuterProduct(e_vector.column(0));
      const auto N2 = RankTwoTensor::selfOuterProduct(e_vector.column(1));
      const auto N3 = RankTwoTensor::selfOuterProduct(e_vector.column(2));

      RankTwoTensor Uhat = N1 * lambda1 + N2 * lambda2 + N3 * lambda3;
      RankTwoTensor invUhat(Uhat.inverse());

      rotation_increment = _Fhat[_qp] * invUhat;

      total_strain_increment =
          N1 * std::log(lambda1) + N2 * std::log(lambda2) + N3 * std::log(lambda3);
      break;
    }

    default:
      mooseError("ComputeFiniteStrainNOSPD Error: Invalid decomposition type! Please specify : "
                 "TaylorExpansion or "
                 "EigenSolution.");
  }
}

void
ComputeFiniteStrainNOSPD::subtractEigenstrainIncrementFromStrain(RankTwoTensor & strain)
{
  for (unsigned int i = 0; i < _eigenstrains.size(); ++i)
  {
    strain -= (*_eigenstrains[i])[_qp];
    strain += (*_eigenstrains_old[i])[_qp];
  }
}
