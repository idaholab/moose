//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "HillElastoPlasticityStressUpdate.h"
#include "RankTwoScalarTools.h"
#include "ElasticityTensorTools.h"

registerMooseObject("SolidMechanicsApp", ADHillElastoPlasticityStressUpdate);
registerMooseObject("SolidMechanicsApp", HillElastoPlasticityStressUpdate);

template <bool is_ad>
InputParameters
HillElastoPlasticityStressUpdateTempl<is_ad>::validParams()
{
  InputParameters params = AnisotropicReturnPlasticityStressUpdateBaseTempl<is_ad>::validParams();
  params.addClassDescription(
      "This class uses the generalized radial return for anisotropic elasto-plasticity model."
      "This class can be used in conjunction with other creep and plasticity materials for "
      "more complex simulations.");

  params.addRequiredParam<Real>("hardening_constant",
                                "Hardening constant (H) for anisotropic plasticity");
  params.addParam<Real>(
      "hardening_exponent", 1.0, "Hardening exponent (n) for anisotropic plasticity");
  params.addRequiredParam<Real>("yield_stress",
                                "Yield stress (constant value) for anisotropic plasticity");
  params.addParam<bool>(
      "local_cylindrical_csys",
      false,
      "Compute inelastic strain increment in local cylindrical coordinate system");

  params.addParam<MooseEnum>("axis",
                             MooseEnum("x y z", "y"),
                             "The axis of cylindrical component about which to rotate when "
                             "computing inelastic strain increment in local coordinate system");

  return params;
}

template <bool is_ad>
HillElastoPlasticityStressUpdateTempl<is_ad>::HillElastoPlasticityStressUpdateTempl(
    const InputParameters & parameters)
  : AnisotropicReturnPlasticityStressUpdateBaseTempl<is_ad>(parameters),
    _qsigma(0.0),
    _eigenvalues_hill(6),
    _eigenvectors_hill(6, 6),
    _anisotropic_elastic_tensor(6, 6),
    _elasticity_tensor_name(this->_base_name + "elasticity_tensor"),
    _elasticity_tensor(
        this->template getGenericMaterialProperty<RankFourTensor, is_ad>(_elasticity_tensor_name)),
    _hardening_constant(this->template getParam<Real>("hardening_constant")),
    _hardening_exponent(this->template getParam<Real>("hardening_exponent")),
    _hardening_variable(this->template declareGenericProperty<Real, is_ad>(this->_base_name +
                                                                           "hardening_variable")),
    _hardening_variable_old(
        this->template getMaterialPropertyOld<Real>(this->_base_name + "hardening_variable")),
    _elasticity_eigenvectors(this->template declareGenericProperty<DenseMatrix<Real>, is_ad>(
        this->_base_name + "elasticity_eigenvectors")),
    _elasticity_eigenvalues(this->template declareGenericProperty<DenseVector<Real>, is_ad>(
        this->_base_name + "elasticity_eigenvalues")),
    _b_eigenvectors(this->template declareGenericProperty<DenseMatrix<Real>, is_ad>(
        this->_base_name + "b_eigenvectors")),
    _b_eigenvalues(this->template declareGenericProperty<DenseVector<Real>, is_ad>(
        this->_base_name + "b_eigenvalues")),
    _alpha_matrix(this->template declareGenericProperty<DenseMatrix<Real>, is_ad>(this->_base_name +
                                                                                  "alpha_matrix")),
    _sigma_tilde(this->template declareGenericProperty<DenseVector<Real>, is_ad>(this->_base_name +
                                                                                 "sigma_tilde")),
    _sigma_tilde_rotated(this->template declareGenericProperty<DenseVector<Real>, is_ad>(
        this->_base_name + "sigma_tilde_rotated")),
    _hardening_derivative(0.0),
    _yield_condition(1.0),
    _yield_stress(this->template getParam<Real>("yield_stress")),
    _hill_tensor(this->template getMaterialPropertyByName<DenseMatrix<Real>>(this->_base_name +
                                                                             "hill_tensor")),
    _rotation_matrix(
        this->template declareProperty<RankTwoTensor>(this->_base_name + "rotation_matrix")),
    _rotation_matrix_transpose(this->template declareProperty<RankTwoTensor>(
        this->_base_name + "rotation_matrix_transpose")),
    _rotation_matrix_old(
        this->template getMaterialPropertyOld<RankTwoTensor>(this->_base_name + "rotation_matrix")),
    _rotation_matrix_transpose_old(this->template getMaterialPropertyOld<RankTwoTensor>(
        this->_base_name + "rotation_matrix_transpose")),
    _local_cylindrical_csys(this->template getParam<bool>("local_cylindrical_csys")),
    _axis(this->template getParam<MooseEnum>("axis").template getEnum<Axis>())
{
  if (_local_cylindrical_csys && !parameters.isParamSetByUser("axis"))
  {
    this->paramError(
        "local_cylindrical_csys",
        "\nIf parameter local_cylindrical_csys is provided then parameter axis should be also "
        "provided.");
  }

  if (!_local_cylindrical_csys && parameters.isParamSetByUser("axis"))
  {
    this->paramError("axis",
                     "\nIf parameter axis is provided then the parameter local_cylindrical_csys "
                     "should be set to true");
  }
}

template <bool is_ad>
void
HillElastoPlasticityStressUpdateTempl<is_ad>::initQpStatefulProperties()
{
  RealVectorValue normal_vector(0, 0, 0);
  RealVectorValue axial_vector(0, 0, 0);

  if (_local_cylindrical_csys)
  {
    if (_axis == Axis::X)
    {
      normal_vector(0) = 0.0;
      normal_vector(1) = _q_point[_qp](1);
      normal_vector(2) = _q_point[_qp](2);
      axial_vector(0) = 1.0;
    }
    else if (_axis == Axis::Y)
    {
      normal_vector(0) = _q_point[_qp](0);
      normal_vector(1) = 0.0;
      normal_vector(2) = _q_point[_qp](2);
      axial_vector(1) = 1.0;
    }
    else if (_axis == Axis::Z)
    {
      normal_vector(0) = _q_point[_qp](0);
      normal_vector(1) = _q_point[_qp](1);
      normal_vector(2) = 0.0;
      axial_vector(2) = 1.0;
    }
    else
      mooseError("\nInvalid value for axis parameter provided in HillElastoPlasticityStressUpdate");
  }

  if (_local_cylindrical_csys)
  {
    Real nv_norm = normal_vector.norm();
    Real av_norm = axial_vector.norm();

    if (!(MooseUtils::absoluteFuzzyEqual(nv_norm, 0.0)))
      normal_vector /= nv_norm;
    else
    {
      mooseError("The normal vector cannot be a zero vector in "
                 "HillElastoPlasticityStressUpdate");
    }

    if (!(MooseUtils::absoluteFuzzyEqual(av_norm, 0.0)))
      axial_vector /= av_norm;
    else
    {
      mooseError("The axial vector cannot be a zero vector in "
                 "HillElastoPlasticityStressUpdate");
    }

    RankTwoScalarTools::setRotationMatrix(
        normal_vector, axial_vector, _rotation_matrix[_qp], false);
    _rotation_matrix_transpose[_qp] = _rotation_matrix[_qp].transpose();
  }

  AnisotropicReturnPlasticityStressUpdateBaseTempl<is_ad>::initQpStatefulProperties();
}

template <bool is_ad>
void
HillElastoPlasticityStressUpdateTempl<is_ad>::propagateQpStatefulProperties()
{
  _hardening_variable[_qp] = _hardening_variable_old[_qp];
  _plasticity_strain[_qp] = _plasticity_strain_old[_qp];

  if (_local_cylindrical_csys)
  {
    _rotation_matrix[_qp] = _rotation_matrix_old[_qp];
    _rotation_matrix_transpose[_qp] = _rotation_matrix_transpose_old[_qp];
  }

  AnisotropicReturnPlasticityStressUpdateBaseTempl<is_ad>::propagateQpStatefulProperties();
}

template <bool is_ad>
void
HillElastoPlasticityStressUpdateTempl<is_ad>::computeStressInitialize(
    const GenericDenseVector<is_ad> & stress_dev,
    const GenericDenseVector<is_ad> & stress,
    const GenericRankFourTensor<is_ad> & /*elasticity_tensor*/)
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
  _sigma_tilde_rotated[_qp].resize(6);

  // Algebra needed for the generalized return mapping of anisotropic elastoplasticity
  computeElasticityTensorEigenDecomposition();

  _yield_condition = 1.0; // Some positive value
  _yield_condition = -computeResidual(stress_dev, stress, 0.0);
}

template <bool is_ad>
void
HillElastoPlasticityStressUpdateTempl<is_ad>::computeElasticityTensorEigenDecomposition()
{
  // Helper method to compute qp matrices required for the elasto-plasticity algorithm.

  GenericRankFourTensor<is_ad> elasticity_tensor = _elasticity_tensor[_qp];

  if (_local_cylindrical_csys)
  {
    elasticity_tensor.rotate(_rotation_matrix_transpose[_qp]);
  }

  ElasticityTensorTools::toVoigtNotation<is_ad>(_anisotropic_elastic_tensor, elasticity_tensor);

  const unsigned int dimension = _anisotropic_elastic_tensor.n();

  AnisotropyMatrixReal A = AnisotropyMatrixReal::Zero();
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
  GenericDenseMatrix<is_ad> sqrt_Delta(6, 6);
  for (unsigned int i = 0; i < 6; i++)
  {
    sqrt_Delta(i, i) = std::sqrt(_elasticity_eigenvalues[_qp](i));
  }

  GenericDenseMatrix<is_ad> eigenvectors_elasticity_transpose(6, 6);
  _elasticity_eigenvectors[_qp].get_transpose(eigenvectors_elasticity_transpose);

  GenericDenseMatrix<is_ad> b_matrix(6, 6);
  b_matrix = _hill_tensor[_qp];

  // Right multiply by matrix of eigenvectors transpose
  b_matrix.right_multiply(_elasticity_eigenvectors[_qp]);
  b_matrix.right_multiply(sqrt_Delta);
  b_matrix.left_multiply(eigenvectors_elasticity_transpose);
  b_matrix.left_multiply(sqrt_Delta);

  AnisotropyMatrixReal B_eigen = AnisotropyMatrixReal::Zero();
  for (unsigned int index_i = 0; index_i < dimension; index_i++)
    for (unsigned int index_j = 0; index_j < dimension; index_j++)
      B_eigen(index_i, index_j) = MetaPhysicL::raw_value(b_matrix(index_i, index_j));

  if (isBlockDiagonal(B_eigen))
  {
    Eigen::SelfAdjointEigenSolver<AnisotropyMatrixRealBlock> es_b_left(B_eigen.block<3, 3>(0, 0));
    Eigen::SelfAdjointEigenSolver<AnisotropyMatrixRealBlock> es_b_right(B_eigen.block<3, 3>(3, 3));

    auto lambda_b_left = es_b_left.eigenvalues();
    auto v_b_left = es_b_left.eigenvectors();

    auto lambda_b_right = es_b_right.eigenvalues();
    auto v_b_right = es_b_right.eigenvectors();

    _b_eigenvalues[_qp](0) = lambda_b_left(0);
    _b_eigenvalues[_qp](1) = lambda_b_left(1);
    _b_eigenvalues[_qp](2) = lambda_b_left(2);
    _b_eigenvalues[_qp](3) = lambda_b_right(0);
    _b_eigenvalues[_qp](4) = lambda_b_right(1);
    _b_eigenvalues[_qp](5) = lambda_b_right(2);

    _b_eigenvectors[_qp](0, 0) = v_b_left(0, 0);
    _b_eigenvectors[_qp](0, 1) = v_b_left(0, 1);
    _b_eigenvectors[_qp](0, 2) = v_b_left(0, 2);
    _b_eigenvectors[_qp](1, 0) = v_b_left(1, 0);
    _b_eigenvectors[_qp](1, 1) = v_b_left(1, 1);
    _b_eigenvectors[_qp](1, 2) = v_b_left(1, 2);
    _b_eigenvectors[_qp](2, 0) = v_b_left(2, 0);
    _b_eigenvectors[_qp](2, 1) = v_b_left(2, 1);
    _b_eigenvectors[_qp](2, 2) = v_b_left(2, 2);
    _b_eigenvectors[_qp](3, 3) = v_b_right(0, 0);
    _b_eigenvectors[_qp](3, 4) = v_b_right(0, 1);
    _b_eigenvectors[_qp](3, 5) = v_b_right(0, 2);
    _b_eigenvectors[_qp](4, 3) = v_b_right(1, 0);
    _b_eigenvectors[_qp](4, 4) = v_b_right(1, 1);
    _b_eigenvectors[_qp](4, 5) = v_b_right(1, 2);
    _b_eigenvectors[_qp](5, 3) = v_b_right(2, 0);
    _b_eigenvectors[_qp](5, 4) = v_b_right(2, 1);
    _b_eigenvectors[_qp](5, 5) = v_b_right(2, 2);
  }
  else
  {
    Eigen::SelfAdjointEigenSolver<AnisotropyMatrixReal> es_b(B_eigen);

    auto lambda_b = es_b.eigenvalues();
    auto v_b = es_b.eigenvectors();
    for (unsigned int index_i = 0; index_i < dimension; index_i++)
      _b_eigenvalues[_qp](index_i) = lambda_b(index_i);

    for (unsigned int index_i = 0; index_i < dimension; index_i++)
      for (unsigned int index_j = 0; index_j < dimension; index_j++)
        _b_eigenvectors[_qp](index_i, index_j) = v_b(index_i, index_j);
  }

  _alpha_matrix[_qp] = sqrt_Delta;
  _alpha_matrix[_qp].right_multiply(_b_eigenvectors[_qp]);
  _alpha_matrix[_qp].left_multiply(_elasticity_eigenvectors[_qp]);
}

template <bool is_ad>
GenericReal<is_ad>
HillElastoPlasticityStressUpdateTempl<is_ad>::computeOmega(
    const GenericReal<is_ad> & delta_gamma, const GenericDenseVector<is_ad> & /*stress_trial*/)
{
  GenericDenseVector<is_ad> K(6);
  GenericReal<is_ad> omega = 0.0;

  for (unsigned int i = 0; i < 6; i++)
  {
    K(i) = _b_eigenvalues[_qp](i) / Utility::pow<2>(1 + delta_gamma * _b_eigenvalues[_qp](i));

    if (_local_cylindrical_csys)
      omega += K(i) * _sigma_tilde_rotated[_qp](i) * _sigma_tilde_rotated[_qp](i);
    else
      omega += K(i) * _sigma_tilde[_qp](i) * _sigma_tilde[_qp](i);
  }
  omega *= 0.5;

  if (omega == 0.0)
    omega = 1.0e-36;

  return std::sqrt(omega);
}

template <bool is_ad>
Real
HillElastoPlasticityStressUpdateTempl<is_ad>::computeReferenceResidual(
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
HillElastoPlasticityStressUpdateTempl<is_ad>::computeResidual(
    const GenericDenseVector<is_ad> & /*stress_dev*/,
    const GenericDenseVector<is_ad> & stress_sigma,
    const GenericReal<is_ad> & delta_gamma)
{

  // If in elastic regime, just return
  if (_yield_condition <= 0.0)
    return 0.0;

  GenericReal<is_ad> omega;

  if (_local_cylindrical_csys)
  {
    // Get stress_tilde_rotated
    GenericRankTwoTensor<is_ad> stress;
    RankTwoScalarTools::VoigtVectorToRankTwoTensor<is_ad>(stress_sigma, stress);

    stress.rotate(_rotation_matrix_transpose[_qp]);

    GenericDenseVector<is_ad> stress_sigma_rotated(6);
    RankTwoScalarTools::RankTwoTensorToVoigtVector<is_ad>(stress, stress_sigma_rotated);

    GenericDenseVector<is_ad> stress_tilde_rotated(6);
    GenericDenseMatrix<is_ad> alpha_temp_rotated(_alpha_matrix[_qp]);
    alpha_temp_rotated.lu_solve(stress_sigma_rotated, stress_tilde_rotated);

    // Material property used in computeStressFinalize
    _sigma_tilde_rotated[_qp] = stress_tilde_rotated;
    omega = computeOmega(delta_gamma, stress_tilde_rotated);
  }

  else
  {
    // Get stress_tilde
    GenericDenseVector<is_ad> stress_tilde(6);
    GenericDenseMatrix<is_ad> alpha_temp(_alpha_matrix[_qp]);
    alpha_temp.lu_solve(stress_sigma, stress_tilde);

    // Material property used in computeStressFinalize
    _sigma_tilde[_qp] = stress_tilde;
    omega = computeOmega(delta_gamma, stress_tilde);
  }

  // Hardening variable is \alpha isotropic hardening for now.
  _hardening_variable[_qp] = computeHardeningValue(delta_gamma, omega);

  // A small value of 1.0e-30 is added to the hardening variable to avoid numerical issues
  // related to hardening variable becoming negative early in the iteration leading to non-
  // convergence. Note that std::pow(x,n) requires x to be positive if n is less than 1.

  GenericReal<is_ad> s_y =
      _hardening_constant * std::pow(_hardening_variable[_qp] + 1.0e-30, _hardening_exponent) +
      _yield_stress;

  GenericReal<is_ad> residual = 0.0;
  residual = s_y / omega - 1.0;

  return residual;
}

template <bool is_ad>
GenericReal<is_ad>
HillElastoPlasticityStressUpdateTempl<is_ad>::computeDerivative(
    const GenericDenseVector<is_ad> & /*stress_dev*/,
    const GenericDenseVector<is_ad> & stress_sigma,
    const GenericReal<is_ad> & delta_gamma)
{
  // If in elastic regime, return unit derivative
  if (_yield_condition <= 0.0)
    return 1.0;

  GenericReal<is_ad> omega = computeOmega(delta_gamma, stress_sigma);
  _hardening_derivative = computeHardeningDerivative();

  GenericReal<is_ad> hardeningVariable = computeHardeningValue(delta_gamma, omega);
  GenericReal<is_ad> sy =
      _hardening_constant * std::pow(hardeningVariable + 1.0e-30, _hardening_exponent) +
      _yield_stress;
  GenericReal<is_ad> sy_alpha = _hardening_derivative;

  GenericReal<is_ad> omega_gamma;
  GenericReal<is_ad> sy_gamma;

  computeDeltaDerivatives(delta_gamma, stress_sigma, sy_alpha, omega, omega_gamma, sy_gamma);
  GenericReal<is_ad> residual_derivative = 1 / omega * (sy_gamma - 1 / omega * omega_gamma * sy);

  return residual_derivative;
}

template <bool is_ad>
void
HillElastoPlasticityStressUpdateTempl<is_ad>::computeDeltaDerivatives(
    const GenericReal<is_ad> & delta_gamma,
    const GenericDenseVector<is_ad> & /*stress_trial*/,
    const GenericReal<is_ad> & sy_alpha,
    GenericReal<is_ad> & omega,
    GenericReal<is_ad> & omega_gamma,
    GenericReal<is_ad> & sy_gamma)
{
  omega_gamma = 0.0;
  sy_gamma = 0.0;

  GenericDenseVector<is_ad> K_deltaGamma(6);

  if (_local_cylindrical_csys)
    omega = computeOmega(delta_gamma, _sigma_tilde_rotated[_qp]);
  else
    omega = computeOmega(delta_gamma, _sigma_tilde[_qp]);

  GenericDenseVector<is_ad> K(6);
  for (unsigned int i = 0; i < 6; i++)
    K(i) = _b_eigenvalues[_qp](i) / Utility::pow<2>(1 + delta_gamma * _b_eigenvalues[_qp](i));

  for (unsigned int i = 0; i < 6; i++)
    K_deltaGamma(i) =
        -2.0 * _b_eigenvalues[_qp](i) * K(i) / (1 + _b_eigenvalues[_qp](i) * delta_gamma);

  for (unsigned int i = 0; i < 6; i++)
  {
    if (_local_cylindrical_csys)
      omega_gamma += K_deltaGamma(i) * _sigma_tilde_rotated[_qp](i) * _sigma_tilde_rotated[_qp](i);
    else
      omega_gamma += K_deltaGamma(i) * _sigma_tilde[_qp](i) * _sigma_tilde[_qp](i);
  }

  omega_gamma /= 4.0 * omega;
  sy_gamma = 2.0 * sy_alpha * (omega + delta_gamma * omega_gamma);
}

template <bool is_ad>
GenericReal<is_ad>
HillElastoPlasticityStressUpdateTempl<is_ad>::computeHardeningValue(
    const GenericReal<is_ad> & delta_gamma, const GenericReal<is_ad> & omega)
{
  return _hardening_variable_old[_qp] + 2.0 * delta_gamma * omega;
}

template <bool is_ad>
Real
HillElastoPlasticityStressUpdateTempl<is_ad>::computeHardeningDerivative()
{
  return _hardening_constant * _hardening_exponent *
         MetaPhysicL::raw_value(
             std::pow(_hardening_variable[_qp] + 1.0e-30, _hardening_exponent - 1));
}

template <bool is_ad>
void
HillElastoPlasticityStressUpdateTempl<is_ad>::computeStrainFinalize(
    GenericRankTwoTensor<is_ad> & inelasticStrainIncrement,
    const GenericRankTwoTensor<is_ad> & stress,
    const GenericDenseVector<is_ad> & stress_dev,
    const GenericReal<is_ad> & delta_gamma)
{
  // e^P = delta_gamma * hill_tensor * stress
  GenericDenseVector<is_ad> inelasticStrainIncrement_vector(6);
  GenericDenseVector<is_ad> hill_stress(6);
  GenericDenseVector<is_ad> stress_vector(6);

  if (_local_cylindrical_csys)
  {
    GenericRankTwoTensor<is_ad> stress_rotated(stress);
    stress_rotated.rotate(_rotation_matrix_transpose[_qp]);

    RankTwoScalarTools::RankTwoTensorToVoigtVector<is_ad>(stress_rotated, stress_vector);
  }
  else
  {
    RankTwoScalarTools::RankTwoTensorToVoigtVector<is_ad>(stress, stress_vector);
  }

  _hill_tensor[_qp].vector_mult(hill_stress, stress_vector);
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

  if (_local_cylindrical_csys)
    inelasticStrainIncrement.rotate(_rotation_matrix[_qp]);

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
HillElastoPlasticityStressUpdateTempl<is_ad>::computeStressFinalize(
    const GenericRankTwoTensor<is_ad> & /*plastic_strain_increment*/,
    const GenericReal<is_ad> & delta_gamma,
    GenericRankTwoTensor<is_ad> & stress_new,
    const GenericDenseVector<is_ad> & /*stress_dev*/,
    const GenericRankTwoTensor<is_ad> & /*stress_old*/,
    const GenericRankFourTensor<is_ad> & /*elasticity_tensor*/)
{
  // Need to compute this iteration's stress tensor based on the scalar variable
  // For deviatoric
  // sigma(n+1) = {Alpha [I + delta_gamma*Delta_b]^(-1) A^-1}  sigma(trial)

  if (_yield_condition <= 0.0)
    return;
  GenericDenseMatrix<is_ad> inv_matrix(6, 6);
  inv_matrix.zero();

  for (unsigned int i = 0; i < 6; i++)
    inv_matrix(i, i) = 1 / (1 + delta_gamma * _b_eigenvalues[_qp](i));

  _alpha_matrix[_qp].right_multiply(inv_matrix);

  GenericDenseVector<is_ad> stress_output(6);
  if (_local_cylindrical_csys)
    _alpha_matrix[_qp].vector_mult(stress_output, _sigma_tilde_rotated[_qp]);
  else
    _alpha_matrix[_qp].vector_mult(stress_output, _sigma_tilde[_qp]);

  RankTwoScalarTools::VoigtVectorToRankTwoTensor<is_ad>(stress_output, stress_new);

  if (_local_cylindrical_csys)
    stress_new.rotate(_rotation_matrix[_qp]);
}

template <bool is_ad>
Real
HillElastoPlasticityStressUpdateTempl<is_ad>::computeStrainEnergyRateDensity(
    const GenericMaterialProperty<RankTwoTensor, is_ad> & /*stress*/,
    const GenericMaterialProperty<RankTwoTensor, is_ad> & /*strain_rate*/)
{
  mooseError("computeStrainEnergyRateDensity not implemented for anisotropic plasticity.");
  return 0.0;
}

template class HillElastoPlasticityStressUpdateTempl<false>;
template class HillElastoPlasticityStressUpdateTempl<true>;
