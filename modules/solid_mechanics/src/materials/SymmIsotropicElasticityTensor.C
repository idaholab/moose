#include "SymmIsotropicElasticityTensor.h"

SymmIsotropicElasticityTensor::SymmIsotropicElasticityTensor(const bool constant)
  : SymmElasticityTensor(constant),
    _lambda_set(false),
    _mu_set(false),
    _nu_set(false),
    _k_set(false),
    _lambda(0),
    _mu(0),
    _nu(0),
    _k(0)
{}

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

void
SymmIsotropicElasticityTensor::calculateLameCoefficients()
{
  if(_lambda_set && _mu_set) // First and second Lame
    return;
  else if(_lambda_set && _nu_set)
    _mu = (_lambda * (1.0 - 2.0 * _nu)) / (2.0 * _nu);
  else if(_lambda_set && _k_set)
    _mu = ( 3.0 * (_k - _lambda) ) / 2.0;
  else if(_lambda_set && _E_set)
    _mu = ( (_E - 3.0*_lambda) / 4.0 ) + ( std::sqrt( (_E-3.0*_lambda)*(_E-3.0*_lambda) + 8.0*_lambda*_E ) / 4.0 );
  else if(_mu_set && _nu_set)
    _lambda = ( 2.0 * _mu * _nu ) / (1.0 - 2.0*_nu);
  else if(_mu_set && _k_set)
    _lambda = ( 3.0 * _k - 2.0 * _mu ) / 3.0;
  else if(_mu_set && _E_set)
    _lambda = ((2.0*_mu - _E) * _mu) / (_E - 3.0*_mu);
  else if(_nu_set && _k_set)
  {
    _lambda = (3.0 * _k * _nu) / (1.0 + _nu);
    _mu = (3.0 * _k * (1.0 - 2.0*_nu)) / (2.0 * (1.0 + _nu));
  }
  else if(_E_set && _nu_set) // Young's Modulus and Poisson's Ratio
  {
    _lambda = (_nu * _E) / ( (1.0+_nu) * (1-2.0*_nu) );
    _mu = _E / ( 2.0 * (1.0+_nu));
  }
  else if(_E_set && _k_set)
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
  const Real C11(2*C44+C12);

  _val[ 0] = _val[ 6] = _val[11] = C11;
  _val[ 1] = _val[ 2] = _val[ 7] = C12;
  _val[15] = _val[18] = _val[20] = C44;
}
