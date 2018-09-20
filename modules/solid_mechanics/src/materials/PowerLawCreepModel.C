//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PowerLawCreepModel.h"

#include "SymmIsotropicElasticityTensor.h"

registerMooseObject("SolidMechanicsApp", PowerLawCreepModel);

template <>
InputParameters
validParams<PowerLawCreepModel>()
{
  InputParameters params = validParams<ReturnMappingModel>();

  // Power-law creep material parameters
  params.addRequiredParam<Real>("coefficient", "Leading coefficent in power-law equation");
  params.addRequiredParam<Real>("n_exponent", "Exponent on effective stress in power-law equation");
  params.addParam<Real>("m_exponent", 0.0, "Exponent on time in power-law equation");
  params.addRequiredParam<Real>("activation_energy", "Activation energy");
  params.addParam<Real>("gas_constant", 8.3143, "Universal gas constant");
  params.addParam<Real>("start_time", 0, "Start time (if not zero)");
  params.addCoupledVar("temp", "Coupled Temperature");

  return params;
}

PowerLawCreepModel::PowerLawCreepModel(const InputParameters & parameters)
  : ReturnMappingModel(parameters, "creep"),
    _coefficient(parameters.get<Real>("coefficient")),
    _n_exponent(parameters.get<Real>("n_exponent")),
    _m_exponent(parameters.get<Real>("m_exponent")),
    _activation_energy(parameters.get<Real>("activation_energy")),
    _gas_constant(parameters.get<Real>("gas_constant")),
    _start_time(getParam<Real>("start_time")),
    _creep_strain(declareProperty<SymmTensor>("creep_strain")),
    _creep_strain_old(getMaterialPropertyOld<SymmTensor>("creep_strain"))
{
}

void
PowerLawCreepModel::computeStressInitialize(Real /*effectiveTrialStress*/,
                                            const SymmElasticityTensor & elasticityTensor)
{
  const SymmIsotropicElasticityTensor * eT =
      dynamic_cast<const SymmIsotropicElasticityTensor *>(&elasticityTensor);
  if (!eT)
  {
    mooseError("PowerLawCreepModel requires a SymmIsotropicElasticityTensor");
  }
  _shear_modulus = eT->shearModulus();

  _exponential = 1;
  if (_has_temp)
  {
    _exponential = std::exp(-_activation_energy / (_gas_constant * _temperature[_qp]));
  }

  _expTime = std::pow(_t - _start_time, _m_exponent);

  _creep_strain[_qp] = _creep_strain_old[_qp];
}

void
PowerLawCreepModel::computeStressFinalize(const SymmTensor & plasticStrainIncrement)
{
  _creep_strain[_qp] += plasticStrainIncrement;
}

Real
PowerLawCreepModel::computeResidual(const Real effectiveTrialStress, const Real scalar)
{
  const Real stress_delta = effectiveTrialStress - 3.0 * _shear_modulus * scalar;
  Real creep_rate = _coefficient * std::pow(stress_delta, _n_exponent) * _exponential * _expTime;
  return creep_rate * _dt - scalar;
}

Real
PowerLawCreepModel::computeDerivative(const Real effectiveTrialStress, const Real scalar)
{
  return -3.0 * _coefficient * _shear_modulus * _n_exponent *
             std::pow(effectiveTrialStress - 3.0 * _shear_modulus * scalar, _n_exponent - 1.0) *
             _exponential * _expTime * _dt -
         1.0;
}
