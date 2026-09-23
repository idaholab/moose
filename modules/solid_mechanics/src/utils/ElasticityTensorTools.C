//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MooseTypes.h"
#include "PermutationTensor.h"
#include "RankFourTensor.h"
#include "ElasticityTensorTools.h"

#include "libmesh/vector_value.h"

namespace ElasticityTensorTools
{

Real
elasticJacobian(const RankFourTensor & r4t,
                unsigned int i,
                unsigned int k,
                const RealGradient & grad_test,
                const RealGradient & grad_phi)
{
  // d(stress_ij*d(test)/dx_j)/du_k = d(C_ijmn*du_m/dx_n dtest/dx_j)/du_k (which is nonzero for m ==
  // k)

  const Real gt0 = grad_test(0);
  const Real gt1 = grad_test(1);
  const Real gt2 = grad_test(2);
  const Real gp0 = grad_phi(0);
  const Real gp1 = grad_phi(1);
  const Real gp2 = grad_phi(2);

  // clang-format off
  // This is the algorithm that is unrolled below:
  //
  //    Real sum = 0.0;
  //    for (const auto j: make_range(Moose::dim))
  //      for (const auto l: make_range(Moose::dim))
  //        sum += r4t(i, j, k, l) * grad_phi(l) * grad_test(j);
  //    return sum;

  return
     (
         r4t(i,0,k,0) * gp0
       + r4t(i,0,k,1) * gp1
       + r4t(i,0,k,2) * gp2
     ) * gt0
     +
     (
         r4t(i,1,k,0) * gp0
       + r4t(i,1,k,1) * gp1
       + r4t(i,1,k,2) * gp2
     ) * gt1
     +
     (
         r4t(i,2,k,0) * gp0
       + r4t(i,2,k,1) * gp1
       + r4t(i,2,k,2) * gp2
     ) * gt2;
  // clang-format on
}

Real
elasticJacobianWC(const RankFourTensor & r4t,
                  unsigned int i,
                  unsigned int k,
                  const RealGradient & grad_test,
                  Real phi)
{
  // d(stress_ij*d(test)/dx_j)/dw_k = d(C_ijmn*eps_mnp*w_p*dtest/dx_j)/dw_k (only nonzero for p ==
  // k)
  Real sum = 0.0;
  for (const auto j : make_range(Moose::dim))
    for (const auto m : make_range(Moose::dim))
      for (const auto n : make_range(Moose::dim))
        sum += r4t(i, j, m, n) * PermutationTensor::eps(m, n, k) * grad_test(j);
  return sum * phi;
}

Real
momentJacobian(const RankFourTensor & r4t,
               unsigned int i,
               unsigned int k,
               Real test,
               const RealGradient & grad_phi)
{
  // Jacobian entry: d(eps_ijm*stress_jm*test)/du_k = d(eps_ijm*C_jmln*du_l/dx_n*test)/du_k (only
  // nonzero for l == k)
  Real sum = 0.0;
  for (const auto j : make_range(Moose::dim))
    for (const auto m : make_range(Moose::dim))
      for (const auto n : make_range(Moose::dim))
        sum += PermutationTensor::eps(i, j, m) * r4t(j, m, k, n) * grad_phi(n);
  return test * sum;
}

Real
momentJacobianWC(const RankFourTensor & r4t, unsigned int i, unsigned int k, Real test, Real phi)
{
  // Jacobian entry: d(eps_ijm*stress_jm*test)/dw_k = d(eps_ijm*C_jmln*eps_lnp*w_p*test)/dw_k (only
  // nonzero for p ==k)
  Real sum = 0.0;
  for (const auto j : make_range(Moose::dim))
    for (const auto l : make_range(Moose::dim))
      for (const auto m : make_range(Moose::dim))
        for (const auto n : make_range(Moose::dim))
          sum +=
              PermutationTensor::eps(i, j, m) * r4t(j, m, l, n) * PermutationTensor::eps(l, n, k);

  return test * phi * sum;
}

IsotropicElasticConstants
isotropicElasticConstants(const IsotropicElasticInput & input)
{
  const bool bulk_modulus_set = input.bulk_modulus.has_value();
  const bool lambda_set = input.lambda.has_value();
  const bool poissons_ratio_set = input.poissons_ratio.has_value();
  const bool shear_modulus_set = input.shear_modulus.has_value();
  const bool youngs_modulus_set = input.youngs_modulus.has_value();

  const unsigned int num_supplied =
      bulk_modulus_set + lambda_set + poissons_ratio_set + shear_modulus_set + youngs_modulus_set;
  if (num_supplied != 2)
    mooseError("Exactly two isotropic elastic constants are required, but ",
               num_supplied,
               " were supplied.");

  const Real bulk_modulus = input.bulk_modulus.value_or(0);
  const Real lambda = input.lambda.value_or(0);
  const Real poissons_ratio = input.poissons_ratio.value_or(0);
  const Real shear_modulus = input.shear_modulus.value_or(0);
  const Real youngs_modulus = input.youngs_modulus.value_or(0);

  IsotropicElasticConstants result;
  Real elas_mod;
  Real poiss_rat;

  if (youngs_modulus_set && poissons_ratio_set)
  {
    // The conversion RankFourTensorTempl::fillSymmetricIsotropicEandNu performs internally
    result.lambda =
        youngs_modulus * poissons_ratio / ((1.0 + poissons_ratio) * (1.0 - 2.0 * poissons_ratio));
    result.shear_modulus = youngs_modulus / (2.0 * (1.0 + poissons_ratio));
    result.effective_stiffness =
        std::max(std::sqrt((youngs_modulus * (1 - poissons_ratio)) /
                           ((1 + poissons_ratio) * (1 - 2 * poissons_ratio))),
                 std::sqrt(youngs_modulus / (2 * (1 + poissons_ratio))));
    return result;
  }

  if (lambda_set && shear_modulus_set)
  {
    result.lambda = lambda;
    result.shear_modulus = shear_modulus;
    elas_mod = (shear_modulus * (3 * lambda + 2 * shear_modulus)) / (lambda + shear_modulus);
    poiss_rat = lambda / (2 * (lambda + shear_modulus));
    result.effective_stiffness =
        std::max(std::sqrt((elas_mod * (1 - poiss_rat)) / ((1 + poiss_rat) * (1 - 2 * poiss_rat))),
                 std::sqrt(shear_modulus));
  }
  else if (shear_modulus_set && bulk_modulus_set)
  {
    result.lambda = bulk_modulus - 2.0 / 3.0 * shear_modulus;
    result.shear_modulus = shear_modulus;
    elas_mod = (9 * bulk_modulus * shear_modulus) / (3 * bulk_modulus + shear_modulus);
    poiss_rat = (3 * bulk_modulus - 2 * shear_modulus) / (2 * (3 * bulk_modulus + shear_modulus));
    result.effective_stiffness =
        std::max(std::sqrt((elas_mod * (1 - poiss_rat)) / ((1 + poiss_rat) * (1 - 2 * poiss_rat))),
                 std::sqrt(shear_modulus));
  }
  else if (poissons_ratio_set && bulk_modulus_set)
  {
    result.lambda = 3.0 * bulk_modulus * poissons_ratio / (1.0 + poissons_ratio);
    result.shear_modulus =
        3.0 * bulk_modulus * (1.0 - 2.0 * poissons_ratio) / (2.0 * (1.0 + poissons_ratio));
    elas_mod = 3 * bulk_modulus * (1 - 2 * poissons_ratio);
    poiss_rat = poissons_ratio;
    result.effective_stiffness =
        std::max(std::sqrt((elas_mod * (1 - poiss_rat)) / ((1 + poiss_rat) * (1 - 2 * poiss_rat))),
                 std::sqrt(elas_mod / (2 * (1 + poiss_rat))));
  }
  else if (lambda_set && bulk_modulus_set)
  {
    result.lambda = lambda;
    result.shear_modulus = 3.0 * (bulk_modulus - lambda) / 2.0;
    elas_mod = (9 * bulk_modulus * (bulk_modulus - lambda)) / (3 * bulk_modulus - lambda);
    poiss_rat = (lambda) / ((3 * bulk_modulus - lambda));
    result.effective_stiffness =
        std::max(std::sqrt((elas_mod * (1 - poiss_rat)) / ((1 + poiss_rat) * (1 - 2 * poiss_rat))),
                 std::sqrt(elas_mod / (2 * (1 + poiss_rat))));
  }
  else if (shear_modulus_set && youngs_modulus_set)
  {
    result.lambda = shear_modulus * (youngs_modulus - 2.0 * shear_modulus) /
                    (3.0 * shear_modulus - youngs_modulus);
    result.shear_modulus = shear_modulus;
    elas_mod = youngs_modulus;
    poiss_rat = (youngs_modulus - 2 * shear_modulus) / (2 * shear_modulus);
    result.effective_stiffness =
        std::max(std::sqrt((elas_mod * (1 - poiss_rat)) / ((1 + poiss_rat) * (1 - 2 * poiss_rat))),
                 std::sqrt(elas_mod / (2 * (1 + poiss_rat))));
  }
  else if (shear_modulus_set && poissons_ratio_set)
  {
    result.lambda = 2.0 * shear_modulus * poissons_ratio / (1.0 - 2.0 * poissons_ratio);
    result.shear_modulus = shear_modulus;
    elas_mod = (2 * shear_modulus * (1 + poissons_ratio));
    poiss_rat = (poissons_ratio);
    result.effective_stiffness =
        std::max(std::sqrt((elas_mod * (1 - poiss_rat)) / ((1 + poiss_rat) * (1 - 2 * poiss_rat))),
                 std::sqrt(elas_mod / (2 * (1 + poiss_rat))));
  }
  else if (youngs_modulus_set && bulk_modulus_set)
  {
    result.lambda = 3.0 * bulk_modulus * (3.0 * bulk_modulus - youngs_modulus) /
                    (9.0 * bulk_modulus - youngs_modulus);
    result.shear_modulus =
        3.0 * bulk_modulus * youngs_modulus / (9.0 * bulk_modulus - youngs_modulus);
    elas_mod = (youngs_modulus);
    poiss_rat = (3 * bulk_modulus - youngs_modulus) / (6 * bulk_modulus);
    result.effective_stiffness =
        std::max(std::sqrt((elas_mod * (1 - poiss_rat)) / ((1 + poiss_rat) * (1 - 2 * poiss_rat))),
                 std::sqrt(elas_mod / (2 * (1 + poiss_rat))));
  }
  else if (lambda_set && poissons_ratio_set)
  {
    result.lambda = lambda;
    result.shear_modulus = lambda * (1.0 - 2.0 * poissons_ratio) / (2.0 * poissons_ratio);
    elas_mod = (lambda * (1 + poissons_ratio) * (1 - 2 * poissons_ratio)) / (poissons_ratio);
    poiss_rat = (poissons_ratio);
    result.effective_stiffness =
        std::max(std::sqrt((elas_mod * (1 - poiss_rat)) / ((1 + poiss_rat) * (1 - 2 * poiss_rat))),
                 std::sqrt(elas_mod / (2 * (1 + poiss_rat))));
  }
  else if (lambda_set && youngs_modulus_set)
  {
    result.lambda = lambda;
    result.shear_modulus = (youngs_modulus - 3.0 * lambda +
                            std::sqrt(youngs_modulus * youngs_modulus + 9.0 * lambda * lambda +
                                      2.0 * youngs_modulus * lambda)) /
                           4.0;
    elas_mod = (youngs_modulus);
    poiss_rat = (2 * lambda) / (youngs_modulus + lambda +
                                std::sqrt(std::pow(youngs_modulus, 2) + 9 * std::pow(lambda, 2) +
                                          2 * youngs_modulus * lambda));
    result.effective_stiffness =
        std::max(std::sqrt((elas_mod * (1 - poiss_rat)) / ((1 + poiss_rat) * (1 - 2 * poiss_rat))),
                 std::sqrt(elas_mod / (2 * (1 + poiss_rat))));
  }
  else
    mooseError("Incorrect combination of isotropic elastic properties.");

  return result;
}

void
toVoigtNotationIndexConversion(int k, int & a, int & b)
{
  if (k < 3 && k >= 0)
    a = b = k;
  else if (k == 3)
  {
    a = 1;
    b = 2;
  }
  else if (k == 4)
  {
    a = 0;
    b = 2;
  }
  else if (k == 5)
  {
    a = 0;
    b = 1;
  }
  else
    mooseError("\nIndex out of bound while converting from tensor to voigt notation in "
               "toVoigtNotationIndexConversion");
}

// MOOSE uses the stress tensor with components ordered as (11, 22, 33, 12, 23, 13) in voigt form.
// This is in conflict with the voigt notation used in literature which has components ordered as
// (11, 22, 33, 23, 13, 12). Whenever an operation involving the voigt forms of stress and
// elasticity tensor has to be performed the voigt form of the elasticity tensor should be built
// taking into account this difference in component-ordering for stress. The
// toMooseVoigtNotationIndexConversion function facilitates that.

void
toMooseVoigtNotationIndexConversion(int k, int & a, int & b)
{
  if (k < 3 && k >= 0)
    a = b = k;
  else if (k == 3)
  {
    a = 0;
    b = 1;
  }
  else if (k == 4)
  {
    a = 1;
    b = 2;
  }
  else if (k == 5)
  {
    a = 0;
    b = 2;
  }
  else
    mooseError("\nIndex out of bound while converting from tensor to MOOSE voigt notation in "
               "toMooseVoigtNotationIndexConversion");
}
}
