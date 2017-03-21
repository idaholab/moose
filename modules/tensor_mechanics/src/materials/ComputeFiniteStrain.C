/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "ComputeFiniteStrain.h"
#include "Assembly.h"

// libmesh includes
#include "libmesh/quadrature.h"

MooseEnum
ComputeFiniteStrain::decompositionType()
{
  return MooseEnum("TaylorExpansion EigenSolution", "TaylorExpansion");
}

template <>
InputParameters
validParams<ComputeFiniteStrain>()
{
  InputParameters params = validParams<ComputeIncrementalStrainBase>();
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
    _decomposition_method(getParam<MooseEnum>("decomposition_method").getEnum<DecompMethod>())
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
    RankTwoTensor A((*_grad_disp[0])[_qp],
                    (*_grad_disp[1])[_qp],
                    (*_grad_disp[2])[_qp]); // Deformation gradient
    RankTwoTensor Fbar((*_grad_disp_old[0])[_qp],
                       (*_grad_disp_old[1])[_qp],
                       (*_grad_disp_old[2])[_qp]); // Old Deformation gradient

    _deformation_gradient[_qp] = A;
    _deformation_gradient[_qp].addIa(1.0); // Gauss point deformation gradient

    // A = gradU - gradUold
    A -= Fbar;

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

  // two ways to calculate these increments: TaylorExpansion(default) or EigenSolution
  computeQpIncrements(total_strain_increment, _rotation_increment[_qp]);

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
          std::sqrt(p + 3.0 * std::pow(p, 2.0) * (1.0 - (p + q)) / std::pow(p + q, 2.0) -
                    2.0 * std::pow(p, 3.0) * (1.0 - (p + q)) / std::pow(p + q, 3.0));

      Real C2;
      if (q > 0.01)
        // (1-cos theta_a)/4q
        C2 = (1.0 - C1) / (4.0 * q);
      else
        // alternate form for small q
        C2 =
            0.125 + q * 0.03125 * (std::pow(p, 2.0) - 12.0 * (p - 1.0)) / std::pow(p, 2.0) +
            std::pow(q, 2.0) * (p - 2.0) * (std::pow(p, 2.0) - 10.0 * p + 32.0) / std::pow(p, 3.0) +
            std::pow(q, 3.0) * (1104.0 - 992.0 * p + 376.0 * std::pow(p, 2.0) -
                                72.0 * std::pow(p, 3.0) + 5.0 * std::pow(p, 4.0)) /
                (512.0 * std::pow(p, 4.0));

      const Real C3 = 0.5 * std::sqrt((p * q * (3.0 - q) + std::pow(p, 3.0) + std::pow(q, 2.0)) /
                                      std::pow(p + q, 3.0)); // sin theta_a/(2 sqrt(q))

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
      RankTwoTensor e_vector, N1, N2, N3;

      RankTwoTensor Chat = _Fhat[_qp].transpose() * _Fhat[_qp];
      Chat.symmetricEigenvaluesEigenvectors(e_value, e_vector);

      const Real lambda1 = std::sqrt(e_value[0]);
      const Real lambda2 = std::sqrt(e_value[1]);
      const Real lambda3 = std::sqrt(e_value[2]);

      N1.vectorOuterProduct(e_vector.column(0), e_vector.column(0));
      N2.vectorOuterProduct(e_vector.column(1), e_vector.column(1));
      N3.vectorOuterProduct(e_vector.column(2), e_vector.column(2));

      RankTwoTensor Uhat = N1 * lambda1 + N2 * lambda2 + N3 * lambda3;
      RankTwoTensor invUhat(Uhat.inverse());

      rotation_increment = _Fhat[_qp] * invUhat;

      total_strain_increment =
          N1 * std::log(lambda1) + N2 * std::log(lambda2) + N3 * std::log(lambda3);
      break;
    }

    default:
      mooseError("ComputeFiniteStrain Error: Pass valid decomposition type: TaylorExpansion or "
                 "EigenSolution.");
  }
}
