//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADHillElastoPlasticityStressUpdate.h"

registerMooseObject("TensorMechanicsApp", ADHillElastoPlasticityStressUpdate);

InputParameters
ADHillElastoPlasticityStressUpdate::validParams()
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

ADHillElastoPlasticityStressUpdate::ADHillElastoPlasticityStressUpdate(
    const InputParameters & parameters)
  : ADAnisotropicReturnPlasticityStressUpdateBase(parameters),
    _hill_constants(6),
    _qsigma(0.0),
    _eigenvalues_hill(6),
    _eigenvectors_hill(6, 6),
    _anisotropic_elastic_tensor(6, 6),
    _elasticity_tensor_name(_base_name + "elasticity_tensor"),
    _elasticity_tensor(getADMaterialProperty<RankFourTensor>(_elasticity_tensor_name)),
    _hardening_constant(getParam<Real>("hardening_constant")),
    _hardening_function(isParamValid("hardening_function") ? &getFunction("hardening_function")
                                                           : nullptr),
    _hardening_variable(declareADProperty<Real>(_base_name + "hardening_variable")),
    _hardening_variable_old(getMaterialPropertyOld<Real>(_base_name + "hardening_variable")),
    _elasticity_eigenvectors(
        declareADProperty<DenseMatrix<Real>>(_base_name + "elasticity_eigenvectors")),
    _elasticity_eigenvalues(
        declareADProperty<DenseVector<Real>>(_base_name + "elasticity_eigenvalues")),
    _b_eigenvectors(declareADProperty<DenseMatrix<Real>>(_base_name + "b_eigenvectors")),
    _b_eigenvalues(declareADProperty<DenseVector<Real>>(_base_name + "b_eigenvalues")),
    _alpha_matrix(declareADProperty<DenseMatrix<Real>>(_base_name + "alpha_matrix")),
    _sigma_tilde(declareADProperty<DenseVector<Real>>(_base_name + "sigma_tilde")),
    _hardening_slope(0.0),
    _yield_condition(1.0),
    _yield_stress(getParam<Real>("yield_stress")),
    _hill_tensor(6, 6)
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
}

void
ADHillElastoPlasticityStressUpdate::propagateQpStatefulProperties()
{
  _hardening_variable[_qp] = _hardening_variable_old[_qp];
  _plasticity_strain[_qp] = _plasticity_strain_old[_qp];
  ADAnisotropicReturnPlasticityStressUpdateBase::propagateQpStatefulProperties();
}

void
ADHillElastoPlasticityStressUpdate::computeStressInitialize(
    const ADDenseVector & stress_dev,
    const ADDenseVector & stress,
    const ADRankFourTensor & /*elasticity_tensor*/)
{
  _hardening_variable[_qp] = _hardening_variable_old[_qp];
  _plasticity_strain[_qp] = _plasticity_strain_old[_qp];
  _effective_inelastic_strain[_qp] = _effective_inelastic_strain_old[_qp];

  _elasticity_eigenvectors[_qp].resize(6, 6);
  _elasticity_eigenvalues[_qp].resize(6);

  _b_eigenvectors[_qp].resize(6, 6);
  _b_eigenvalues[_qp].resize(6);

  _alpha_matrix[_qp].resize(6, 6);
  _sigma_tilde[_qp].resize(6);

  // Algebra needed for the generalized return mapping of anisotropic elastoplasticity
  computeElasticityTensorEigenDecomposition();

  _yield_condition = 1.0; // Some positive value
  _yield_condition = -computeResidual(stress_dev, stress, 0.0);
}

void
ADHillElastoPlasticityStressUpdate::computeElasticityTensorEigenDecomposition()
{
  // Helper method to compute qp matrices required for the elasto-plasticity algorithm.
  _anisotropic_elastic_tensor(0, 0) = (_elasticity_tensor)[_qp](0, 0, 0, 0);
  _anisotropic_elastic_tensor(0, 1) = (_elasticity_tensor)[_qp](0, 0, 1, 1);
  _anisotropic_elastic_tensor(0, 2) = (_elasticity_tensor)[_qp](0, 0, 2, 2);
  _anisotropic_elastic_tensor(0, 3) = (_elasticity_tensor)[_qp](0, 0, 1, 2);
  _anisotropic_elastic_tensor(0, 4) = (_elasticity_tensor)[_qp](0, 0, 0, 2);
  _anisotropic_elastic_tensor(0, 5) = (_elasticity_tensor)[_qp](0, 0, 0, 1);

  _anisotropic_elastic_tensor(1, 0) = (_elasticity_tensor)[_qp](1, 1, 0, 0);
  _anisotropic_elastic_tensor(1, 1) = (_elasticity_tensor)[_qp](1, 1, 1, 1);
  _anisotropic_elastic_tensor(1, 2) = (_elasticity_tensor)[_qp](1, 1, 2, 2);
  _anisotropic_elastic_tensor(1, 3) = (_elasticity_tensor)[_qp](1, 1, 1, 2);
  _anisotropic_elastic_tensor(1, 4) = (_elasticity_tensor)[_qp](1, 1, 0, 2);
  _anisotropic_elastic_tensor(1, 5) = (_elasticity_tensor)[_qp](1, 1, 0, 1);

  _anisotropic_elastic_tensor(2, 0) = (_elasticity_tensor)[_qp](2, 2, 0, 0);
  _anisotropic_elastic_tensor(2, 1) = (_elasticity_tensor)[_qp](2, 2, 1, 1);
  _anisotropic_elastic_tensor(2, 2) = (_elasticity_tensor)[_qp](2, 2, 2, 2);
  _anisotropic_elastic_tensor(2, 3) = (_elasticity_tensor)[_qp](2, 2, 1, 2);
  _anisotropic_elastic_tensor(2, 4) = (_elasticity_tensor)[_qp](2, 2, 0, 2);
  _anisotropic_elastic_tensor(2, 5) = (_elasticity_tensor)[_qp](2, 2, 0, 1);

  _anisotropic_elastic_tensor(3, 0) = (_elasticity_tensor)[_qp](1, 2, 0, 0);
  _anisotropic_elastic_tensor(3, 1) = (_elasticity_tensor)[_qp](1, 2, 1, 1);
  _anisotropic_elastic_tensor(3, 2) = (_elasticity_tensor)[_qp](1, 2, 2, 2);
  _anisotropic_elastic_tensor(3, 3) = (_elasticity_tensor)[_qp](1, 2, 1, 2);
  _anisotropic_elastic_tensor(3, 4) = (_elasticity_tensor)[_qp](1, 2, 0, 2);
  _anisotropic_elastic_tensor(3, 5) = (_elasticity_tensor)[_qp](1, 2, 0, 1);

  _anisotropic_elastic_tensor(4, 0) = (_elasticity_tensor)[_qp](0, 2, 0, 0);
  _anisotropic_elastic_tensor(4, 1) = (_elasticity_tensor)[_qp](0, 2, 1, 1);
  _anisotropic_elastic_tensor(4, 2) = (_elasticity_tensor)[_qp](0, 2, 2, 2);
  _anisotropic_elastic_tensor(4, 3) = (_elasticity_tensor)[_qp](0, 2, 1, 2);
  _anisotropic_elastic_tensor(4, 4) = (_elasticity_tensor)[_qp](0, 2, 0, 2);
  _anisotropic_elastic_tensor(4, 5) = (_elasticity_tensor)[_qp](0, 2, 0, 1);

  _anisotropic_elastic_tensor(5, 0) = (_elasticity_tensor)[_qp](0, 1, 0, 0);
  _anisotropic_elastic_tensor(5, 1) = (_elasticity_tensor)[_qp](0, 1, 1, 1);
  _anisotropic_elastic_tensor(5, 2) = (_elasticity_tensor)[_qp](0, 1, 2, 2);
  _anisotropic_elastic_tensor(5, 3) = (_elasticity_tensor)[_qp](0, 1, 1, 2);
  _anisotropic_elastic_tensor(5, 4) = (_elasticity_tensor)[_qp](0, 1, 0, 2);
  _anisotropic_elastic_tensor(5, 5) = (_elasticity_tensor)[_qp](0, 1, 0, 1);

  const unsigned int dimension = _anisotropic_elastic_tensor.n();

  AnisotropyMatrixReal A;
  for (unsigned int index_i = 0; index_i < dimension; index_i++)
    for (unsigned int index_j = 0; index_j < dimension; index_j++)
      A(index_i, index_j) = MetaPhysicL::raw_value(_anisotropic_elastic_tensor(index_i, index_j));

  Eigen::SelfAdjointEigenSolver<AnisotropyMatrixReal> es(A);

  auto lambda = es.eigenvalues();
  auto v = es.eigenvectors();

  for (unsigned int index_i = 0; index_i < dimension; index_i++)
    _elasticity_eigenvalues[_qp](index_i) = lambda(index_i);

  for (unsigned int index_i = 0; index_i < dimension; index_i++)
    for (unsigned int index_j = 0; index_j < dimension; index_j++)
      _elasticity_eigenvectors[_qp](index_i, index_j) = v(index_i, index_j);

  // Compute sqrt(Delta_c) * QcT * A * Qc * sqrt(Delta_c)
  ADDenseMatrix sqrt_Delta(6, 6);
  for (unsigned int i = 0; i < 6; i++)
  {
    sqrt_Delta(i, i) = std::sqrt(_elasticity_eigenvalues[_qp](i));
  }

  ADDenseMatrix eigenvectors_elasticity_transpose(6, 6);
  _elasticity_eigenvectors[_qp].get_transpose(eigenvectors_elasticity_transpose);

  ADDenseMatrix b_matrix(_hill_tensor);

  // Right multiply by matrix of eigenvectors transpose
  b_matrix.right_multiply(_elasticity_eigenvectors[_qp]);
  b_matrix.right_multiply(sqrt_Delta);
  b_matrix.left_multiply(eigenvectors_elasticity_transpose);
  b_matrix.left_multiply(sqrt_Delta);

  AnisotropyMatrixReal B_eigen;
  for (unsigned int index_i = 0; index_i < dimension; index_i++)
    for (unsigned int index_j = 0; index_j < dimension; index_j++)
      B_eigen(index_i, index_j) = MetaPhysicL::raw_value(b_matrix(index_i, index_j));

  Eigen::SelfAdjointEigenSolver<AnisotropyMatrixReal> es_b(B_eigen);

  auto lambda_b = es_b.eigenvalues();
  auto v_b = es_b.eigenvectors();
  for (unsigned int index_i = 0; index_i < dimension; index_i++)
    _b_eigenvalues[_qp](index_i) = lambda_b(index_i);

  for (unsigned int index_i = 0; index_i < dimension; index_i++)
    for (unsigned int index_j = 0; index_j < dimension; index_j++)
      _b_eigenvectors[_qp](index_i, index_j) = v_b(index_i, index_j);

  _alpha_matrix[_qp] = sqrt_Delta;
  _alpha_matrix[_qp].right_multiply(_b_eigenvectors[_qp]);
  _alpha_matrix[_qp].left_multiply(_elasticity_eigenvectors[_qp]);
}

ADReal
ADHillElastoPlasticityStressUpdate::computeOmega(const ADReal & delta_gamma,
                                                 const ADDenseVector & /*stress_trial*/)
{
  ADDenseVector K(6);
  ADReal omega = 0.0;

  for (unsigned int i = 0; i < 6; i++)
  {
    K(i) = _b_eigenvalues[_qp](i) / Utility::pow<2>(1 + delta_gamma * _b_eigenvalues[_qp](i));
    omega += K(i) * _sigma_tilde[_qp](i) * _sigma_tilde[_qp](i);
  }
  omega *= 0.5;

  if (omega == 0.0)
    omega = 1.0e-36;

  return std::sqrt(omega);
}

Real
ADHillElastoPlasticityStressUpdate::computeReferenceResidual(
    const ADDenseVector & /*effective_trial_stress*/,
    const ADDenseVector & /*stress_new*/,
    const ADReal & /*residual*/,
    const ADReal & /*scalar_effective_inelastic_strain*/)
{
  // Equation is already normalized.
  return 1.0;
}

ADReal
ADHillElastoPlasticityStressUpdate::computeResidual(const ADDenseVector & /*stress_dev*/,
                                                    const ADDenseVector & stress_sigma,
                                                    const ADReal & delta_gamma)
{

  // If in elastic regime, just return
  if (_yield_condition <= 0.0)
    return 0.0;

  // Get stress_tilde
  ADDenseVector stress_tilde(6);
  ADDenseMatrix alpha_temp(_alpha_matrix[_qp]);
  alpha_temp.lu_solve(stress_sigma, stress_tilde);

  // Material property used in computeStressFinalize
  _sigma_tilde[_qp] = stress_tilde;

  ADReal omega = computeOmega(delta_gamma, stress_tilde);
  _hardening_slope = computeHardeningDerivative();

  // Hardening variable is \alpha isotropic hardening for now.
  _hardening_variable[_qp] = computeHardeningValue(delta_gamma, omega);
  ADReal s_y = _hardening_slope * _hardening_variable[_qp] + _yield_stress;

  ADReal residual = 0.0;
  residual = s_y / omega - 1.0;

  return residual;
}

ADReal
ADHillElastoPlasticityStressUpdate::computeDerivative(const ADDenseVector & /*stress_dev*/,
                                                      const ADDenseVector & stress_sigma,
                                                      const ADReal & delta_gamma)
{
  // If in elastic regime, return unit derivative
  if (_yield_condition <= 0.0)
    return 1.0;

  ADReal omega = computeOmega(delta_gamma, stress_sigma);
  _hardening_slope = computeHardeningDerivative();

  ADReal sy = _hardening_slope * computeHardeningValue(delta_gamma, omega) + _yield_stress;
  ADReal sy_alpha = _hardening_slope;

  ADReal omega_gamma;
  ADReal sy_gamma;

  computeDeltaDerivatives(delta_gamma, stress_sigma, sy_alpha, omega, omega_gamma, sy_gamma);
  ADReal residual_derivative = 1 / omega * (sy_gamma - 1 / omega * omega_gamma * sy);

  return residual_derivative;
}

void
ADHillElastoPlasticityStressUpdate::computeDeltaDerivatives(const ADReal & delta_gamma,
                                                            const ADDenseVector & /*stress_trial*/,
                                                            const ADReal & sy_alpha,
                                                            ADReal & omega,
                                                            ADReal & omega_gamma,
                                                            ADReal & sy_gamma)
{
  omega_gamma = 0.0;
  sy_gamma = 0.0;

  ADDenseVector K_deltaGamma(6);
  omega = computeOmega(delta_gamma, _sigma_tilde[_qp]);

  ADDenseVector K(6);
  for (unsigned int i = 0; i < 6; i++)
    K(i) = _b_eigenvalues[_qp](i) / Utility::pow<2>(1 + delta_gamma * _b_eigenvalues[_qp](i));

  for (unsigned int i = 0; i < 6; i++)
    K_deltaGamma(i) =
        -2.0 * _b_eigenvalues[_qp](i) * K(i) / (1 + _b_eigenvalues[_qp](i) * delta_gamma);

  for (unsigned int i = 0; i < 6; i++)
    omega_gamma += K_deltaGamma(i) * _sigma_tilde[_qp](i) * _sigma_tilde[_qp](i);

  omega_gamma /= 4.0 * omega;
  sy_gamma = 2.0 * sy_alpha * (omega + delta_gamma * omega_gamma);
}

ADReal
ADHillElastoPlasticityStressUpdate::computeHardeningValue(const ADReal & delta_gamma,
                                                          const ADReal & omega)
{
  return _hardening_variable_old[_qp] + 2.0 * delta_gamma * omega;
}

ADReal
ADHillElastoPlasticityStressUpdate::computeHardeningDerivative()
{
  return _hardening_constant;
}

void
ADHillElastoPlasticityStressUpdate::computeStrainFinalize(
    ADRankTwoTensor & inelasticStrainIncrement,
    const ADRankTwoTensor & stress,
    const ADDenseVector & stress_dev,
    const ADReal & delta_gamma)
{
  // e^P = delta_gamma * hill_tensor * stress
  ADDenseVector inelasticStrainIncrement_vector(6);
  ADDenseVector hill_stress(6);
  ADDenseVector stress_vector(6);

  stress_vector(0) = stress(0, 0);
  stress_vector(1) = stress(1, 1);
  stress_vector(2) = stress(2, 2);
  stress_vector(3) = stress(0, 1);
  stress_vector(4) = stress(1, 2);
  stress_vector(5) = stress(0, 2);

  _hill_tensor.vector_mult(hill_stress, stress_vector);
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
ADHillElastoPlasticityStressUpdate::computeStressFinalize(
    const ADRankTwoTensor & /*plastic_strain_increment*/,
    const ADReal & delta_gamma,
    ADRankTwoTensor & stress_new,
    const ADDenseVector & /*stress_dev*/,
    const ADRankTwoTensor & /*stress_old*/,
    const ADRankFourTensor & /*elasticity_tensor*/)
{
  // Need to compute this iteration's stress tensor based on the scalar variable
  // For deviatoric
  // sigma(n+1) = {Alpha [I + delta_gamma*Delta_b]^(-1) A^-1}  sigma(trial)

  if (_yield_condition <= 0.0)
    return;
  ADDenseMatrix inv_matrix(6, 6);
  inv_matrix.zero();

  for (unsigned int i = 0; i < 6; i++)
    inv_matrix(i, i) = 1 / (1 + delta_gamma * _b_eigenvalues[_qp](i));

  _alpha_matrix[_qp].right_multiply(inv_matrix);

  ADDenseVector stress_output(6);
  _alpha_matrix[_qp].vector_mult(stress_output, _sigma_tilde[_qp]);

  stress_new(0, 0) = stress_output(0);
  stress_new(1, 1) = stress_output(1);
  stress_new(2, 2) = stress_output(2);
  stress_new(0, 1) = stress_output(3);
  stress_new(1, 2) = stress_output(4);
  stress_new(0, 2) = stress_output(5);
}

Real
ADHillElastoPlasticityStressUpdate::computeStrainEnergyRateDensity(
    const ADMaterialProperty<RankTwoTensor> & /*stress*/,
    const ADMaterialProperty<RankTwoTensor> & /*strain_rate*/)
{
  mooseError("computeStrainEnergyRateDensity not implemented for anisotropic creep.");
  return 0.0;
}
