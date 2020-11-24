//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADTransverselyIsotropicCreepStressUpdate.h"

registerMooseObject("TensorMechanicsApp", ADTransverselyIsotropicCreepStressUpdate);

InputParameters
ADTransverselyIsotropicCreepStressUpdate::validParams()
{
  InputParameters params = ADAnisotropicReturnCreepStressUpdateBase::validParams();
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

ADTransverselyIsotropicCreepStressUpdate::ADTransverselyIsotropicCreepStressUpdate(
    const InputParameters & parameters)
  : ADAnisotropicReturnCreepStressUpdateBase(parameters),
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
    _qsigma(0.0),
    _eigenvalues_hill(6),
    _eigenvectors_hill(6, 6)
{
  if (_start_time < _app.getStartTime() && (std::trunc(_m_exponent) != _m_exponent))
    paramError("start_time",
               "Start time must be equal to or greater than the Executioner start_time if a "
               "non-integer m_exponent is used");

  _hill_constants = getParam<std::vector<Real>>("hill_constants");

  // Hill constants, some constraints apply
  const Real F = _hill_constants[0];
  const Real G = _hill_constants[1];
  const Real H = _hill_constants[2];
  const Real L = _hill_constants[3];
  const Real M = _hill_constants[4];
  const Real N = _hill_constants[5];

  ADDenseMatrix hill_tensor(6, 6);
  hill_tensor(0, 0) = G + H;
  hill_tensor(1, 1) = F + H;
  hill_tensor(2, 2) = F + G;
  hill_tensor(0, 1) = hill_tensor(1, 0) = -H;
  hill_tensor(0, 2) = hill_tensor(2, 0) = -G;
  hill_tensor(1, 2) = hill_tensor(2, 1) = -F;

  hill_tensor(3, 3) = 2.0 * N;
  hill_tensor(4, 4) = 2.0 * L;
  hill_tensor(5, 5) = 2.0 * M;
  //  Moose::out << "hill_tensor constructor: " << hill_tensor << "\n";

  computeHillTensorEigenDecomposition(hill_tensor);
}

void
ADTransverselyIsotropicCreepStressUpdate::computeStressInitialize(
    const ADDenseVector & /*effective_trial_stress*/,
    const ADRankFourTensor & /*elasticity_tensor*/)
{
  if (_has_temp)
    _exponential = std::exp(-_activation_energy / (_gas_constant * _temperature[_qp]));

  _exp_time = std::pow(_t - _start_time, _m_exponent);
}

ADReal
ADTransverselyIsotropicCreepStressUpdate::computeResidual(
    const ADDenseVector & /*effective_trial_stress*/,
    const ADDenseVector & stress_new,
    const ADReal & delta_gamma)
{
  // Hill constants, some constraints apply
  const Real F = _hill_constants[0];
  const Real G = _hill_constants[1];
  const Real H = _hill_constants[2];
  const Real L = _hill_constants[3];
  const Real M = _hill_constants[4];
  const Real N = _hill_constants[5];

  //  ADDenseMatrix inv_matrix(6, 6);
  //  for (unsigned int i = 0; i < 6; i++)
  //    inv_matrix(i, i) = 1 / (1 + _two_shear_modulus * delta_gamma * _eigenvalues_hill(i));
  //
  //  ADDenseVector stress_vector(6);
  //  stress_vector(0) = stress_new(0);
  //  stress_vector(1) = stress_new(1);
  //  stress_vector(2) = stress_new(2);
  //  stress_vector(3) = stress_new(3);
  //  stress_vector(4) = stress_new(4);
  //  stress_vector(5) = stress_new(5);
  //
  //  ADDenseMatrix eigenvectors_hill_transpose(6, 6);
  //
  //  _eigenvectors_hill.get_transpose(eigenvectors_hill_transpose);
  //  ADDenseMatrix eigenvectors_hill_copy(_eigenvectors_hill);
  //
  //  // Right multiply by matrix of eigenvectors transpose
  //  inv_matrix.right_multiply(eigenvectors_hill_transpose);
  //  // Right multiply eigenvector matrix by [I + 2*nu*delta_gamma*Delta]^(-1) Q^T
  //  eigenvectors_hill_copy.right_multiply(inv_matrix);
  //
  //  ADDenseVector stress_np1(6);
  //  eigenvectors_hill_copy.vector_mult(stress_np1, stress_vector);

  ADReal qsigma_square = F * (stress_new(1) - stress_new(2)) * (stress_new(1) - stress_new(2));
  qsigma_square += G * (stress_new(2) - stress_new(0)) * (stress_new(2) - stress_new(0));
  qsigma_square += H * (stress_new(0) - stress_new(1)) * (stress_new(0) - stress_new(1));
  qsigma_square += 2 * L * stress_new(4) * stress_new(4);
  qsigma_square += 2 * M * stress_new(5) * stress_new(5);
  qsigma_square += 2 * N * stress_new(3) * stress_new(3);

  // moose assert > 0
  qsigma_square = std::sqrt(qsigma_square);
  const ADReal creep_rate =
      _coefficient * std::pow(qsigma_square, _n_exponent) * _exponential * _exp_time;

  // Return iteration difference between creep strain and inelastic strain multiplier
  return creep_rate * _dt - delta_gamma;
}
Real
ADTransverselyIsotropicCreepStressUpdate::computeReferenceResidual(
    const ADDenseVector & /*effective_trial_stress*/,
    const ADDenseVector & /*stress_new*/,
    const ADReal & /*residual*/,
    const ADReal & /*scalar_effective_inelastic_strain*/)
{
  // TODO, fix this joke reference residual.
  return 0.00000001;
}

ADReal
ADTransverselyIsotropicCreepStressUpdate::computeDerivative(
    const ADDenseVector & /*effective_trial_stress*/,
    const ADDenseVector & stress_new,
    const ADReal & delta_gamma)
{
  // Hill constants, some constraints apply
  const Real F = _hill_constants[0];
  const Real G = _hill_constants[1];
  const Real H = _hill_constants[2];
  const Real L = _hill_constants[3];
  const Real M = _hill_constants[4];
  const Real N = _hill_constants[5];

  //  ADDenseMatrix inv_matrix(6, 6);
  //  for (unsigned int i = 0; i < 6; i++)
  //    inv_matrix(i, i) = 1 / (1 + _two_shear_modulus * delta_gamma * _eigenvalues_hill(i));
  //
  //  ADDenseVector stress_vector(6);
  //  stress_vector(0) = stress_new(0);
  //  stress_vector(1) = stress_new(1);
  //  stress_vector(2) = stress_new(2);
  //  stress_vector(3) = stress_new(3);
  //  stress_vector(4) = stress_new(4);
  //  stress_vector(5) = stress_new(5);
  //
  //  ADDenseMatrix eigenvectors_hill_transpose(6, 6);
  //
  //  _eigenvectors_hill.get_transpose(eigenvectors_hill_transpose);
  //  ADDenseMatrix eigenvectors_hill_copy(_eigenvectors_hill);
  //
  //  // Right multiply by matrix of eigenvectors transpose
  //  inv_matrix.right_multiply(eigenvectors_hill_transpose);
  //  // Right multiply eigenvector matrix by [I + 2*nu*delta_gamma*Delta]^(-1) Q^T
  //  eigenvectors_hill_copy.right_multiply(inv_matrix);
  //
  //  ADDenseVector stress_np1(6);
  //  eigenvectors_hill_copy.vector_mult(stress_np1, stress_vector);

  // Equivalent deviatoric stress function.
  ADReal qsigma_square = F * (stress_new(1) - stress_new(2)) * (stress_new(1) - stress_new(2));
  qsigma_square += G * (stress_new(2) - stress_new(0)) * (stress_new(2) - stress_new(0));
  qsigma_square += H * (stress_new(0) - stress_new(1)) * (stress_new(0) - stress_new(1));
  qsigma_square += 2 * L * stress_new(4) * stress_new(4);
  qsigma_square += 2 * M * stress_new(5) * stress_new(5);
  qsigma_square += 2 * N * stress_new(3) * stress_new(3);

  // moose assert > 0
  qsigma_square = std::sqrt(qsigma_square);
  _qsigma = qsigma_square;

  const ADReal creep_rate_derivative = 1.0 * _coefficient * _n_exponent *
                                       std::pow(qsigma_square, _n_exponent - 1.0) * _exponential *
                                       _exp_time;
  return (creep_rate_derivative * _dt - 1.0);
}

void
ADTransverselyIsotropicCreepStressUpdate::computeHillTensorEigenDecomposition(
    ADDenseMatrix & hill_tensor)
{
  unsigned int dimension = hill_tensor.n();

  AnisotropyMatrix A;
  for (unsigned int index_i = 0; index_i < dimension; index_i++)
    for (unsigned int index_j = 0; index_j < dimension; index_j++)
      A(index_i, index_j) = hill_tensor(index_i, index_j);

  //  Moose::out << "Print A matrix: " << MetaPhysicL::raw_value(A) << "\n";

  Eigen::SelfAdjointEigenSolver<AnisotropyMatrix> es(A);
  //  std::cout << "The eigenvalues of the pencil (A) are:" << std::endl
  //            << es.eigenvalues() << std::endl;
  //  std::cout << "The matrix of eigenvectors, V, is:" << std::endl
  //            << es.eigenvectors() << std::endl
  //            << std::endl;

  auto lambda = es.eigenvalues();
  auto v = es.eigenvectors();
  //  std::cout << "Consider the first eigenvalue, lambda = " << lambda << std::endl;
  //  std::cout << "If v is the corresponding eigenvector, then A * v = " << std::endl
  //            << A * v << std::endl;

  // hill_tensor_real.evd_left(eigenvalues, eigenvalues_imag, eigenvectors);
  for (unsigned int index_i = 0; index_i < dimension; index_i++)
    _eigenvalues_hill(index_i) = lambda(index_i);

  for (unsigned int index_i = 0; index_i < dimension; index_i++)
    for (unsigned int index_j = 0; index_j < dimension; index_j++)
      _eigenvectors_hill(index_i, index_j) = es.eigenvectors()(index_i, index_j);

  //  for (unsigned int index_i = 0; index_i < dimension; index_i++)
  //    Moose::out << "eigenvalues: " << _eigenvalues_hill(index_i) << "\n";
  //
  //  for (unsigned int index_i = 0; index_i < dimension; index_i++)
  //    for (unsigned int index_j = 0; index_j < dimension; index_j++)
  //      Moose::out << "eigenvectors print: " << _eigenvectors_hill(index_i, index_j) << "\n";
}

void
ADTransverselyIsotropicCreepStressUpdate::computeStrainFinalize(
    ADRankTwoTensor & inelasticStrainIncrement,
    const ADRankTwoTensor & stress,
    const ADReal & delta_gamma)
{
  // Hill constants, some constraints apply
  const Real F = _hill_constants[0];
  const Real G = _hill_constants[1];
  const Real H = _hill_constants[2];
  const Real L = _hill_constants[3];
  const Real M = _hill_constants[4];
  const Real N = _hill_constants[5];

  // Equivalent deviatoric stress function.
  ADReal qsigma_square = F * (stress(1, 1) - stress(2, 2)) * (stress(1, 1) - stress(2, 2));
  qsigma_square += G * (stress(2, 2) - stress(0, 0)) * (stress(2, 2) - stress(0, 0));
  qsigma_square += H * (stress(0, 0) - stress(1, 1)) * (stress(0, 0) - stress(1, 1));
  qsigma_square += 2 * L * stress(1, 2) * stress(1, 2);
  qsigma_square += 2 * M * stress(0, 2) * stress(0, 2);
  qsigma_square += 2 * N * stress(0, 1) * stress(0, 1);

  // moose assert > 0
  qsigma_square = std::sqrt(qsigma_square);
  if (qsigma_square == 0)
  {
    inelasticStrainIncrement(0, 0) = inelasticStrainIncrement(1, 1) =
        inelasticStrainIncrement(2, 2) = inelasticStrainIncrement(0, 1) =
            inelasticStrainIncrement(1, 0) = inelasticStrainIncrement(2, 0) =
                inelasticStrainIncrement(0, 2) = inelasticStrainIncrement(1, 2) =
                    inelasticStrainIncrement(2, 1) = 0.0;

    ADAnisotropicReturnCreepStressUpdateBase::computeStrainFinalize(
        inelasticStrainIncrement, stress, delta_gamma);
    return;
  }

  // Use Hill-type flow rule to compute the time step inelastic increment.
  const ADReal prefactor = delta_gamma / qsigma_square;

  inelasticStrainIncrement(0, 0) =
      prefactor * (H * (stress(0, 0) - stress(1, 1)) - G * (stress(2, 2) - stress(0, 0)));
  inelasticStrainIncrement(1, 1) =
      prefactor * (F * (stress(1, 1) - stress(2, 2)) - H * (stress(0, 0) - stress(1, 1)));
  inelasticStrainIncrement(2, 2) =
      prefactor * (G * (stress(2, 2) - stress(0, 0)) - F * (stress(1, 1) - stress(2, 2)));

  inelasticStrainIncrement(0, 1) = inelasticStrainIncrement(1, 0) =
      prefactor * 2.0 * N * stress(0, 1);
  inelasticStrainIncrement(0, 2) = inelasticStrainIncrement(2, 0) =
      prefactor * 2.0 * M * stress(0, 2);
  inelasticStrainIncrement(1, 2) = inelasticStrainIncrement(2, 1) =
      prefactor * 2.0 * L * stress(1, 2);

  ADAnisotropicReturnCreepStressUpdateBase::computeStrainFinalize(
      inelasticStrainIncrement, stress, delta_gamma);
}

void
ADTransverselyIsotropicCreepStressUpdate::computeStressFinalize(
    const ADRankTwoTensor & /*creepStrainIncrement*/,
    const ADReal & delta_gamma,
    ADRankTwoTensor & stress_new)
{
  // Need to compute this iteration's stress tensor based on the scalar variable
  // s(n+1) = {Q [I + 2*nu*delta_gamma*Delta]^(-1) Q^T}  s(trial)

  //  ADDenseMatrix inv_matrix(6, 6);
  //  for (unsigned int i = 0; i < 6; i++)
  //    inv_matrix(i, i) = 1 / (1 + _two_shear_modulus * delta_gamma * _eigenvalues_hill(i));
  //
  //  ADDenseVector stress_vector(6);
  //  stress_vector(0) = stress_new(0, 0);
  //  stress_vector(1) = stress_new(1, 1);
  //  stress_vector(2) = stress_new(2, 2);
  //  stress_vector(3) = stress_new(0, 1);
  //  stress_vector(4) = stress_new(1, 2);
  //  stress_vector(5) = stress_new(0, 2);
  //
  //  ADDenseMatrix eigenvectors_hill_transpose(6, 6);
  //
  //  _eigenvectors_hill.get_transpose(eigenvectors_hill_transpose);
  //  ADDenseMatrix eigenvectors_hill_copy(_eigenvectors_hill);
  //
  //  // Right multiply by matrix of eigenvectors transpose
  //  inv_matrix.right_multiply(eigenvectors_hill_transpose);
  //  // Right multiply eigenvector matrix by [I + 2*nu*delta_gamma*Delta]^(-1) Q^T
  //  eigenvectors_hill_copy.right_multiply(inv_matrix);
  //
  //  ADDenseVector stress_np1(6);
  //  eigenvectors_hill_copy.vector_mult(stress_np1, stress_vector);
  //  stress_new(0, 0) = stress_vector(0);
  //  stress_new(1, 1) = stress_vector(1);
  //  stress_new(2, 2) = stress_vector(2);
  //  stress_new(0, 1) = stress_new(1, 0) = stress_vector(3);
  //  stress_new(1, 2) = stress_new(2, 1) = stress_vector(4);
  //  stress_new(0, 2) = stress_new(2, 0) = stress_vector(5);
}

Real
ADTransverselyIsotropicCreepStressUpdate::computeStrainEnergyRateDensity(
    const ADMaterialProperty<RankTwoTensor> & /*stress*/,
    const ADMaterialProperty<RankTwoTensor> & /*strain_rate*/)
{
  mooseError("computeStrainEnergyRateDensity not implemented for anisotropic creep.");
  return 0.0;
}
