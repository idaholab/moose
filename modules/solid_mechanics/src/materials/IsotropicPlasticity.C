#include "IsotropicPlasticity.h"

#include "SymmIsotropicElasticityTensor.h"

template<>
InputParameters validParams<IsotropicPlasticity>()
{
  InputParameters params = validParams<ReturnMappingModel>();

  // Linear strain hardening parameters
  params.addRequiredParam<Real>("yield_stress", "The point at which plastic strain begins accumulating");
  params.addRequiredParam<Real>("hardening_constant", "Hardening slope");

  return params;
}


IsotropicPlasticity::IsotropicPlasticity( const std::string & name,
                                      InputParameters parameters )
  :ReturnMappingModel( name, parameters ),
   _yield_stress(parameters.get<Real>("yield_stress")),
   _hardening_constant(parameters.get<Real>("hardening_constant")),

   _plastic_strain(declareProperty<SymmTensor>("plastic_strain")),
   _plastic_strain_old(declarePropertyOld<SymmTensor>("plastic_strain")),

   _hardening_variable(declareProperty<Real>("hardening_variable")),
   _hardening_variable_old(declarePropertyOld<Real>("hardening_variable"))
{
  if (_yield_stress <= 0)
  {
    mooseError("Yield stress must be greater than zero");
  }
}

void
IsotropicPlasticity::initStatefulProperties(unsigned n_points)
{
  for (unsigned qp(0); qp < n_points; ++qp)
  {
    _hardening_variable[qp] = _hardening_variable_old[qp] = 0;
  }
  ReturnMappingModel::initStatefulProperties( n_points );
}

void
IsotropicPlasticity::computeStressInitialize(unsigned qp, Real effectiveTrialStress, const SymmElasticityTensor & elasticityTensor)
{
  const SymmIsotropicElasticityTensor * eT = dynamic_cast<const SymmIsotropicElasticityTensor*>(&elasticityTensor);
  if (!eT)
  {
    mooseError("IsotropicPlasticity requires a SymmIsotropicElasticityTensor");
  }
  _shear_modulus = eT->shearModulus();
  _yield_condition = effectiveTrialStress - _hardening_variable_old[qp] - _yield_stress;
  _hardening_variable[qp] = _hardening_variable_old[qp];
  _plastic_strain[qp] = _plastic_strain_old[qp];
}

void
IsotropicPlasticity::computeStressFinalize(unsigned qp, const SymmTensor & plasticStrainIncrement)
{
  _plastic_strain[qp] += plasticStrainIncrement;
}

Real
IsotropicPlasticity::computeResidual(unsigned qp, Real effectiveTrialStress, Real scalar)
{
  Real residual(0);
  if (_yield_condition > 0)
  {
    residual = effectiveTrialStress - (3. * _shear_modulus * scalar) - _hardening_variable[qp] - _yield_stress;
    _hardening_variable[qp] = _hardening_variable_old[qp] + (_hardening_constant * scalar);
  }
  return residual;
}

Real
IsotropicPlasticity::computeDerivative(unsigned /*qp*/, Real /*effectiveTrialStress*/, Real /*scalar*/)
{
  Real derivative(1);
  if (_yield_condition > 0)
  {
    derivative = -3 * _shear_modulus - _hardening_constant;
  }
  return derivative;
}

void
IsotropicPlasticity::iterationFinalize(unsigned qp, Real scalar)
{
  _hardening_variable[qp] = _hardening_variable_old[qp] + (_hardening_constant * scalar);
}
