#include "IsotropicElasticityTensorRZ.h"

IsotropicElasticityTensorRZ::IsotropicElasticityTensorRZ(const bool constant)
  : IsotropicElasticityTensor(constant)
{}

void
IsotropicElasticityTensorRZ::calculateLameCoefficients()
{
  IsotropicElasticityTensor::calculateLameCoefficients();

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
IsotropicElasticityTensorRZ::calculateEntries(unsigned int /*qp*/)
{

  calculateLameCoefficients();

  for (unsigned int q=0; q<81; ++q)
  {
    _values[q] = 0;
  }

  const Real C0 = _E * (1 - _nu) / ((1 + _nu)*(1 - 2*_nu));
  const Real C1 = 1;
  const Real C2 = _nu / (1 - _nu);
  const Real C3 = (1 - 2*_nu) / ( 2 * (1 - _nu) );

  const int row(9);

  _values[0*row+0] = C0 * C1;
  _values[0*row+4] = C0 * C2;
  _values[0*row+8] = C0 * C2;

  _values[1*row+1] = C0 * C3;
  _values[1*row+3] = C0 * C3;

  _values[3*row+1] = C0 * C3;
  _values[3*row+3] = C0 * C3;

  _values[4*row+0] = C0 * C2;
  _values[4*row+4] = C0 * C1;
  _values[4*row+8] = C0 * C2;

  _values[8*row+0] = C0 * C2;
  _values[8*row+4] = C0 * C2;
  _values[8*row+8] = C0 * C1;

}


