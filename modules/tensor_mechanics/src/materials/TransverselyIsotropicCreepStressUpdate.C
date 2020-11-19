//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TransverselyIsotropicCreepStressUpdate.h"

registerMooseObject("TensorMechanicsApp", TransverselyIsotropicCreepStressUpdate);

InputParameters
TransverselyIsotropicCreepStressUpdate::validParams()
{
  InputParameters params = AnisotropicReturnCreepStressUpdateBase::validParams();
  params.addClassDescription(
      "This class uses the stress update material in a radial return isotropic power law creep "
      "model.  This class can be used in conjunction with other creep and plasticity materials for "
      "more complex simulations.");

  // Linear strain hardening parameters
  params.addCoupledVar("temperature", "Coupled temperature");
  params.addRequiredParam<Real>("coefficient", "Leading coefficient in power-law equation");
  params.addRequiredParam<Real>("n_exponent", "Exponent on effective stress in power-law equation");
  params.addParam<Real>("m_exponent", 0.0, "Exponent on time in power-law equation");
  params.addRequiredParam<Real>("activation_energy", "Activation energy");
  params.addParam<Real>("gas_constant", 8.3143, "Universal gas constant");
  params.addParam<Real>("start_time", 0.0, "Start time (if not zero)");
  params.addRequiredParam<std::vector<Real>>("hill_constants",
                                             "Hill material constants in order: F, "
                                             "G, H, L, M, N");

  return params;
}

TransverselyIsotropicCreepStressUpdate::TransverselyIsotropicCreepStressUpdate(
    const InputParameters & parameters)
  : AnisotropicReturnCreepStressUpdateBase(parameters),
    _has_temp(isParamValid("temperature")),
    _temperature(_has_temp ? coupledValue("temperature") : _zero),
    _coefficient(getParam<Real>("coefficient")),
    _n_exponent(getParam<Real>("n_exponent")),
    _m_exponent(getParam<Real>("m_exponent")),
    _activation_energy(getParam<Real>("activation_energy")),
    _gas_constant(getParam<Real>("gas_constant")),
    _start_time(getParam<Real>("start_time")),
    _exponential(1.0),
    _exp_time(1.0),
    _hill_constants(6),
    _qsigma(0.0)
{
  if (_start_time < _app.getStartTime() && (std::trunc(_m_exponent) != _m_exponent))
    paramError("start_time",
               "Start time must be equal to or greater than the Executioner start_time if a "
               "non-integer m_exponent is used");

  _hill_constants = getParam<std::vector<Real>>("hill_tensors");
}

void
TransverselyIsotropicCreepStressUpdate::computeStressInitialize(
    const DenseVector<Real> & /*effective_trial_stress*/,
    const RankFourTensor & /*elasticity_tensor*/)
{
  if (_has_temp)
    _exponential = std::exp(-_activation_energy / (_gas_constant * _temperature[_qp]));

  _exp_time = std::pow(_t - _start_time, _m_exponent);
}

Real
TransverselyIsotropicCreepStressUpdate::computeResidual(
    const DenseVector<Real> & /*effective_trial_stress*/,
    const DenseVector<Real> & stress_new,
    const Real delta_gamma)
{
  // Hill constants, some constraints apply
  const Real F = _hill_constants[0];
  const Real G = _hill_constants[1];
  const Real H = _hill_constants[2];
  const Real L = _hill_constants[3];
  const Real M = _hill_constants[4];
  const Real N = _hill_constants[5];

  // Need to compute this iteration's stress tensor based on the scalar variable
  // Basically..
  // s(n+1) = {Q [I + 2*nu*delta_gamma*Delta]^(-1) Q^T}  s(trial)

  DenseVector<Real> dev_np1(6);
  DenseMatrix<Real> inv_matrix(6, 6);
  inv_matrix(0, 0) = 1 / (1 + 2.0 * delta_gamma * _eigenvalues_hill(0));
  inv_matrix(1, 1) = 1 / (1 + 2.0 * delta_gamma * _eigenvalues_hill(1));
  inv_matrix(2, 2) = 1 / (1 + 2.0 * delta_gamma * _eigenvalues_hill(2));
  inv_matrix(3, 3) = 1 / (1 + 2.0 * delta_gamma * _eigenvalues_hill(3));
  inv_matrix(4, 4) = 1 / (1 + 2.0 * delta_gamma * _eigenvalues_hill(4));
  inv_matrix(5, 5) = 1 / (1 + 2.0 * delta_gamma * _eigenvalues_hill(5));

  DenseMatrix<Real> transform_to_np1(6, 6);
  DenseMatrix<Real> eigenvectors_hill_transpose(6, 6);
  _eigenvectors_hill.get_transpose(eigenvectors_hill_transpose);
  DenseMatrix<Real> eigenvectors_hill_copy(_eigenvectors_hill);
  inv_matrix.right_multiply(eigenvectors_hill_transpose);
  eigenvectors_hill_copy.right_multiply(inv_matrix);
  transform_to_np1 = eigenvectors_hill_copy;

  DenseVector<Real> stress_np1(6);
  transform_to_np1.vector_mult(stress_np1, stress_new);

  // Equivalent deviatoric stress function.

  Real qsigma_square = F * (stress_new(1) - stress_new(2)) * (stress_new(1) - stress_new(2));
  qsigma_square += G * (stress_new(2) - stress_new(0)) * (stress_new(2) - stress_new(0));
  qsigma_square += H * (stress_new(0) - stress_new(1)) * (stress_new(0) - stress_new(1));
  qsigma_square += 2 * L * stress_new(3) * stress_new(3);
  qsigma_square += 2 * M * stress_new(4) * stress_new(4);
  qsigma_square += 2 * N * stress_new(5) * stress_new(5);

  // moose assert > 0
  qsigma_square = std::sqrt(qsigma_square);
  const Real creep_rate =
      _coefficient * std::pow(qsigma_square, _n_exponent) * _exponential * _exp_time;

  // Return iteration difference between creep strain and inelastic strain multiplier
  return creep_rate * _dt - delta_gamma;
}
Real
TransverselyIsotropicCreepStressUpdate::computeReferenceResidual(
    const DenseVector<Real> & /*effective_trial_stress*/,
    const DenseVector<Real> & /*stress_new*/,
    const Real residual,
    const Real /*scalar_effective_inelastic_strain*/)
{
  // Since here residual is a strain. Let's avoid scaling for now.
  return residual;
}

Real
TransverselyIsotropicCreepStressUpdate::computeDerivative(
    const DenseVector<Real> & /*effective_trial_stress*/,
    const DenseVector<Real> & stress_new,
    const Real delta_gamma)
{
  // Hill constants, some constraints apply
  const Real F = _hill_constants[0];
  const Real G = _hill_constants[1];
  const Real H = _hill_constants[2];
  const Real L = _hill_constants[3];
  const Real M = _hill_constants[4];
  const Real N = _hill_constants[5];

  // Equivalent deviatoric stress function.
  Real qsigma_square = F * (stress_new(1) - stress_new(2)) * (stress_new(1) - stress_new(2));
  qsigma_square += G * (stress_new(2) - stress_new(0)) * (stress_new(2) - stress_new(0));
  qsigma_square += H * (stress_new(0) - stress_new(1)) * (stress_new(0) - stress_new(1));
  qsigma_square += 2 * L * stress_new(3) * stress_new(3);
  qsigma_square += 2 * M * stress_new(4) * stress_new(4);
  qsigma_square += 2 * N * stress_new(5) * stress_new(5);

  // moose assert > 0
  qsigma_square = std::sqrt(qsigma_square);
  _qsigma = qsigma_square;

  const Real creep_rate_derivative = -1.0 * _coefficient * _three_shear_modulus * _n_exponent *
                                     std::pow(qsigma_square, _n_exponent - 1.0) * _exponential *
                                     _exp_time;
  return creep_rate_derivative * _dt - 1.0;
}

void
TransverselyIsotropicCreepStressUpdate::computeStrainFinalize(
    RankTwoTensor & inelasticStrainIncrement, const RankTwoTensor & stress, const Real delta_gamma)
{
  // Hill constants, some constraints apply
  const Real F = _hill_constants[0];
  const Real G = _hill_constants[1];
  const Real H = _hill_constants[2];
  const Real L = _hill_constants[3];
  const Real M = _hill_constants[4];
  const Real N = _hill_constants[5];

  // Moose::out output for q_sigma
  // Abaqus
  if (_qsigma == 0)
  {
    inelasticStrainIncrement(0, 0) = inelasticStrainIncrement(1, 1) =
        inelasticStrainIncrement(2, 2) = inelasticStrainIncrement(0, 1) =
            inelasticStrainIncrement(1, 0) = inelasticStrainIncrement(2, 0) =
                inelasticStrainIncrement(0, 2) = inelasticStrainIncrement(1, 2) =
                    inelasticStrainIncrement(2, 1) = 0.0;
    return;
  }

  const Real prefactor = delta_gamma / _qsigma;

  inelasticStrainIncrement(0, 0) =
      prefactor * (H * (stress(0, 0) - stress(1, 1)) - G * (stress(2, 2) - stress(0, 0)));
  inelasticStrainIncrement(1, 1) =
      prefactor * (F * (stress(1, 1) - stress(2, 2)) - H * (stress(0, 0) - stress(1, 1)));
  inelasticStrainIncrement(2, 2) =
      prefactor * (G * (stress(2, 2) - stress(0, 0)) - F * (stress(1, 1) - stress(2, 2)));

  inelasticStrainIncrement(0, 1) = prefactor * 2.0 * N * stress(0, 1);
  inelasticStrainIncrement(0, 2) = prefactor * 2.0 * M * stress(0, 2);
  inelasticStrainIncrement(1, 2) = prefactor * 2.0 * L * stress(1, 2);
}

Real
TransverselyIsotropicCreepStressUpdate::computeStrainEnergyRateDensity(
    const MaterialProperty<RankTwoTensor> & /*stress*/,
    const MaterialProperty<RankTwoTensor> & /*strain_rate*/)
{
  return 0.0;
}
