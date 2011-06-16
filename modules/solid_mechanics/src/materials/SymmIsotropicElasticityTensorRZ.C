#include "SymmIsotropicElasticityTensorRZ.h"

SymmIsotropicElasticityTensorRZ::SymmIsotropicElasticityTensorRZ(const bool constant)
  : SymmIsotropicElasticityTensor(constant)
{}

void
SymmIsotropicElasticityTensorRZ::calculateLameCoefficients()
{
  SymmIsotropicElasticityTensor::calculateLameCoefficients();

  mooseAssert( _lambda_set && _mu_set, "Both lambda and mu must be set" );

  // Calculate lambda, the shear modulus, and Young's modulus
  if(!_nu_set)
  {
    _nu = _lambda / ( 2 * ( _lambda + _mu ) );
  }
  if(!_E_set)
  {
    _E = _mu*(3*_lambda+2*_mu)/(_lambda+_mu);
  }

  _nu_set = true;
  _E_set = true;
}

void
SymmIsotropicElasticityTensorRZ::calculateEntries(unsigned int /*qp*/)
{

  calculateLameCoefficients();

  const Real C0 = _E * (1 - _nu) / ((1 + _nu)*(1 - 2*_nu));
  const Real C1 = C0;
  const Real C2 = C0 * (_nu / (1 - _nu));
  const Real C3 = C0 * ((1 - 2*_nu) / (2*( 1 - _nu )));

  setEntries( C1, C2, C3 );
}


