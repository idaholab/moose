/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "ElasticityTensorR4.h"

// MOOSE includes
#include "PermutationTensor.h"

template <>
void
mooseSetToZero<ElasticityTensorR4>(ElasticityTensorR4 & v)
{
  v.zero();
}

template <>
void
dataStore(std::ostream & stream, ElasticityTensorR4 & et, void * context)
{
  dataStore(stream, et._vals, context);
}

template <>
void
dataLoad(std::istream & stream, ElasticityTensorR4 & et, void * context)
{
  dataLoad(stream, et._vals, context);
}

ElasticityTensorR4::ElasticityTensorR4(const RankFourTensor & a) : RankFourTensor(a)
{
  mooseDeprecated("Use RankFourTensor instead of ElasticityTensorR4. See "
                  "http://mooseframework.org/wiki/PhysicsModules/TensorMechanics/Deprecations/"
                  "ElasticityTensorR4/ to update your code");
}

Real
ElasticityTensorR4::elasticJacobian(const unsigned int i,
                                    const unsigned int k,
                                    const RealGradient & grad_test,
                                    const RealGradient & grad_phi) const
{
  // d(stress_ij*d(test)/dx_j)/du_k = d(C_ijmn*du_m/dx_n dtest/dx_j)/du_k (which is nonzero for m=k)
  Real the_sum = 0.0;
  const unsigned int ik1 = i * N * N * N + k * N;
  for (unsigned int j = 0; j < N; ++j)
  {
    const unsigned int j1 = j * N * N;
    for (unsigned int l = 0; l < N; ++l)
      the_sum += _vals[ik1 + j1 + l] * grad_phi(l) * grad_test(j);
  }
  return the_sum;
}

Real
ElasticityTensorR4::elasticJacobianwc(const unsigned int i,
                                      const unsigned int k,
                                      const RealGradient & grad_test,
                                      Real phi) const
{
  // d(stress_ij*d(test)/dx_j)/dw_k = d(C_ijmn*eps_mnp*w_p*dtest/dx_j)/dw_k (only nonzero for p=k)
  Real the_sum = 0.0;
  unsigned int ij1 = i * N * N * N;
  for (unsigned int j = 0; j < N; ++j)
  {
    for (unsigned int m = 0; m < N; ++m)
      for (unsigned int n = 0; n < N; ++n)
        the_sum += _vals[ij1 + m * N + n] * PermutationTensor::eps(m, n, k) * grad_test(j);
    ij1 += N * N;
  }
  return the_sum * phi;
}

Real
ElasticityTensorR4::momentJacobian(const unsigned int i,
                                   const unsigned int k,
                                   Real test,
                                   const RealGradient & grad_phi) const
{
  // Jacobian entry: d(eps_ijm*stress_jm*test)/du_k = d(eps_ijm*C_jmln*du_l/dx_n*test)/du_k (only
  // nonzero for l=k)
  Real the_sum = 0.0;
  for (unsigned int j = 0; j < N; ++j)
    for (unsigned int m = 0; m < N; ++m)
      for (unsigned int n = 0; n < N; ++n)
        the_sum += PermutationTensor::eps(i, j, m) * (*this)(j, m, k, n) * grad_phi(n);
  return test * the_sum;
}

Real
ElasticityTensorR4::momentJacobianwc(const unsigned int i,
                                     const unsigned int k,
                                     Real test,
                                     Real phi) const
{
  // Jacobian entry: d(eps_ijm*stress_jm*test)/dw_k = d(eps_ijm*C_jmln*eps_lnp*w_p*test)/dw_k (only
  // nonzero for p=k)
  Real the_sum = 0.0;
  for (unsigned int j = 0; j < N; ++j)
    for (unsigned int l = 0; l < N; ++l)
      for (unsigned int m = 0; m < N; ++m)
        for (unsigned int n = 0; n < N; ++n)
          the_sum += PermutationTensor::eps(i, j, m) * (*this)(j, m, l, n) *
                     PermutationTensor::eps(l, n, k);
  return test * phi * the_sum;
}
