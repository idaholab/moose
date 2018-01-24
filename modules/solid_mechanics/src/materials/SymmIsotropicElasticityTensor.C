/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "SymmIsotropicElasticityTensor.h"

SymmIsotropicElasticityTensor::SymmIsotropicElasticityTensor(const bool constant)
  : SymmElasticityTensor(constant),
    _lambda_set(false),
    _mu_set(false),
    _E_set(false),
    _nu_set(false),
    _k_set(false),
    _lambda(0),
    _mu(0),
    _E(0),
    _nu(0),
    _k(0)
{
}

void
SymmIsotropicElasticityTensor::setLambda(const Real lambda)
{
  _lambda = lambda;
  _lambda_set = true;
}

void
SymmIsotropicElasticityTensor::setMu(const Real mu)
{
  _mu = mu;
  _mu_set = true;
}

void
SymmIsotropicElasticityTensor::setYoungsModulus(const Real E)
{
  _E = E;
  _E_set = true;
}

void
SymmIsotropicElasticityTensor::setPoissonsRatio(const Real nu)
{
  _nu = nu;
  _nu_set = true;
}

void
SymmIsotropicElasticityTensor::setBulkModulus(const Real k)
{
  _k = k;
  _k_set = true;
}

void
SymmIsotropicElasticityTensor::setShearModulus(const Real k)
{
  setMu(k);
}

Real
SymmIsotropicElasticityTensor::shearModulus() const
{
  return mu();
}

Real
SymmIsotropicElasticityTensor::youngsModulus() const
{
  if (!_E_set)
    mooseError("Youngs modulus not set");

  return _E;
}

Real
SymmIsotropicElasticityTensor::mu() const
{
  if (!_mu_set)
  {
    mooseError("mu not set");
  }
  return _mu;
}

void
SymmIsotropicElasticityTensor::calculateLameCoefficients()
{
  if (_lambda_set && _mu_set) // First and second Lame
    return;
  else if (_lambda_set && _nu_set)
    _mu = (_lambda * (1.0 - 2.0 * _nu)) / (2.0 * _nu);
  else if (_lambda_set && _k_set)
    _mu = (3.0 * (_k - _lambda)) / 2.0;
  else if (_lambda_set && _E_set)
    _mu = ((_E - 3.0 * _lambda) / 4.0) +
          (std::sqrt((_E - 3.0 * _lambda) * (_E - 3.0 * _lambda) + 8.0 * _lambda * _E) / 4.0);
  else if (_mu_set && _nu_set)
    _lambda = (2.0 * _mu * _nu) / (1.0 - 2.0 * _nu);
  else if (_mu_set && _k_set)
    _lambda = (3.0 * _k - 2.0 * _mu) / 3.0;
  else if (_mu_set && _E_set)
    _lambda = ((2.0 * _mu - _E) * _mu) / (_E - 3.0 * _mu);
  else if (_nu_set && _k_set)
  {
    _lambda = (3.0 * _k * _nu) / (1.0 + _nu);
    _mu = (3.0 * _k * (1.0 - 2.0 * _nu)) / (2.0 * (1.0 + _nu));
  }
  else if (_E_set && _nu_set) // Young's Modulus and Poisson's Ratio
  {
    _lambda = (_nu * _E) / ((1.0 + _nu) * (1 - 2.0 * _nu));
    _mu = _E / (2.0 * (1.0 + _nu));
  }
  else if (_E_set && _k_set)
  {
    _lambda = (3.0 * _k * (3.0 * _k - _E)) / (9.0 * _k - _E);
    _mu = (3.0 * _E * _k) / (9.0 * _k - _E);
  }
  _lambda_set = true;
  _mu_set = true;
}

void
SymmIsotropicElasticityTensor::calculateEntries(unsigned int /*qp*/)
{
  calculateLameCoefficients();

  const Real C12(_lambda);
  const Real C44(_mu);
  const Real C11(2 * C44 + C12);

  setEntries(C11, C12, C44);
}

void
SymmIsotropicElasticityTensor::setEntries(Real C11, Real C12, Real C44)
{
  _val[0] = _val[6] = _val[11] = C11;
  _val[1] = _val[2] = _val[7] = C12;
  _val[15] = _val[18] = _val[20] = C44;
  _val[3] = _val[4] = _val[5] = 0;
  _val[8] = _val[9] = _val[10] = 0;
  _val[12] = _val[13] = _val[14] = 0;
  _val[16] = _val[17] = 0;
  _val[19] = 0;
}

Real
SymmIsotropicElasticityTensor::stiffness(const unsigned int i,
                                         const unsigned int j,
                                         const RealGradient & test,
                                         const RealGradient & phi) const
{
  RealGradient b;
  if (0 == i && 0 == j)
  {
    b(0) = _val[0] * phi(0);
    b(1) = _val[15] * phi(1);
    b(2) = _val[20] * phi(2);
  }
  else if (1 == i && 1 == j)
  {
    b(0) = _val[15] * phi(0);
    b(1) = _val[6] * phi(1);
    b(2) = _val[18] * phi(2);
  }
  else if (2 == i && 2 == j)
  {
    b(0) = _val[20] * phi(0);
    b(1) = _val[18] * phi(1);
    b(2) = _val[11] * phi(2);
  }
  else if (0 == i && 1 == j)
  {
    b(0) = _val[1] * phi(1);
    b(1) = _val[15] * phi(0);
    b(2) = 0;
  }
  else if (1 == i && 0 == j)
  {
    b(0) = _val[15] * phi(1);
    b(1) = _val[1] * phi(0);
    b(2) = 0;
  }
  else if (1 == i && 2 == j)
  {
    b(0) = 0;
    b(1) = _val[7] * phi(2);
    b(2) = _val[18] * phi(1);
  }
  else if (2 == i && 1 == j)
  {
    b(0) = 0;
    b(1) = _val[18] * phi(2);
    b(2) = _val[7] * phi(1);
  }
  else if (0 == i && 2 == j)
  {
    b(0) = _val[2] * phi(2);
    b(1) = 0;
    b(2) = _val[20] * phi(0);
  }
  else if (2 == i && 0 == j)
  {
    b(0) = _val[20] * phi(2);
    b(1) = 0;
    b(2) = _val[2] * phi(0);
  }
  else
  {
    mooseError("Wrong index in stiffness calculation");
  }
  return test * b;
}

void
SymmIsotropicElasticityTensor::multiply(const SymmTensor & x, SymmTensor & b) const
{
  const Real xx = x.xx();
  const Real yy = x.yy();
  const Real zz = x.zz();
  const Real xy = x.xy();
  const Real yz = x.yz();
  const Real zx = x.zx();

  b.xx() = _val[0] * xx + _val[1] * yy + _val[2] * zz;
  b.yy() = _val[1] * xx + _val[6] * yy + _val[7] * zz;
  b.zz() = _val[2] * xx + _val[7] * yy + _val[11] * zz;
  b.xy() = 2 * _val[15] * xy;
  b.yz() = 2 * _val[18] * yz;
  b.zx() = 2 * _val[20] * zx;

  b.xx() += 2 * (_val[3] * xy + _val[4] * yz + _val[5] * zx);
  b.yy() += 2 * (_val[8] * xy + _val[9] * yz + _val[10] * zx);
  b.zz() += 2 * (_val[12] * xy + _val[13] * yz + _val[14] * zx);
  b.xy() += _val[3] * xx + _val[8] * yy + _val[12] * zz;
  b.yz() += _val[4] * xx + _val[9] * yy + _val[13] * zz;
  b.zx() += _val[5] * xx + _val[10] * yy + _val[14] * zz;
  b.yz() += 2 * _val[16] * xy;
  b.zx() += 2 * _val[17] * xy + 2 * _val[19] * yz;
}

void
SymmIsotropicElasticityTensor::adjustForCracking(const RealVectorValue & crack_flags)
{
  const RealVectorValue & c(crack_flags);
  const Real c0(c(0));
  const Real c0_coupled(c0 < 1 ? 0 : 1);
  const Real c1(c(1));
  const Real c1_coupled(c1 < 1 ? 0 : 1);
  const Real c2(c(2));
  const Real c2_coupled(c2 < 1 ? 0 : 1);

  const Real c01(c0_coupled * c1_coupled);
  const Real c02(c0_coupled * c2_coupled);
  const Real c12(c1_coupled * c2_coupled);
  const Real c012(c0_coupled * c12);

  const Real ym = _mu * (3 * _lambda + 2 * _mu) / (_lambda + _mu);

  // Assume Poisson's ratio goes to zero for the cracked direction.

  _val[0] = (c0 < 1 ? c0 * ym : _val[0]);
  _val[1] *= c01;
  _val[2] *= c02;
  _val[3] *= c01;
  _val[4] *= c012;
  _val[5] *= c02;

  _val[6] = (c1 < 1 ? c1 * ym : _val[6]);
  _val[7] *= c12;
  _val[8] *= c01;
  _val[9] *= c12;
  _val[10] *= c012;

  _val[11] = (c2 < 1 ? c2 * ym : _val[11]);
  _val[12] *= c012;
  _val[13] *= c12;
  _val[14] *= c02;
}

void
SymmIsotropicElasticityTensor::adjustForCrackingWithShearRetention(
    const RealVectorValue & crack_flags)
{
  const RealVectorValue & c = crack_flags;
  const Real c0 = c(0);
  const Real c0_coupled = (c0 < 1 ? 0 : 1);
  const Real c1 = c(1);
  const Real c1_coupled = (c1 < 1 ? 0 : 1);
  const Real c2 = c(2);
  const Real c2_coupled = (c2 < 1 ? 0 : 1);
  const Real c01 = c0_coupled * c1_coupled;
  const Real c02 = c0_coupled * c2_coupled;
  const Real c12 = c1_coupled * c2_coupled;
  const Real c012 = c0_coupled * c12;
  adjustForCracking(crack_flags);
  _val[15] *= c01;
  _val[16] *= c012;
  _val[17] *= c012;
  _val[18] *= c12;
  _val[19] *= c012;
  _val[20] *= c02;
}
