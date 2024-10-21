//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "HillCreepStressUpdate.h"
#include "ElasticityTensorTools.h"
#include "Function.h"

registerMooseObject("SolidMechanicsApp", ADHillCreepStressUpdate);
registerMooseObject("SolidMechanicsApp", HillCreepStressUpdate);

template <bool is_ad>
InputParameters
HillCreepStressUpdateTempl<is_ad>::validParams()
{
  InputParameters params = AnisotropicReturnCreepStressUpdateBaseTempl<is_ad>::validParams();
  params.addClassDescription(
      "This class uses the stress update material in a generalized radial return anisotropic power "
      "law creep "
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
  params.addParam<bool>("anisotropic_elasticity", false, "Enable using anisotropic elasticity");
  params.addParam<FunctionName>(
      "creep_prefactor", "Optional function to use as a scalar prefactor on the creep strain.");
  return params;
}

template <bool is_ad>
HillCreepStressUpdateTempl<is_ad>::HillCreepStressUpdateTempl(const InputParameters & parameters)
  : AnisotropicReturnCreepStressUpdateBaseTempl<is_ad>(parameters),
    _has_temp(this->isParamValid("temperature")),
    _temperature(this->_has_temp ? this->coupledValue("temperature") : this->_zero),
    _coefficient(this->template getParam<Real>("coefficient")),
    _n_exponent(this->template getParam<Real>("n_exponent")),
    _m_exponent(this->template getParam<Real>("m_exponent")),
    _activation_energy(this->template getParam<Real>("activation_energy")),
    _gas_constant(this->template getParam<Real>("gas_constant")),
    _start_time(this->template getParam<Real>("start_time")),
    _exponential(1.0),
    _exp_time(1.0),
    _hill_constants(this->template getMaterialPropertyByName<std::vector<Real>>(this->_base_name +
                                                                                "hill_constants")),
    _hill_tensor(this->_use_transformation
                     ? &this->template getMaterialPropertyByName<DenseMatrix<Real>>(
                           this->_base_name + "hill_tensor")
                     : nullptr),
    _C(6, 6),
    _elasticity_tensor_name(this->_base_name + "elasticity_tensor"),
    _elasticity_tensor(
        this->template getGenericMaterialProperty<RankFourTensor, is_ad>(_elasticity_tensor_name)),
    _anisotropic_elasticity(this->template getParam<bool>("anisotropic_elasticity")),
    _prefactor_function(
        this->isParamValid("creep_prefactor") ? &this->getFunction("creep_prefactor") : nullptr)
{
  if (_start_time < this->_app.getStartTime() && (std::trunc(_m_exponent) != _m_exponent))
    this->template paramError(
        "start_time",
        "Start time must be equal to or greater than the Executioner start_time if a "
        "non-integer m_exponent is used");
}

template <bool is_ad>
void
HillCreepStressUpdateTempl<is_ad>::computeStressInitialize(
    const GenericDenseVector<is_ad> & /*stress_dev*/,
    const GenericDenseVector<is_ad> & /*stress*/,
    const GenericRankFourTensor<is_ad> & elasticity_tensor)
{
  if (_has_temp)
    _exponential = std::exp(-_activation_energy / (_gas_constant * _temperature[_qp]));

  _two_shear_modulus = 2.0 * ElasticityTensorTools::getIsotropicShearModulus(elasticity_tensor);

  _exp_time = std::pow(_t - _start_time, _m_exponent);
}

template <bool is_ad>
GenericReal<is_ad>
HillCreepStressUpdateTempl<is_ad>::initialGuess(const GenericDenseVector<is_ad> & /*stress_dev*/)
{
  return 0.0;
}

template <bool is_ad>
GenericReal<is_ad>
HillCreepStressUpdateTempl<is_ad>::computeResidual(
    const GenericDenseVector<is_ad> & /*effective_trial_stress*/,
    const GenericDenseVector<is_ad> & stress_new,
    const GenericReal<is_ad> & delta_gamma)
{
  GenericReal<is_ad> qsigma_changed;
  GenericRankTwoTensor<is_ad> stress_changed;

  // Get the qsigma_changed and stress_changed (not used)
  // delta_gamma (scalar plastic strain) will change the stress (and qsigma as well)
  computeQsigmaChanged(qsigma_changed, stress_new, delta_gamma, stress_changed);

  GenericReal<is_ad> creep_rate =
      _coefficient * std::pow(qsigma_changed, _n_exponent) * _exponential * _exp_time;

  if (_prefactor_function)
    creep_rate *= _prefactor_function->value(_t, _q_point[_qp]);

  this->_inelastic_strain_rate[_qp] = MetaPhysicL::raw_value(creep_rate);
  // Return iteration difference between creep strain and inelastic strain multiplier
  return creep_rate * _dt - delta_gamma;
}

template <bool is_ad>
Real
HillCreepStressUpdateTempl<is_ad>::computeReferenceResidual(
    const GenericDenseVector<is_ad> & /*effective_trial_stress*/,
    const GenericDenseVector<is_ad> & /*stress_new*/,
    const GenericReal<is_ad> & /*residual*/,
    const GenericReal<is_ad> & /*scalar_effective_inelastic_strain*/)
{
  return 1.0;
}

template <bool is_ad>
GenericReal<is_ad>
HillCreepStressUpdateTempl<is_ad>::computeDerivative(
    const GenericDenseVector<is_ad> & /*effective_trial_stress*/,
    const GenericDenseVector<is_ad> & stress_new,
    const GenericReal<is_ad> & delta_gamma)
{
  GenericReal<is_ad> qsigma_changed;
  GenericRankTwoTensor<is_ad> stress_changed;

  // Get the qsigma_changed and stress_changed
  // delta_gamma (scalar plastic strain) will change the stress (and qsigma as well)
  computeQsigmaChanged(qsigma_changed, stress_new, delta_gamma, stress_changed);

  ElasticityTensorTools::toMooseVoigtNotation<is_ad>(_C, _elasticity_tensor[_qp]);
  const unsigned int dimension = _C.n();
  GenericDenseMatrix<is_ad> d_stress_d_inelasticStrainIncrement(dimension, dimension);

  for (unsigned int index_i = 0; index_i < dimension; index_i++)
    for (unsigned int index_j = 0; index_j < dimension; index_j++)
    {
      if (index_j < 3)
        d_stress_d_inelasticStrainIncrement(index_i, index_j) =
            -1.0 * MetaPhysicL::raw_value(_C(index_i, index_j));
      else
        d_stress_d_inelasticStrainIncrement(index_i, index_j) =
            -2.0 * MetaPhysicL::raw_value(_C(index_i, index_j));
    }

  // Hill constants, some constraints apply
  const Real & F = _hill_constants[_qp][0];
  const Real & G = _hill_constants[_qp][1];
  const Real & H = _hill_constants[_qp][2];
  const Real & L = _hill_constants[_qp][3];
  const Real & M = _hill_constants[_qp][4];
  const Real & N = _hill_constants[_qp][5];

  GenericDenseVector<is_ad> d_qsigma_d_inelasticStrainIncrement(6);
  for (unsigned int index_k = 0; index_k < 6; index_k++)
  {
    d_qsigma_d_inelasticStrainIncrement(index_k) =
        F * (stress_changed(1, 1) - stress_changed(2, 2)) *
            (d_stress_d_inelasticStrainIncrement(1, index_k) -
             d_stress_d_inelasticStrainIncrement(2, index_k)) +
        G * (stress_changed(2, 2) - stress_changed(0, 0)) *
            (d_stress_d_inelasticStrainIncrement(2, index_k) -
             d_stress_d_inelasticStrainIncrement(0, index_k)) +
        H * (stress_changed(0, 0) - stress_changed(1, 1)) *
            (d_stress_d_inelasticStrainIncrement(0, index_k) -
             d_stress_d_inelasticStrainIncrement(1, index_k)) +
        2.0 * L * stress_changed(1, 2) * d_stress_d_inelasticStrainIncrement(4, index_k) +
        2.0 * M * stress_changed(2, 0) * d_stress_d_inelasticStrainIncrement(5, index_k) +
        2.0 * N * stress_changed(0, 1) * d_stress_d_inelasticStrainIncrement(3, index_k);
    d_qsigma_d_inelasticStrainIncrement(index_k) /= qsigma_changed;
  }

  GenericDenseVector<is_ad> d_qsigma_d_sigma(6);

  d_qsigma_d_sigma(0) = (H * (stress_changed(0, 0) - stress_changed(1, 1)) -
                         G * (stress_changed(2, 2) - stress_changed(0, 0))) /
                        qsigma_changed;
  d_qsigma_d_sigma(1) = (F * (stress_changed(1, 1) - stress_changed(2, 2)) -
                         H * (stress_changed(0, 0) - stress_changed(1, 1))) /
                        qsigma_changed;
  d_qsigma_d_sigma(2) = (G * (stress_changed(2, 2) - stress_changed(0, 0)) -
                         F * (stress_changed(1, 1) - stress_changed(2, 2))) /
                        qsigma_changed;
  d_qsigma_d_sigma(3) = 2.0 * N * stress_changed(0, 1) / qsigma_changed;
  d_qsigma_d_sigma(4) = 2.0 * L * stress_changed(1, 2) / qsigma_changed;
  d_qsigma_d_sigma(5) = 2.0 * M * stress_changed(0, 2) / qsigma_changed;

  GenericReal<is_ad> d_qsigma_d_deltaGamma =
      d_qsigma_d_inelasticStrainIncrement.dot(d_qsigma_d_sigma);

  GenericReal<is_ad> creep_rate_derivative = _coefficient * d_qsigma_d_deltaGamma * _n_exponent *
                                             std::pow(qsigma_changed, _n_exponent - 1.0) *
                                             _exponential * _exp_time;

  if (_prefactor_function)
    creep_rate_derivative *= _prefactor_function->value(_t, _q_point[_qp]);

  return (creep_rate_derivative * _dt - 1.0);
}

template <bool is_ad>
void
HillCreepStressUpdateTempl<is_ad>::computeStrainFinalize(
    GenericRankTwoTensor<is_ad> & inelasticStrainIncrement,
    const GenericRankTwoTensor<is_ad> & stress,
    const GenericDenseVector<is_ad> & stress_dev,
    const GenericReal<is_ad> & delta_gamma)
{
  GenericReal<is_ad> qsigma_square;
  if (!this->_use_transformation)
  {
    // Hill constants, some constraints apply
    const Real & F = _hill_constants[_qp][0];
    const Real & G = _hill_constants[_qp][1];
    const Real & H = _hill_constants[_qp][2];
    const Real & L = _hill_constants[_qp][3];
    const Real & M = _hill_constants[_qp][4];
    const Real & N = _hill_constants[_qp][5];

    // Equivalent deviatoric stress function.
    qsigma_square = F * (stress(1, 1) - stress(2, 2)) * (stress(1, 1) - stress(2, 2));
    qsigma_square += G * (stress(2, 2) - stress(0, 0)) * (stress(2, 2) - stress(0, 0));
    qsigma_square += H * (stress(0, 0) - stress(1, 1)) * (stress(0, 0) - stress(1, 1));
    qsigma_square += 2 * L * stress(1, 2) * stress(1, 2);
    qsigma_square += 2 * M * stress(0, 2) * stress(0, 2);
    qsigma_square += 2 * N * stress(0, 1) * stress(0, 1);
  }
  else
  {
    GenericDenseVector<is_ad> Ms(6);
    (*_hill_tensor)[_qp].vector_mult(Ms, stress_dev);
    qsigma_square = Ms.dot(stress_dev);
  }

  if (qsigma_square == 0)
  {
    inelasticStrainIncrement.zero();

    AnisotropicReturnCreepStressUpdateBaseTempl<is_ad>::computeStrainFinalize(
        inelasticStrainIncrement, stress, stress_dev, delta_gamma);

    this->_effective_inelastic_strain[_qp] = this->_effective_inelastic_strain_old[_qp];

    return;
  }

  // Use Hill-type flow rule to compute the time step inelastic increment.
  GenericReal<is_ad> prefactor = delta_gamma / std::sqrt(qsigma_square);

  if (!this->_use_transformation)
  {
    // Hill constants, some constraints apply
    const Real & F = _hill_constants[_qp][0];
    const Real & G = _hill_constants[_qp][1];
    const Real & H = _hill_constants[_qp][2];
    const Real & L = _hill_constants[_qp][3];
    const Real & M = _hill_constants[_qp][4];
    const Real & N = _hill_constants[_qp][5];

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
  }
  else
  {
    GenericDenseVector<is_ad> inelastic_strain_increment(6);
    GenericDenseVector<is_ad> Ms(6);
    (*_hill_tensor)[_qp].vector_mult(Ms, stress_dev);

    for (unsigned int i = 0; i < 6; i++)
      inelastic_strain_increment(i) = Ms(i) * prefactor;

    inelasticStrainIncrement(0, 0) = inelastic_strain_increment(0);
    inelasticStrainIncrement(1, 1) = inelastic_strain_increment(1);
    inelasticStrainIncrement(2, 2) = inelastic_strain_increment(2);

    inelasticStrainIncrement(0, 1) = inelasticStrainIncrement(1, 0) = inelastic_strain_increment(3);
    inelasticStrainIncrement(1, 2) = inelasticStrainIncrement(2, 1) = inelastic_strain_increment(4);
    inelasticStrainIncrement(0, 2) = inelasticStrainIncrement(2, 0) = inelastic_strain_increment(5);
  }

  AnisotropicReturnCreepStressUpdateBaseTempl<is_ad>::computeStrainFinalize(
      inelasticStrainIncrement, stress, stress_dev, delta_gamma);

  this->_effective_inelastic_strain[_qp] = this->_effective_inelastic_strain_old[_qp] + delta_gamma;
}

template <bool is_ad>
void
HillCreepStressUpdateTempl<is_ad>::computeStressFinalize(
    const GenericRankTwoTensor<is_ad> & creepStrainIncrement,
    const GenericReal<is_ad> & /*delta_gamma*/,
    GenericRankTwoTensor<is_ad> & stress_new,
    const GenericDenseVector<is_ad> & /*stress_dev*/,
    const GenericRankTwoTensor<is_ad> & stress_old,
    const GenericRankFourTensor<is_ad> & elasticity_tensor)
{
  // Class only valid for isotropic elasticity (for now)
  stress_new -= elasticity_tensor * creepStrainIncrement;

  // Compute the maximum time step allowed due to creep strain numerical integration error
  Real stress_dif = MetaPhysicL::raw_value(stress_new - stress_old).L2norm();

  // Get a representative value of the elasticity tensor
  Real elasticity_value =
      1.0 / 3.0 *
      MetaPhysicL::raw_value((elasticity_tensor(0, 0, 0, 0) + elasticity_tensor(1, 1, 1, 1) +
                              elasticity_tensor(2, 2, 2, 2)));

  if (std::abs(stress_dif) > TOLERANCE * TOLERANCE)
    this->_max_integration_error_time_step =
        _dt / (stress_dif / elasticity_value / this->_max_integration_error);
  else
    this->_max_integration_error_time_step = std::numeric_limits<Real>::max();
}

template <bool is_ad>
void
HillCreepStressUpdateTempl<is_ad>::computeQsigmaChanged(
    GenericReal<is_ad> & qsigma_changed,
    const GenericDenseVector<is_ad> & stress_new,
    const GenericReal<is_ad> & delta_gamma,
    GenericRankTwoTensor<is_ad> & stress_changed)
{
  GenericReal<is_ad> qsigma_square;

  // Hill constants, some constraints apply
  const Real & F = _hill_constants[_qp][0];
  const Real & G = _hill_constants[_qp][1];
  const Real & H = _hill_constants[_qp][2];
  const Real & L = _hill_constants[_qp][3];
  const Real & M = _hill_constants[_qp][4];
  const Real & N = _hill_constants[_qp][5];

  if (!this->_use_transformation)
  {
    qsigma_square = F * (stress_new(1) - stress_new(2)) * (stress_new(1) - stress_new(2));
    qsigma_square += G * (stress_new(2) - stress_new(0)) * (stress_new(2) - stress_new(0));
    qsigma_square += H * (stress_new(0) - stress_new(1)) * (stress_new(0) - stress_new(1));
    qsigma_square += 2 * L * stress_new(4) * stress_new(4);
    qsigma_square += 2 * M * stress_new(5) * stress_new(5);
    qsigma_square += 2 * N * stress_new(3) * stress_new(3);
  }
  else
  {
    GenericDenseVector<is_ad> Ms(6);
    (*_hill_tensor)[_qp].vector_mult(Ms, stress_new);
    qsigma_square = Ms.dot(stress_new);
  }

  GenericReal<is_ad> qsigma = std::sqrt(qsigma_square);

  if (!_anisotropic_elasticity)
    qsigma_changed = qsigma - 1.5 * _two_shear_modulus * delta_gamma;
  else
  {
    GenericRankTwoTensor<is_ad> stress;
    stress(0, 0) = stress_new(0);
    stress(1, 1) = stress_new(1);
    stress(2, 2) = stress_new(2);
    stress(0, 1) = stress(1, 0) = stress_new(3);
    stress(1, 2) = stress(2, 1) = stress_new(4);
    stress(0, 2) = stress(2, 0) = stress_new(5);

    GenericDenseVector<is_ad> b(6);
    b(0) = H * (stress(0, 0) - stress(1, 1)) - G * (stress(2, 2) - stress(0, 0));
    b(1) = F * (stress(1, 1) - stress(2, 2)) - H * (stress(0, 0) - stress(1, 1));
    b(2) = G * (stress(2, 2) - stress(0, 0)) - F * (stress(1, 1) - stress(2, 2));
    b(3) = 2.0 * N * stress(0, 1);
    b(4) = 2.0 * L * stress(1, 2);
    b(5) = 2.0 * M * stress(0, 2);

    GenericRankTwoTensor<is_ad> inelasticStrainIncrement;
    inelasticStrainIncrement(0, 0) = delta_gamma * b(0) / qsigma;
    inelasticStrainIncrement(1, 1) = delta_gamma * b(1) / qsigma;
    inelasticStrainIncrement(2, 2) = delta_gamma * b(2) / qsigma;
    inelasticStrainIncrement(0, 1) = inelasticStrainIncrement(1, 0) = delta_gamma * b(3) / qsigma;
    inelasticStrainIncrement(1, 2) = inelasticStrainIncrement(2, 1) = delta_gamma * b(4) / qsigma;
    inelasticStrainIncrement(0, 2) = inelasticStrainIncrement(2, 0) = delta_gamma * b(5) / qsigma;

    stress_changed = stress - _elasticity_tensor[_qp] * inelasticStrainIncrement;

    GenericReal<is_ad> qsigma_square_changed;
    qsigma_square_changed = F * (stress_changed(1, 1) - stress_changed(2, 2)) *
                            (stress_changed(1, 1) - stress_changed(2, 2));
    qsigma_square_changed += G * (stress_changed(2, 2) - stress_changed(0, 0)) *
                             (stress_changed(2, 2) - stress_changed(0, 0));
    qsigma_square_changed += H * (stress_changed(0, 0) - stress_changed(1, 1)) *
                             (stress_changed(0, 0) - stress_changed(1, 1));
    qsigma_square_changed += 2 * L * stress_changed(1, 2) * stress_changed(1, 2);
    qsigma_square_changed += 2 * M * stress_changed(0, 2) * stress_changed(0, 2);
    qsigma_square_changed += 2 * N * stress_changed(0, 1) * stress_changed(0, 1);
    qsigma_changed = std::sqrt(qsigma_square_changed);
  }
}

template class HillCreepStressUpdateTempl<false>;
template class HillCreepStressUpdateTempl<true>;
