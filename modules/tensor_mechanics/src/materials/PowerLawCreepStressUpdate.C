/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "PowerLawCreepStressUpdate.h"

#include "Function.h"

template <>
InputParameters
validParams<PowerLawCreepStressUpdate>()
{
  InputParameters params = validParams<RadialReturnStressUpdate>();
  params.addClassDescription("This class uses the discrete material in a radial return isotropic "
                             "power law creep model.  This class can be used in conjunction with "
                             "other creep and plasticity materials for more complex simulations.");

  // Linear strain hardening parameters
  params.addRequiredParam<Real>("coefficient", "Leading coefficent in power-law equation");
  params.addRequiredParam<Real>("n_exponent", "Exponent on effective stress in power-law equation");
  params.addParam<Real>("m_exponent", 0.0, "Exponent on time in power-law equation");
  params.addRequiredParam<Real>("activation_energy", "Activation energy");
  params.addParam<Real>("gas_constant", 8.3143, "Universal gas constant");
  params.addParam<Real>("start_time", 0.0, "Start time (if not zero)");
  params.addCoupledVar("temperature", 0.0, "Coupled temperature");

  return params;
}

PowerLawCreepStressUpdate::PowerLawCreepStressUpdate(const InputParameters & parameters)
  : RadialReturnStressUpdate(parameters),
    _coefficient(parameters.get<Real>("coefficient")),
    _n_exponent(parameters.get<Real>("n_exponent")),
    _m_exponent(parameters.get<Real>("m_exponent")),
    _activation_energy(parameters.get<Real>("activation_energy")),
    _gas_constant(parameters.get<Real>("gas_constant")),
    _start_time(getParam<Real>("start_time")),
    _shear_modulus(0.0),
    _has_temp(isCoupled("temperature")),
    _temperature(_has_temp ? coupledValue("temperature") : _zero),
    _creep_strain(declareProperty<RankTwoTensor>(_base_name + "creep_strain")),
    _creep_strain_old(declarePropertyOld<RankTwoTensor>(_base_name + "creep_strain"))
{
}

void
PowerLawCreepStressUpdate::initQpStatefulProperties()
{
  _creep_strain[_qp].zero();
  _creep_strain_old[_qp].zero();
}

void PowerLawCreepStressUpdate::computeStressInitialize(Real /*effectiveTrialStress*/)
{
  _shear_modulus = getIsotropicShearModulus();

  if (_has_temp)
    _exponential = std::exp(-_activation_energy / (_gas_constant * _temperature[_qp]));
  else
    _exponential = 1;

  _exp_time = std::pow(_t - _start_time, _m_exponent);

  _creep_strain[_qp] = _creep_strain_old[_qp];
}

Real
PowerLawCreepStressUpdate::computeResidual(Real effectiveTrialStress, Real scalar)
{
  return _coefficient * std::pow(effectiveTrialStress - 3 * _shear_modulus * scalar, _n_exponent) *
             _exponential * _exp_time -
         scalar / _dt;
}

Real
PowerLawCreepStressUpdate::computeDerivative(Real effectiveTrialStress, Real scalar)
{
  return -3 * _coefficient * _shear_modulus * _n_exponent *
             std::pow(effectiveTrialStress - 3 * _shear_modulus * scalar, _n_exponent - 1) *
             _exponential * _exp_time -
         1 / _dt;
}

void
PowerLawCreepStressUpdate::computeStressFinalize(const RankTwoTensor & plasticStrainIncrement)
{
  _creep_strain[_qp] += plasticStrainIncrement;
}
