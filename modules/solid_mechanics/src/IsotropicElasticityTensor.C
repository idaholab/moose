#include "IsotropicElasticityTensor.h"

IsotropicElasticityTensor::IsotropicElasticityTensor(const bool constant)
  : ElasticityTensor(constant),
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
IsotropicElasticityTensor::setLambda(const Real lambda)
{
  _lambda = lambda;
  _lambda_set = true;
}

void
IsotropicElasticityTensor::setMu(const Real mu)
{
  _mu = mu;
  _mu_set = true;
}

void
IsotropicElasticityTensor::setYoungsModulus(const Real E)
{
  _E = E;
  _E_set = true;
}

void
IsotropicElasticityTensor::setPoissonsRatio(const Real nu)
{
  _nu = nu;
  _nu_set = true;
}

void
IsotropicElasticityTensor::setBulkModulus(const Real k)
{
  _k = k;
  _k_set = true;
}

void
IsotropicElasticityTensor::setShearModulus(const Real k)
{
  setMu(k);
}

void
IsotropicElasticityTensor::calculateLameCoefficients()
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
}

Real
IsotropicElasticityTensor::isotropicEntry(const unsigned int i, const unsigned j, const unsigned k, const unsigned l)
{
  return _lambda*(i==j)*(k==l) + _mu*((i==k)*(j==l)+(i==l)*(j==k));
}

void
IsotropicElasticityTensor::calculateEntries(unsigned int qp)
{  
  calculateLameCoefficients();

  unsigned int i, j, k, l;
  i = j = k = l = 0;

  // Walk down the columns of the 9x9 matrix
  for (unsigned int q=0; q<81; ++q)
  {
    // This algorithm was developed by Derek Gaston and Cody Permann
    // it's based on page 29 of Michael Tonk's notes
    j += i/3;
    k += j/3;
    l += k/3;

    i %= 3;
    j %= 3;
    k %= 3;
    l %= 3;

    _values[q] = isotropicEntry(i, j, k, l);

    ++i;
  }
}


