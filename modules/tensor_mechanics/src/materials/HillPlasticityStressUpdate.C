//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "HillPlasticityStressUpdate.h"
#include "ElasticityTensorTools.h"

registerMooseObject("TensorMechanicsApp", ADHillPlasticityStressUpdate);
registerMooseObject("TensorMechanicsApp", HillPlasticityStressUpdate);

template <bool is_ad>
InputParameters
HillPlasticityStressUpdateTempl<is_ad>::validParams()
{
  InputParameters params = AnisotropicReturnPlasticityStressUpdateBaseTempl<is_ad>::validParams();
  params.addClassDescription(
      "This class uses the generalized radial return for anisotropic plasticity model."
      "This class can be used in conjunction with other creep and plasticity materials for "
      "more complex simulations.");

  params.addRequiredParam<Real>("hardening_constant",
                                "Hardening constant (H) for anisotropic plasticity");
  params.addParam<Real>(
      "hardening_exponent", 1.0, "Hardening exponent (n) for anisotropic plasticity");
  params.addRequiredParam<Real>("yield_stress",
                                "Yield stress (constant value) for anisotropic plasticity");

  return params;
}

template <bool is_ad>
HillPlasticityStressUpdateTempl<is_ad>::HillPlasticityStressUpdateTempl(
    const InputParameters & parameters)
  : AnisotropicReturnPlasticityStressUpdateBaseTempl<is_ad>(parameters),
    _qsigma(0.0),
    _eigenvalues_hill(6),
    _eigenvectors_hill(6, 6),
    _hardening_constant(this->template getParam<Real>("hardening_constant")),
    _hardening_exponent(this->template getParam<Real>("hardening_exponent")),
    _hardening_variable(this->template declareGenericProperty<Real, is_ad>(this->_base_name +
                                                                           "hardening_variable")),
    _hardening_variable_old(
        this->template getMaterialPropertyOld<Real>(this->_base_name + "hardening_variable")),
    _hardening_derivative(0.0),
    _yield_condition(1.0),
    _yield_stress(this->template getParam<Real>("yield_stress")),
    _hill_tensor(this->template getMaterialPropertyByName<DenseMatrix<Real>>(this->_base_name +
                                                                             "hill_tensor")),
    _stress_np1(6)
{
}

template <bool is_ad>
void
HillPlasticityStressUpdateTempl<is_ad>::propagateQpStatefulProperties()
{
  _hardening_variable[_qp] = _hardening_variable_old[_qp];
  _plasticity_strain[_qp] = _plasticity_strain_old[_qp];
  AnisotropicReturnPlasticityStressUpdateBaseTempl<is_ad>::propagateQpStatefulProperties();
}

template <bool is_ad>
void
HillPlasticityStressUpdateTempl<is_ad>::computeStressInitialize(
    const GenericDenseVector<is_ad> & stress_dev,
    const GenericDenseVector<is_ad> & /*stress*/,
    const GenericRankFourTensor<is_ad> & elasticity_tensor)
{
  _hardening_variable[_qp] = _hardening_variable_old[_qp];
  _plasticity_strain[_qp] = _plasticity_strain_old[_qp];
  _effective_inelastic_strain[_qp] = _effective_inelastic_strain_old[_qp];

  _two_shear_modulus = 2.0 * ElasticityTensorTools::getIsotropicShearModulus(elasticity_tensor);

  // Hill constants: We use directly the transformation tensor, which won't be updated if not
  // necessary in the Hill tensor material.
  computeHillTensorEigenDecomposition(_hill_tensor[_qp]);

  _yield_condition = 1.0; // Some positive value
  _yield_condition = -computeResidual(stress_dev, stress_dev, 0.0);
}

template <bool is_ad>
GenericReal<is_ad>
HillPlasticityStressUpdateTempl<is_ad>::computeOmega(const GenericReal<is_ad> & delta_gamma,
                                                     const GenericDenseVector<is_ad> & stress_trial)
{
  GenericDenseVector<is_ad> K(6);
  GenericReal<is_ad> omega = 0.0;

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

template <bool is_ad>
void
HillPlasticityStressUpdateTempl<is_ad>::computeDeltaDerivatives(
    const GenericReal<is_ad> & delta_gamma,
    const GenericDenseVector<is_ad> & stress_trial,
    const GenericReal<is_ad> & sy_alpha,
    GenericReal<is_ad> & omega,
    GenericReal<is_ad> & omega_gamma,
    GenericReal<is_ad> & sy_gamma)
{
  omega_gamma = 0.0;
  sy_gamma = 0.0;

  GenericDenseVector<is_ad> K_deltaGamma(6);
  omega = computeOmega(delta_gamma, stress_trial);

  GenericDenseVector<is_ad> K(6);
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

template <bool is_ad>
Real
HillPlasticityStressUpdateTempl<is_ad>::computeReferenceResidual(
    const GenericDenseVector<is_ad> & /*effective_trial_stress*/,
    const GenericDenseVector<is_ad> & /*stress_new*/,
    const GenericReal<is_ad> & /*residual*/,
    const GenericReal<is_ad> & /*scalar_effective_inelastic_strain*/)
{
  // Equation is already normalized.
  return 1.0;
}

template <bool is_ad>
GenericReal<is_ad>
HillPlasticityStressUpdateTempl<is_ad>::computeResidual(
    const GenericDenseVector<is_ad> & stress_dev,
    const GenericDenseVector<is_ad> & /*stress_sigma*/,
    const GenericReal<is_ad> & delta_gamma)
{

  // If in elastic regime, just return
  if (_yield_condition <= 0.0)
    return 0.0;

  GenericDenseMatrix<is_ad> eigenvectors_hill_transpose(6, 6);

  _eigenvectors_hill.get_transpose(eigenvectors_hill_transpose);
  eigenvectors_hill_transpose.vector_mult(_stress_np1, stress_dev);

  GenericReal<is_ad> omega = computeOmega(delta_gamma, _stress_np1);

  // Hardening variable is \alpha isotropic hardening for now.
  _hardening_variable[_qp] = computeHardeningValue(delta_gamma, omega);
  GenericReal<is_ad> s_y =
      _hardening_constant * std::pow(_hardening_variable[_qp] + 1.0e-30, _hardening_exponent) +
      _yield_stress;

  GenericReal<is_ad> residual = 0.0;
  residual = s_y / omega - 1.0;

  return residual;
}

template <bool is_ad>
GenericReal<is_ad>
HillPlasticityStressUpdateTempl<is_ad>::computeDerivative(
    const GenericDenseVector<is_ad> & /*stress_dev*/,
    const GenericDenseVector<is_ad> & /*stress_sigma*/,
    const GenericReal<is_ad> & delta_gamma)
{
  // If in elastic regime, return unit derivative
  if (_yield_condition <= 0.0)
    return 1.0;

  GenericReal<is_ad> omega = computeOmega(delta_gamma, _stress_np1);
  _hardening_derivative = computeHardeningDerivative();

  GenericReal<is_ad> sy =
      _hardening_derivative * computeHardeningValue(delta_gamma, omega) + _yield_stress;
  GenericReal<is_ad> sy_alpha = _hardening_derivative;

  GenericReal<is_ad> omega_gamma;
  GenericReal<is_ad> sy_gamma;

  computeDeltaDerivatives(delta_gamma, _stress_np1, sy_alpha, omega, omega_gamma, sy_gamma);
  GenericReal<is_ad> residual_derivative = 1 / omega * (sy_gamma - 1 / omega * omega_gamma * sy);

  return residual_derivative;
}

template <bool is_ad>
void
HillPlasticityStressUpdateTempl<is_ad>::computeHillTensorEigenDecomposition(
    const DenseMatrix<Real> & hill_tensor)
{
  const unsigned int dimension = hill_tensor.n();

  AnisotropyMatrixReal A;
  for (unsigned int index_i = 0; index_i < dimension; index_i++)
    for (unsigned int index_j = 0; index_j < dimension; index_j++)
      A(index_i, index_j) = MetaPhysicL::raw_value(hill_tensor(index_i, index_j));

  if (isBlockDiagonal(A))
  {
    Eigen::SelfAdjointEigenSolver<AnisotropyMatrixRealBlock> es(A.block<3, 3>(0, 0));

    auto lambda = es.eigenvalues();
    auto v = es.eigenvectors();

    _eigenvalues_hill(0) = lambda(0);
    _eigenvalues_hill(1) = lambda(1);
    _eigenvalues_hill(2) = lambda(2);
    _eigenvalues_hill(3) = A(3, 3);
    _eigenvalues_hill(4) = A(4, 4);
    _eigenvalues_hill(5) = A(5, 5);

    _eigenvectors_hill(0, 0) = v(0, 0);
    _eigenvectors_hill(0, 1) = v(0, 1);
    _eigenvectors_hill(0, 2) = v(0, 2);
    _eigenvectors_hill(1, 0) = v(1, 0);
    _eigenvectors_hill(1, 1) = v(1, 1);
    _eigenvectors_hill(1, 2) = v(1, 2);
    _eigenvectors_hill(2, 0) = v(2, 0);
    _eigenvectors_hill(2, 1) = v(2, 1);
    _eigenvectors_hill(2, 2) = v(2, 2);
    _eigenvectors_hill(3, 3) = 1.0;
    _eigenvectors_hill(4, 4) = 1.0;
    _eigenvectors_hill(5, 5) = 1.0;
  }
  else
  {
    Eigen::SelfAdjointEigenSolver<AnisotropyMatrixReal> es_b(A);

    auto lambda_b = es_b.eigenvalues();
    auto v_b = es_b.eigenvectors();
    for (unsigned int index_i = 0; index_i < dimension; index_i++)
      _eigenvalues_hill(index_i) = lambda_b(index_i);

    for (unsigned int index_i = 0; index_i < dimension; index_i++)
      for (unsigned int index_j = 0; index_j < dimension; index_j++)
        _eigenvectors_hill(index_i, index_j) = v_b(index_i, index_j);
  }
}

template <bool is_ad>
GenericReal<is_ad>
HillPlasticityStressUpdateTempl<is_ad>::computeHardeningValue(
    const GenericReal<is_ad> & delta_gamma, const GenericReal<is_ad> & omega)
{
  return _hardening_variable_old[_qp] + 2.0 * delta_gamma * omega;
}

template <bool is_ad>
Real
HillPlasticityStressUpdateTempl<is_ad>::computeHardeningDerivative()
{
  return _hardening_constant * _hardening_exponent *
         MetaPhysicL::raw_value(
             std::pow(_hardening_variable[_qp] + 1.0e-30, _hardening_exponent - 1));
}

template <bool is_ad>
void
HillPlasticityStressUpdateTempl<is_ad>::computeStrainFinalize(
    GenericRankTwoTensor<is_ad> & inelasticStrainIncrement,
    const GenericRankTwoTensor<is_ad> & stress,
    const GenericDenseVector<is_ad> & stress_dev,
    const GenericReal<is_ad> & delta_gamma)
{
  // e^P = delta_gamma * hill_tensor * stress
  GenericDenseVector<is_ad> inelasticStrainIncrement_vector(6);
  GenericDenseVector<is_ad> hill_stress(6);
  _hill_tensor[_qp].vector_mult(hill_stress, stress_dev);
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

  // Calculate equivalent plastic strain
  GenericDenseVector<is_ad> Mepsilon(6);
  _hill_tensor[_qp].vector_mult(Mepsilon, inelasticStrainIncrement_vector);
  GenericReal<is_ad> eq_plastic_strain_inc = Mepsilon.dot(inelasticStrainIncrement_vector);

  if (eq_plastic_strain_inc > 0.0)
    eq_plastic_strain_inc = std::sqrt(eq_plastic_strain_inc);

  _effective_inelastic_strain[_qp] = _effective_inelastic_strain_old[_qp] + eq_plastic_strain_inc;

  AnisotropicReturnPlasticityStressUpdateBaseTempl<is_ad>::computeStrainFinalize(
      inelasticStrainIncrement, stress, stress_dev, delta_gamma);
}

template <bool is_ad>
void
HillPlasticityStressUpdateTempl<is_ad>::computeStressFinalize(
    const GenericRankTwoTensor<is_ad> & /*plastic_strain_increment*/,
    const GenericReal<is_ad> & delta_gamma,
    GenericRankTwoTensor<is_ad> & stress_new,
    const GenericDenseVector<is_ad> & stress_dev,
    const GenericRankTwoTensor<is_ad> & /*sstress_old*/,
    const GenericRankFourTensor<is_ad> & /*elasticity_tensor*/)
{
  // Need to compute this iteration's stress tensor based on the scalar variable for deviatoric
  // s(n+1) = {Q [I + 2*nu*delta_gamma*Delta]^(-1) Q^T}  s(trial)

  if (_yield_condition <= 0.0)
    return;

  GenericDenseMatrix<is_ad> inv_matrix(6, 6);
  for (unsigned int i = 0; i < 6; i++)
    inv_matrix(i, i) = 1 / (1 + _two_shear_modulus * delta_gamma * _eigenvalues_hill(i));

  GenericDenseMatrix<is_ad> eigenvectors_hill_transpose(6, 6);

  _eigenvectors_hill.get_transpose(eigenvectors_hill_transpose);
  GenericDenseMatrix<is_ad> eigenvectors_hill_copy(_eigenvectors_hill);

  // Right multiply by matrix of eigenvectors transpose
  inv_matrix.right_multiply(eigenvectors_hill_transpose);
  // Right multiply eigenvector matrix by [I + 2*nu*delta_gamma*Delta]^(-1) Q^T
  eigenvectors_hill_copy.right_multiply(inv_matrix);

  GenericDenseVector<is_ad> stress_np1(6);
  eigenvectors_hill_copy.vector_mult(stress_np1, stress_dev);

  GenericRankTwoTensor<is_ad> stress_new_volumetric = stress_new - stress_new.deviatoric();

  stress_new(0, 0) = stress_new_volumetric(0, 0) + stress_np1(0);
  stress_new(1, 1) = stress_new_volumetric(1, 1) + stress_np1(1);
  stress_new(2, 2) = stress_new_volumetric(2, 2) + stress_np1(2);
  stress_new(0, 1) = stress_new(1, 0) = stress_np1(3);
  stress_new(1, 2) = stress_new(2, 1) = stress_np1(4);
  stress_new(0, 2) = stress_new(2, 0) = stress_np1(5);

  GenericReal<is_ad> omega = computeOmega(delta_gamma, _stress_np1);
  _hardening_variable[_qp] = computeHardeningValue(delta_gamma, omega);
}

template class HillPlasticityStressUpdateTempl<false>;
template class HillPlasticityStressUpdateTempl<true>;
