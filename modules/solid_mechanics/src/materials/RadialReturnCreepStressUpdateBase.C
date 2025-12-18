//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "RadialReturnCreepStressUpdateBase.h"

template <bool is_ad>
InputParameters
RadialReturnCreepStressUpdateBaseTempl<is_ad>::validParams()
{
  InputParameters params = RadialReturnStressUpdateTempl<is_ad>::validParams();
  params.set<std::string>("effective_inelastic_strain_name") = "effective_creep_strain";
  return params;
}

template <bool is_ad>
RadialReturnCreepStressUpdateBaseTempl<is_ad>::RadialReturnCreepStressUpdateBaseTempl(
    const InputParameters & parameters)
  : RadialReturnStressUpdateTempl<is_ad>(parameters),
    _creep_strain(this->template declareGenericProperty<RankTwoTensor, is_ad>(this->_base_name +
                                                                              "creep_strain")),
    _creep_strain_old(
        this->template getMaterialPropertyOld<RankTwoTensor>(this->_base_name + "creep_strain"))
{
}

template <bool is_ad>
void
RadialReturnCreepStressUpdateBaseTempl<is_ad>::initQpStatefulProperties()
{
  _creep_strain[_qp].zero();

  RadialReturnStressUpdateTempl<is_ad>::initQpStatefulProperties();
}

template <bool is_ad>
void
RadialReturnCreepStressUpdateBaseTempl<is_ad>::propagateQpStatefulProperties()
{
  _creep_strain[_qp] = _creep_strain_old[_qp];

  propagateQpStatefulPropertiesRadialReturn();
}

template <bool is_ad>
Real
RadialReturnCreepStressUpdateBaseTempl<is_ad>::computeStressDerivative(
    const Real /*effective_trial_stress*/, const Real /*scalar*/)
{
  mooseError("computeStressDerivative called: no stress derivative computation is needed for AD");
}

template <>
Real
RadialReturnCreepStressUpdateBaseTempl<false>::computeStressDerivative(
    const Real effective_trial_stress, const Real scalar)
{
  return -(computeDerivative(effective_trial_stress, scalar) + 1.0) / this->_three_shear_modulus;
}

template <bool is_ad>
void
RadialReturnCreepStressUpdateBaseTempl<is_ad>::computeStressFinalize(
    const GenericRankTwoTensor<is_ad> & plastic_strain_increment)
{
  _creep_strain[_qp] = _creep_strain_old[_qp] + plastic_strain_increment;
}

template <bool is_ad>
void
RadialReturnCreepStressUpdateBaseTempl<is_ad>::updateState(
    GenericRankTwoTensor<is_ad> & strain_increment,
    GenericRankTwoTensor<is_ad> & inelastic_strain_increment,
    const GenericRankTwoTensor<is_ad> & /*rotation_increment*/,
    GenericRankTwoTensor<is_ad> & stress_new,
    const RankTwoTensor & stress_old,
    const GenericRankFourTensor<is_ad> & elasticity_tensor,
    const RankTwoTensor & elastic_strain_old,
    bool compute_full_tangent_operator,
    RankFourTensor & tangent_operator)
{
  using std::sqrt;

  // compute the deviatoric trial stress and trial strain from the current intermediate
  // configuration
  GenericRankTwoTensor<is_ad> deviatoric_trial_stress = stress_new.deviatoric();

  // compute the effective trial stress
  GenericReal<is_ad> dev_trial_stress_squared =
      deviatoric_trial_stress.doubleContraction(deviatoric_trial_stress);
  GenericReal<is_ad> effective_trial_stress = MetaPhysicL::raw_value(dev_trial_stress_squared)
                                                  ? sqrt(3.0 / 2.0 * dev_trial_stress_squared)
                                                  : 0.0;

  this->computeStressInitialize(effective_trial_stress, elasticity_tensor);

  mooseAssert(
      _three_shear_modulus != 0.0,
      "Shear modulus is zero. Ensure that the base class computeStressInitialize() is called.");

  GenericRankTwoTensor<is_ad> deviatoric_old_stress = stress_old.deviatoric();
  GenericReal<is_ad> dev_old_stress_squared =
      deviatoric_old_stress.doubleContraction(deviatoric_old_stress);
  GenericReal<is_ad> old_effective_stress = MetaPhysicL::raw_value(dev_old_stress_squared)
                                                ? sqrt(3.0 / 2.0 * dev_old_stress_squared)
                                                : 0.0;
  GenericReal<is_ad> initial_guess =
      (effective_trial_stress - old_effective_stress) / this->_three_shear_modulus;
  if (initial_guess < GenericReal<is_ad>(0.0) || old_effective_stress < 1.0e-10)
    initial_guess = GenericReal<is_ad>(0.0);
  // std::cout << "initial_guess: " << MetaPhysicL::raw_value(initial_guess) << std::endl;
  // std::cout << "old_effective_stress: " << MetaPhysicL::raw_value(old_effective_stress)
  //           << std::endl;
  // std::cout << "effective_trial_stress: " << MetaPhysicL::raw_value(effective_trial_stress)
  //           << std::endl;
  // initial_guess = GenericReal<is_ad>(0.0);

  // Use Newton iteration to determine the scalar effective inelastic strain increment
  this->_effective_inelastic_strain_increment = 0.0;
  if (!MooseUtils::absoluteFuzzyEqual(effective_trial_stress, 0.0))
  {
    this->returnMappingSolve(effective_trial_stress,
                             this->_effective_inelastic_strain_increment,
                             this->_console,
                             initial_guess);
    if (this->_effective_inelastic_strain_increment != 0.0)
      inelastic_strain_increment =
          deviatoric_trial_stress *
          (1.5 * this->_effective_inelastic_strain_increment / effective_trial_stress);
    else
      inelastic_strain_increment.zero();
  }
  else
    inelastic_strain_increment.zero();

  if (this->_apply_strain)
  {
    strain_increment -= inelastic_strain_increment;
    this->updateEffectiveInelasticStrain(this->_effective_inelastic_strain_increment);

    // Use the old elastic strain here because we require tensors used by this class
    // to be isotropic and this method natively allows for changing in time
    // elasticity tensors
    stress_new = elasticity_tensor * (strain_increment + elastic_strain_old);
  }

  this->computeStressFinalize(inelastic_strain_increment);

  if constexpr (!is_ad)
  {
    if (compute_full_tangent_operator)
      this->computeTangentOperator(effective_trial_stress, stress_new, tangent_operator);
  }
  else
  {
    libmesh_ignore(compute_full_tangent_operator);
    libmesh_ignore(tangent_operator);
  }
}

template class RadialReturnCreepStressUpdateBaseTempl<false>;
template class RadialReturnCreepStressUpdateBaseTempl<true>;
