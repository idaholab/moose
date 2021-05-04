//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADHillPlasticityStressUpdate.h"
#include "ElasticityTensorTools.h"

registerMooseObject("TensorMechanicsApp", ADHillPlasticityStressUpdate);

InputParameters
ADHillPlasticityStressUpdate::validParams()
{
  InputParameters params = ADAnisotropicReturnPlasticityStressUpdateBase::validParams();
  params.addClassDescription(
      "This class uses the stress update material in a radial return isotropic power law creep "
      "model.  This class can be used in conjunction with other creep and plasticity materials for "
      "more complex simulations.");

  params.addRequiredParam<Real>("hardening_constant",
                                "Hardening constant (H) for anisotropic plasticity");
  params.addRequiredParam<Real>("yield_stress",
                                "Yield stress (constant value) for anisotropic plasticity");

  params.addRequiredRangeCheckedParam<std::vector<Real>>("hill_constants",
                                                         "hill_constants_size = 6",
                                                         "Hill material constants in order: F, "
                                                         "G, H, L, M, N");

  return params;
}

ADHillPlasticityStressUpdate::ADHillPlasticityStressUpdate(const InputParameters & parameters)
  : ADAnisotropicReturnPlasticityStressUpdateBase(parameters),
    _hill_constants(6),
    _qsigma(0.0),
    _eigenvalues_hill(6),
    _eigenvectors_hill(6, 6),
    _hardening_constant(getParam<Real>("hardening_constant")),
    _hardening_function(isParamValid("hardening_function") ? &getFunction("hardening_function")
                                                           : nullptr),
    _hardening_variable(declareADProperty<Real>(_base_name + "hardening_variable")),
    _hardening_variable_old(getMaterialPropertyOld<Real>(_base_name + "hardening_variable")),
    _hardening_slope(0.0),
    _yield_condition(1.0),
    _yield_stress(getParam<Real>("yield_stress")),
    _hill_tensor(6, 6),
    _stress_np1(6)

{
  _hill_constants = getParam<std::vector<Real>>("hill_constants");

  // Hill constants, some constraints apply
  const Real & F = _hill_constants[0];
  const Real & G = _hill_constants[1];
  const Real & H = _hill_constants[2];
  const Real & L = _hill_constants[3];
  const Real & M = _hill_constants[4];
  const Real & N = _hill_constants[5];

  _hill_tensor.zero();

  _hill_tensor(0, 0) = G + H;
  _hill_tensor(1, 1) = F + H;
  _hill_tensor(2, 2) = F + G;
  _hill_tensor(0, 1) = _hill_tensor(1, 0) = -H;
  _hill_tensor(0, 2) = _hill_tensor(2, 0) = -G;
  _hill_tensor(1, 2) = _hill_tensor(2, 1) = -F;

  _hill_tensor(3, 3) = 2.0 * N;
  _hill_tensor(4, 4) = 2.0 * L;
  _hill_tensor(5, 5) = 2.0 * M;

  computeHillTensorEigenDecomposition(_hill_tensor);
}

void
ADHillPlasticityStressUpdate::propagateQpStatefulProperties()
{
  _hardening_variable[_qp] = _hardening_variable_old[_qp];
  _plasticity_strain[_qp] = _plasticity_strain_old[_qp];
  ADAnisotropicReturnPlasticityStressUpdateBase::propagateQpStatefulProperties();
}

void
ADHillPlasticityStressUpdate::computeStressInitialize(const ADDenseVector & stress_dev,
                                                      const ADDenseVector & /*stress*/,
                                                      const ADRankFourTensor & elasticity_tensor)
{
  _hardening_variable[_qp] = _hardening_variable_old[_qp];
  _plasticity_strain[_qp] = _plasticity_strain_old[_qp];
  _effective_inelastic_strain[_qp] = _effective_inelastic_strain_old[_qp];

  _two_shear_modulus = 2.0 * ElasticityTensorTools::getIsotropicShearModulus(elasticity_tensor);

  _yield_condition = 1.0; // Some positive value
  _yield_condition = -computeResidual(stress_dev, stress_dev, 0.0);
}

ADReal
ADHillPlasticityStressUpdate::computeOmega(const ADReal & delta_gamma,
                                           const ADDenseVector & stress_trial)
{
  ADDenseVector K(6);
  ADReal omega = 0.0;

  for (unsigned int i = 0; i < 6; i++)
  {
    K(i) = _eigenvalues_hill(i) /
           (Utility::pow<2>(1 + _two_shear_modulus * delta_gamma * _eigenvalues_hill(i)));
    omega += K(i) * stress_trial(i) * stress_trial(i);
  }
  omega *= 0.5;

  if (omega == 0.0)
    omega = 1.0e-36;

  return std::sqrt(omega);
}

void
ADHillPlasticityStressUpdate::computeDeltaDerivatives(const ADReal & delta_gamma,
                                                      const ADDenseVector & stress_trial,
                                                      const ADReal & sy_alpha,
                                                      ADReal & omega,
                                                      ADReal & omega_gamma,
                                                      ADReal & sy_gamma)
{
  omega_gamma = 0.0;
  sy_gamma = 0.0;

  ADDenseVector K_deltaGamma(6);
  omega = computeOmega(delta_gamma, stress_trial);

  ADDenseVector K(6);
  for (unsigned int i = 0; i < 6; i++)
    K(i) = _eigenvalues_hill(i) /
           (Utility::pow<2>(1 + _two_shear_modulus * delta_gamma * _eigenvalues_hill(i)));

  for (unsigned int i = 0; i < 6; i++)
    K_deltaGamma(i) = -2.0 * _two_shear_modulus * _eigenvalues_hill(i) * K(i) /
                      (1 + _two_shear_modulus * delta_gamma * _eigenvalues_hill(i));

  for (unsigned int i = 0; i < 6; i++)
    omega_gamma += K_deltaGamma(i) * stress_trial(i) * stress_trial(i);

  omega_gamma /= 4.0 * omega;
  sy_gamma = 2.0 * sy_alpha * (omega + delta_gamma * omega_gamma);
}

Real
ADHillPlasticityStressUpdate::computeReferenceResidual(
    const ADDenseVector & /*effective_trial_stress*/,
    const ADDenseVector & /*stress_new*/,
    const ADReal & /*residual*/,
    const ADReal & /*scalar_effective_inelastic_strain*/)
{
  // Equation is already normalized.
  return 1.0;
}

ADReal
ADHillPlasticityStressUpdate::computeResidual(const ADDenseVector & stress_dev,
                                              const ADDenseVector & /*stress_sigma*/,
                                              const ADReal & delta_gamma)
{

  // If in elastic regime, just return
  if (_yield_condition <= 0.0)
    return 0.0;

  ADDenseMatrix eigenvectors_hill_transpose(6, 6);

  _eigenvectors_hill.get_transpose(eigenvectors_hill_transpose);
  eigenvectors_hill_transpose.vector_mult(_stress_np1, stress_dev);

  ADReal omega = computeOmega(delta_gamma, _stress_np1);
  _hardening_slope = computeHardeningDerivative();

  // Hardening variable is \alpha isotropic hardening for now.
  _hardening_variable[_qp] = computeHardeningValue(delta_gamma, omega);
  ADReal s_y = _hardening_slope * _hardening_variable[_qp] + _yield_stress;

  ADReal residual = 0.0;
  residual = s_y / omega - 1.0;

  return residual;
}

ADReal
ADHillPlasticityStressUpdate::computeDerivative(const ADDenseVector & /*stress_dev*/,
                                                const ADDenseVector & /*stress_sigma*/,
                                                const ADReal & delta_gamma)
{
  // If in elastic regime, return unit derivative
  if (_yield_condition <= 0.0)
    return 1.0;

  ADReal omega = computeOmega(delta_gamma, _stress_np1);
  _hardening_slope = computeHardeningDerivative();

  ADReal sy = _hardening_slope * computeHardeningValue(delta_gamma, omega) + _yield_stress;
  ADReal sy_alpha = _hardening_slope;

  ADReal omega_gamma;
  ADReal sy_gamma;

  computeDeltaDerivatives(delta_gamma, _stress_np1, sy_alpha, omega, omega_gamma, sy_gamma);
  ADReal residual_derivative = 1 / omega * (sy_gamma - 1 / omega * omega_gamma * sy);

  return residual_derivative;
}

void
ADHillPlasticityStressUpdate::computeHillTensorEigenDecomposition(ADDenseMatrix & hill_tensor)
{
  const unsigned int dimension = hill_tensor.n();

  AnisotropyMatrixReal A;
  for (unsigned int index_i = 0; index_i < dimension; index_i++)
    for (unsigned int index_j = 0; index_j < dimension; index_j++)
      A(index_i, index_j) = MetaPhysicL::raw_value(hill_tensor(index_i, index_j));

  Eigen::SelfAdjointEigenSolver<AnisotropyMatrixReal> es(A);

  auto lambda = es.eigenvalues();
  auto v = es.eigenvectors();
  for (unsigned int index_i = 0; index_i < dimension; index_i++)
    _eigenvalues_hill(index_i) = lambda(index_i);

  for (unsigned int index_i = 0; index_i < dimension; index_i++)
    for (unsigned int index_j = 0; index_j < dimension; index_j++)
      _eigenvectors_hill(index_i, index_j) = v(index_i, index_j);
}

ADReal
ADHillPlasticityStressUpdate::computeHardeningValue(const ADReal & delta_gamma,
                                                    const ADReal & omega)
{
  return _hardening_variable_old[_qp] + 2.0 * delta_gamma * omega;
}

ADReal
ADHillPlasticityStressUpdate::computeHardeningDerivative()
{
  return _hardening_constant;
}

void
ADHillPlasticityStressUpdate::computeStrainFinalize(ADRankTwoTensor & inelasticStrainIncrement,
                                                    const ADRankTwoTensor & stress,
                                                    const ADDenseVector & stress_dev,
                                                    const ADReal & delta_gamma)
{
  // e^P = delta_gamma * hill_tensor * stress
  ADDenseVector inelasticStrainIncrement_vector(6);
  ADDenseVector hill_stress(6);
  _hill_tensor.vector_mult(hill_stress, stress_dev);
  hill_stress.scale(delta_gamma);
  inelasticStrainIncrement_vector = hill_stress;

  inelasticStrainIncrement(0, 0) = inelasticStrainIncrement_vector(0);
  inelasticStrainIncrement(1, 1) = inelasticStrainIncrement_vector(1);
  inelasticStrainIncrement(2, 2) = inelasticStrainIncrement_vector(2);
  inelasticStrainIncrement(0, 1) = inelasticStrainIncrement(1, 0) =
      inelasticStrainIncrement_vector(3) / 2.0;
  inelasticStrainIncrement(1, 2) = inelasticStrainIncrement(2, 1) =
      inelasticStrainIncrement_vector(4) / 2.0;
  inelasticStrainIncrement(0, 2) = inelasticStrainIncrement(2, 0) =
      inelasticStrainIncrement_vector(5) / 2.0;

  // Calculate appropriate equivalent plastic strain
  const Real & F = _hill_constants[0];
  const Real & G = _hill_constants[1];
  const Real & H = _hill_constants[2];
  const Real & L = _hill_constants[3];
  const Real & M = _hill_constants[4];
  const Real & N = _hill_constants[5];

  ADReal eq_plastic_strain_inc = (F * Utility::pow<2>(inelasticStrainIncrement(0, 0)) +
                                  G * Utility::pow<2>(inelasticStrainIncrement(1, 1)) +
                                  H * Utility::pow<2>(inelasticStrainIncrement(2, 2))) /
                                     (F * G + F * H + G * H) +
                                 2.0 * Utility::pow<2>(inelasticStrainIncrement(1, 2)) / L +
                                 2.0 * Utility::pow<2>(inelasticStrainIncrement(2, 0)) / M +
                                 2.0 * Utility::pow<2>(inelasticStrainIncrement(0, 1)) / N;

  if (eq_plastic_strain_inc > 0.0)
    eq_plastic_strain_inc = std::sqrt(eq_plastic_strain_inc);

  _effective_inelastic_strain[_qp] = _effective_inelastic_strain_old[_qp] + eq_plastic_strain_inc;

  ADAnisotropicReturnPlasticityStressUpdateBase::computeStrainFinalize(
      inelasticStrainIncrement, stress, stress_dev, delta_gamma);
}

void
ADHillPlasticityStressUpdate::computeStressFinalize(
    const ADRankTwoTensor & /*plastic_strain_increment*/,
    const ADReal & delta_gamma,
    ADRankTwoTensor & stress_new,
    const ADDenseVector & stress_dev,
    const ADRankTwoTensor & /*sstress_old*/,
    const ADRankFourTensor & /*elasticity_tensor*/)
{
  // Need to compute this iteration's stress tensor based on the scalar variable
  // For deviatoric
  // s(n+1) = {Q [I + 2*nu*delta_gamma*Delta]^(-1) Q^T}  s(trial)

  if (_yield_condition <= 0.0)
    return;

  ADDenseMatrix inv_matrix(6, 6);
  for (unsigned int i = 0; i < 6; i++)
    inv_matrix(i, i) = 1 / (1 + _two_shear_modulus * delta_gamma * _eigenvalues_hill(i));

  ADDenseMatrix eigenvectors_hill_transpose(6, 6);

  _eigenvectors_hill.get_transpose(eigenvectors_hill_transpose);
  ADDenseMatrix eigenvectors_hill_copy(_eigenvectors_hill);

  // Right multiply by matrix of eigenvectors transpose
  inv_matrix.right_multiply(eigenvectors_hill_transpose);
  // Right multiply eigenvector matrix by [I + 2*nu*delta_gamma*Delta]^(-1) Q^T
  eigenvectors_hill_copy.right_multiply(inv_matrix);

  ADDenseVector stress_np1(6);
  eigenvectors_hill_copy.vector_mult(stress_np1, stress_dev);

  ADRankTwoTensor stress_new_volumetric = stress_new - stress_new.deviatoric();

  stress_new(0, 0) = stress_new_volumetric(0, 0) + stress_np1(0);
  stress_new(1, 1) = stress_new_volumetric(1, 1) + stress_np1(1);
  stress_new(2, 2) = stress_new_volumetric(2, 2) + stress_np1(2);
  stress_new(0, 1) = stress_new(1, 0) = stress_np1(3);
  stress_new(1, 2) = stress_new(2, 1) = stress_np1(4);
  stress_new(0, 2) = stress_new(2, 0) = stress_np1(5);

  ADReal omega = computeOmega(delta_gamma, _stress_np1);
  _hardening_variable[_qp] = computeHardeningValue(delta_gamma, omega);
}
