
//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "RadialReturnStressUpdate.h"

#include "MooseMesh.h"
#include "ElasticityTensorTools.h"

template <bool is_ad>
InputParameters
RadialReturnStressUpdateTempl<is_ad>::validParams()
{
  InputParameters params = StressUpdateBaseTempl<is_ad>::validParams();
  params.addClassDescription("Calculates the effective inelastic strain increment required to "
                             "return the isotropic stress state to a J2 yield surface.  This class "
                             "is intended to be a parent class for classes with specific "
                             "constitutive models.");
  params += SingleVariableReturnMappingSolutionTempl<is_ad>::validParams();
  params.addParam<Real>("max_inelastic_increment",
                        1e-4,
                        "The maximum inelastic strain increment allowed in a time step");
  params.addRequiredParam<std::string>(
      "effective_inelastic_strain_name",
      "Name of the material property that stores the effective inelastic strain");
  params.addParam<bool>("use_substep", false, "Whether to use substepping");
  params.addParam<Real>("substep_strain_tolerance",
                        0.1,
                        "Maximum ratio of the initial elastic strain increment at start of the "
                        "return mapping solve to the maximum inelastic strain allowable in a "
                        "single substep. Reduce this value to increase the number of substeps");
  params.addParam<bool>("apply_strain", true, "Flag to apply strain. Used for testing.");
  params.addParamNamesToGroup(
      "effective_inelastic_strain_name substep_strain_tolerance apply_strain", "Advanced");
  return params;
}

template <bool is_ad>
RadialReturnStressUpdateTempl<is_ad>::RadialReturnStressUpdateTempl(
    const InputParameters & parameters)
  : StressUpdateBaseTempl<is_ad>(parameters),
    SingleVariableReturnMappingSolutionTempl<is_ad>(parameters),
    _effective_inelastic_strain(this->template declareGenericProperty<Real, is_ad>(
        this->_base_name +
        this->template getParam<std::string>("effective_inelastic_strain_name"))),
    _effective_inelastic_strain_old(this->template getMaterialPropertyOld<Real>(
        this->_base_name +
        this->template getParam<std::string>("effective_inelastic_strain_name"))),
    _max_inelastic_increment(this->template getParam<Real>("max_inelastic_increment")),
    _substep_tolerance(this->template getParam<Real>("substep_strain_tolerance")),
    _identity_two(RankTwoTensor::initIdentity),
    _identity_symmetric_four(RankFourTensor::initIdentitySymmetricFour),
    _deviatoric_projection_four(_identity_symmetric_four -
                                _identity_two.outerProduct(_identity_two) / 3.0),
    _apply_strain(this->template getParam<bool>("apply_strain"))
{
}

template <bool is_ad>
void
RadialReturnStressUpdateTempl<is_ad>::initQpStatefulProperties()
{
  _effective_inelastic_strain[_qp] = 0.0;
}

template <bool is_ad>
void
RadialReturnStressUpdateTempl<is_ad>::propagateQpStatefulPropertiesRadialReturn()
{
  _effective_inelastic_strain[_qp] = _effective_inelastic_strain_old[_qp];
}

template <bool is_ad>
int
RadialReturnStressUpdateTempl<is_ad>::calculateNumberSubsteps(
    const GenericRankTwoTensor<is_ad> & strain_increment)
{
  unsigned int substep_number = 1;
  // compute an effective elastic strain measure
  const GenericReal<is_ad> contracted_elastic_strain =
      strain_increment.doubleContraction(strain_increment);
  const Real effective_elastic_strain =
      std::sqrt(3.0 / 2.0 * MetaPhysicL::raw_value(contracted_elastic_strain));

  if (!MooseUtils::absoluteFuzzyEqual(effective_elastic_strain, 0.0))
  {
    const Real ratio = effective_elastic_strain / _max_inelastic_increment;

    if (ratio > _substep_tolerance)
      substep_number = std::ceil(ratio / _substep_tolerance);
  }

  return substep_number;
}

template <bool is_ad>
void
RadialReturnStressUpdateTempl<is_ad>::computeTangentOperator(Real /*effective_trial_stress*/,
                                                             RankTwoTensor & /*stress_new*/,
                                                             bool /*compute_full_tangent_operator*/,
                                                             RankFourTensor & /*tangent_operator*/)
{
  mooseError("computeTangentOperator called: no tangent computation is needed for AD");
}

template <>
void
RadialReturnStressUpdateTempl<false>::computeTangentOperator(Real effective_trial_stress,
                                                             RankTwoTensor & stress_new,
                                                             bool compute_full_tangent_operator,
                                                             RankFourTensor & tangent_operator)
{
  if (compute_full_tangent_operator &&
      getTangentCalculationMethod() == TangentCalculationMethod::PARTIAL)
  {
    if (MooseUtils::absoluteFuzzyEqual(_scalar_effective_inelastic_strain, 0.0))
      tangent_operator.zero();
    else
    {
      // mu = _three_shear_modulus / 3.0;
      // norm_dev_stress = ||s_n+1||
      // effective_trial_stress = von mises trial stress = std::sqrt(3.0 / 2.0) * ||s_n+1^trial||
      // scalar_effective_inelastic_strain = Delta epsilon^cr_n+1
      // deriv = derivative of scalar_effective_inelastic_strain w.r.t. von mises stress
      // deriv = std::sqrt(3.0 / 2.0) partial Delta epsilon^cr_n+1n over partial ||s_n+1^trial||

      mooseAssert(_three_shear_modulus != 0.0, "Shear modulus is zero");

      const RankTwoTensor deviatoric_stress = stress_new.deviatoric();
      const Real norm_dev_stress =
          std::sqrt(deviatoric_stress.doubleContraction(deviatoric_stress));
      mooseAssert(norm_dev_stress != 0.0, "Norm of the deviatoric is zero");

      const RankTwoTensor flow_direction = deviatoric_stress / norm_dev_stress;
      const RankFourTensor flow_direction_dyad = flow_direction.outerProduct(flow_direction);
      const Real deriv =
          computeStressDerivative(effective_trial_stress, _scalar_effective_inelastic_strain);
      const Real scalar_one = _three_shear_modulus * _scalar_effective_inelastic_strain /
                              std::sqrt(1.5) / norm_dev_stress;

      tangent_operator = scalar_one * _deviatoric_projection_four +
                         (_three_shear_modulus * deriv - scalar_one) * flow_direction_dyad;
    }
  }
}

template <>
void
RadialReturnStressUpdateTempl<false>::updateState(RankTwoTensor & strain_increment,
                                                  RankTwoTensor & inelastic_strain_increment,
                                                  const RankTwoTensor & /*rotation_increment*/,
                                                  RankTwoTensor & stress_new,
                                                  const RankTwoTensor & /*stress_old*/,
                                                  const RankFourTensor & elasticity_tensor,
                                                  const RankTwoTensor & elastic_strain_old,
                                                  bool compute_full_tangent_operator,
                                                  RankFourTensor & tangent_operator)
{
  // compute the deviatoric trial stress and trial strain from the current intermediate
  // configuration
  RankTwoTensor deviatoric_trial_stress = stress_new.deviatoric();

  // compute the effective trial stress
  Real dev_trial_stress_squared =
      deviatoric_trial_stress.doubleContraction(deviatoric_trial_stress);
  Real effective_trial_stress = std::sqrt(3.0 / 2.0 * dev_trial_stress_squared);

  // Set the value of 3 * shear modulus for use as a reference residual value
  _three_shear_modulus = 3.0 * ElasticityTensorTools::getIsotropicShearModulus(elasticity_tensor);

  computeStressInitialize(effective_trial_stress, elasticity_tensor);

  // Use Newton iteration to determine the scalar effective inelastic strain increment
  _scalar_effective_inelastic_strain = 0.0;
  if (!MooseUtils::absoluteFuzzyEqual(effective_trial_stress, 0.0))
  {
    returnMappingSolve(effective_trial_stress, _scalar_effective_inelastic_strain, _console);
    if (_scalar_effective_inelastic_strain != 0.0)
      inelastic_strain_increment =
          deviatoric_trial_stress *
          (1.5 * _scalar_effective_inelastic_strain / effective_trial_stress);
    else
      inelastic_strain_increment.zero();
  }
  else
    inelastic_strain_increment.zero();

  strain_increment -= inelastic_strain_increment;
  _effective_inelastic_strain[_qp] =
      _effective_inelastic_strain_old[_qp] + _scalar_effective_inelastic_strain;

  // Use the old elastic strain here because we require tensors used by this class
  // to be isotropic and this method natively allows for changing in time
  // elasticity tensors
  stress_new = elasticity_tensor * (strain_increment + elastic_strain_old);

  computeStressFinalize(inelastic_strain_increment);

  computeTangentOperator(
      effective_trial_stress, stress_new, compute_full_tangent_operator, tangent_operator);
}

template <>
void
RadialReturnStressUpdateTempl<true>::updateState(
    ADRankTwoTensor & strain_increment,
    ADRankTwoTensor & inelastic_strain_increment,
    const ADRankTwoTensor & /*rotation_increment*/,
    ADRankTwoTensor & stress_new,
    const RankTwoTensor & /*stress_old*/,
    const ADRankFourTensor & elasticity_tensor,
    const RankTwoTensor & elastic_strain_old,
    bool /*compute_full_tangent_operator = false*/,
    RankFourTensor & /*tangent_operator = _identityTensor*/)
{
  // compute the deviatoric trial stress and trial strain from the current intermediate
  // configuration
  ADRankTwoTensor deviatoric_trial_stress = stress_new.deviatoric();

  // compute the effective trial stress
  ADReal dev_trial_stress_squared =
      deviatoric_trial_stress.doubleContraction(deviatoric_trial_stress);
  ADReal effective_trial_stress =
      dev_trial_stress_squared == 0.0 ? 0.0 : std::sqrt(3.0 / 2.0 * dev_trial_stress_squared);

  // Set the value of 3 * shear modulus for use as a reference residual value
  _three_shear_modulus = 3.0 * ElasticityTensorTools::getIsotropicShearModulus(elasticity_tensor);

  computeStressInitialize(effective_trial_stress, elasticity_tensor);

  // Use Newton iteration to determine the scalar effective inelastic strain increment
  _scalar_effective_inelastic_strain = 0.0;
  if (!MooseUtils::absoluteFuzzyEqual(effective_trial_stress, 0.0))
  {
    returnMappingSolve(effective_trial_stress, _scalar_effective_inelastic_strain, _console);
    if (_scalar_effective_inelastic_strain != 0.0)
      inelastic_strain_increment =
          deviatoric_trial_stress *
          (1.5 * _scalar_effective_inelastic_strain / effective_trial_stress);
    else
      inelastic_strain_increment.zero();
  }
  else
    inelastic_strain_increment.zero();

  if (_apply_strain)
  {
    strain_increment -= inelastic_strain_increment;
    _effective_inelastic_strain[_qp] =
        _effective_inelastic_strain_old[_qp] + _scalar_effective_inelastic_strain;

    // Use the old elastic strain here because we require tensors used by this class
    // to be isotropic and this method natively allows for changing in time
    // elasticity tensors
    stress_new = elasticity_tensor * (elastic_strain_old + strain_increment);
  }

  computeStressFinalize(inelastic_strain_increment);
}

template <>
void
RadialReturnStressUpdateTempl<false>::updateStateSubstep(RankTwoTensor & strain_increment,
                                                         RankTwoTensor & inelastic_strain_increment,
                                                         const RankTwoTensor & rotation_increment,
                                                         RankTwoTensor & stress_new,
                                                         const RankTwoTensor & stress_old,
                                                         const RankFourTensor & elasticity_tensor,
                                                         const RankTwoTensor & elastic_strain_old,
                                                         bool compute_full_tangent_operator,
                                                         RankFourTensor & tangent_operator)
{
  const unsigned int total_substeps = calculateNumberSubsteps(strain_increment);

  // if only one substep is needed, then call the original update state method
  if (total_substeps == 1)
  {
    updateState(strain_increment,
                inelastic_strain_increment,
                rotation_increment,
                stress_new,
                stress_old,
                elasticity_tensor,
                elastic_strain_old,
                compute_full_tangent_operator,
                tangent_operator);
    return;
  }
  // Store original _dt; Reset at the end of solve
  Real dt_original = _dt;
  // cut the original timestep
  _dt = dt_original / total_substeps;

  // initialize the inputs
  const RankTwoTensor strain_increment_per_step = strain_increment / total_substeps;
  RankTwoTensor sub_stress_new = elasticity_tensor * elastic_strain_old;

  RankTwoTensor sub_elastic_strain_old = elastic_strain_old;
  RankTwoTensor sub_inelastic_strain_increment = inelastic_strain_increment;

  Real sub_scalar_effective_inelastic_strain = 0;

  // clear the original inputs
  MathUtils::mooseSetToZero(strain_increment);
  MathUtils::mooseSetToZero(inelastic_strain_increment);
  MathUtils::mooseSetToZero(stress_new);

  for (unsigned int step = 0; step < total_substeps; ++step)
  {
    // set up input for this substep
    RankTwoTensor sub_strain_increment = strain_increment_per_step;
    sub_stress_new += elasticity_tensor * sub_strain_increment;
    // compute effective_sub_stress_new
    RankTwoTensor deviatoric_sub_stress_new = sub_stress_new.deviatoric();
    Real dev_sub_stress_new_squared =
        deviatoric_sub_stress_new.doubleContraction(deviatoric_sub_stress_new);
    Real effective_sub_stress_new = std::sqrt(3.0 / 2.0 * dev_sub_stress_new_squared);

    // update stress and strain based on the strain increment
    updateState(sub_strain_increment,
                sub_inelastic_strain_increment,
                rotation_increment, // not used in updateState
                sub_stress_new,
                stress_old, // not used in updateState
                elasticity_tensor,
                elastic_strain_old,
                false, // do not compute tangent until the end of this substep
                tangent_operator);
    // update strain and stress
    strain_increment += sub_strain_increment;
    inelastic_strain_increment += sub_inelastic_strain_increment;
    sub_elastic_strain_old += sub_strain_increment;
    sub_stress_new = elasticity_tensor * sub_elastic_strain_old;
    // accumulate scalar_effective_inelastic_strain
    sub_scalar_effective_inelastic_strain += _scalar_effective_inelastic_strain;
    computeStressFinalize(inelastic_strain_increment);
    computeTangentOperator(
        effective_sub_stress_new, sub_stress_new, compute_full_tangent_operator, tangent_operator);
    // store incremental material properties for this step
    storeIncrementalMaterialProperties();
  }
  // update stress
  stress_new = sub_stress_new;
  // update effective inelastic strain
  _effective_inelastic_strain[_qp] =
      _effective_inelastic_strain_old[_qp] + sub_scalar_effective_inelastic_strain;

  // recover the original timestep
  _dt = dt_original;
}

template <>
void
RadialReturnStressUpdateTempl<true>::updateStateSubstep(
    ADRankTwoTensor & strain_increment,
    ADRankTwoTensor & inelastic_strain_increment,
    const ADRankTwoTensor & rotation_increment,
    ADRankTwoTensor & stress_new,
    const RankTwoTensor & stress_old,
    const ADRankFourTensor & elasticity_tensor,
    const RankTwoTensor & elastic_strain_old,
    bool /*compute_full_tangent_operator*/,
    RankFourTensor & /*tangent_operator*/)
{
  const unsigned int total_substeps = calculateNumberSubsteps(strain_increment);

  // if only one substep is needed, then call the original update state method
  if (total_substeps == 1)
  {
    updateState(strain_increment,
                inelastic_strain_increment,
                rotation_increment,
                stress_new,
                stress_old,
                elasticity_tensor,
                elastic_strain_old);
    return;
  }
  // Store original _dt; Reset at the end of solve
  Real dt_original = _dt;
  // cut the original timestep
  _dt = dt_original / total_substeps;

  // initialize the inputs
  const ADRankTwoTensor strain_increment_per_step = strain_increment / total_substeps;
  ADRankTwoTensor sub_stress_new = elasticity_tensor * elastic_strain_old;

  ADRankTwoTensor sub_elastic_strain_old = elastic_strain_old;
  ADRankTwoTensor sub_inelastic_strain_increment = inelastic_strain_increment;

  ADReal sub_scalar_effective_inelastic_strain = 0;

  // clear the original inputs
  MathUtils::mooseSetToZero(strain_increment);
  MathUtils::mooseSetToZero(inelastic_strain_increment);
  MathUtils::mooseSetToZero(stress_new);

  for (unsigned int step = 0; step < total_substeps; ++step)
  {
    // set up input for this substep
    ADRankTwoTensor sub_strain_increment = strain_increment_per_step;
    sub_stress_new += elasticity_tensor * sub_strain_increment;

    // update stress and strain based on the strain increment
    updateState(sub_strain_increment,
                sub_inelastic_strain_increment,
                rotation_increment, // not used in updateState
                sub_stress_new,
                stress_old, // not used in updateState
                elasticity_tensor,
                elastic_strain_old);
    // update strain and stress
    strain_increment += sub_strain_increment;
    inelastic_strain_increment += sub_inelastic_strain_increment;
    sub_elastic_strain_old += sub_strain_increment;
    sub_stress_new = elasticity_tensor * sub_elastic_strain_old;
    // accumulate scalar_effective_inelastic_strain
    sub_scalar_effective_inelastic_strain += _scalar_effective_inelastic_strain;
    computeStressFinalize(inelastic_strain_increment);
    // store incremental material properties for this step
    storeIncrementalMaterialProperties();
  }
  // update stress
  stress_new = sub_stress_new;
  // update effective inelastic strain
  _effective_inelastic_strain[_qp] =
      _effective_inelastic_strain_old[_qp] + sub_scalar_effective_inelastic_strain;

  // recover the original timestep
  _dt = dt_original;
}

template <bool is_ad>
Real
RadialReturnStressUpdateTempl<is_ad>::computeReferenceResidual(
    const GenericReal<is_ad> & effective_trial_stress,
    const GenericReal<is_ad> & scalar_effective_inelastic_strain)
{
  return MetaPhysicL::raw_value(effective_trial_stress / _three_shear_modulus) -
         MetaPhysicL::raw_value(scalar_effective_inelastic_strain);
}

template <bool is_ad>
GenericReal<is_ad>
RadialReturnStressUpdateTempl<is_ad>::maximumPermissibleValue(
    const GenericReal<is_ad> & effective_trial_stress) const
{
  return effective_trial_stress / _three_shear_modulus;
}

template <bool is_ad>
Real
RadialReturnStressUpdateTempl<is_ad>::computeTimeStepLimit()
{
  const Real scalar_inelastic_strain_incr =
      std::abs(MetaPhysicL::raw_value(_effective_inelastic_strain[_qp]) -
               _effective_inelastic_strain_old[_qp]);
  if (!scalar_inelastic_strain_incr)
    return std::numeric_limits<Real>::max();

  return _dt * _max_inelastic_increment / scalar_inelastic_strain_incr;
}

template <bool is_ad>
void
RadialReturnStressUpdateTempl<is_ad>::outputIterationSummary(std::stringstream * iter_output,
                                                             const unsigned int total_it)
{
  if (iter_output)
  {
    *iter_output << "At element " << _current_elem->id() << " _qp=" << _qp << " Coordinates "
                 << _q_point[_qp] << " block=" << _current_elem->subdomain_id() << '\n';
  }
  SingleVariableReturnMappingSolutionTempl<is_ad>::outputIterationSummary(iter_output, total_it);
}

template class RadialReturnStressUpdateTempl<false>;
template class RadialReturnStressUpdateTempl<true>;
