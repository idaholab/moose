//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
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
  params.addParam<Real>("eff_creep_strain_inc_predictor_scale",
                        0.0,
                        "Scaling factor for creep strain increment from previous step.");
  params.set<std::string>("effective_inelastic_strain_name") = "effective_creep_strain";
  return params;
}

template <bool is_ad>
RadialReturnCreepStressUpdateBaseTempl<is_ad>::RadialReturnCreepStressUpdateBaseTempl(
    const InputParameters & parameters)
  : RadialReturnStressUpdateTempl<is_ad>(parameters),
    _eff_creep_strain_inc_predictor_scale(
        this->template getParam<Real>("eff_creep_strain_inc_predictor_scale")),
    _creep_strain(this->template declareGenericProperty<RankTwoTensor, is_ad>(this->_base_name +
                                                                              "creep_strain")),
    _creep_strain_old(
        this->template getMaterialPropertyOld<RankTwoTensor>(this->_base_name + "creep_strain")),
    _eff_creep_strain_inc(
        this->template declareProperty<Real>(this->_base_name + "eff_creep_strain_inc")),
    _eff_creep_strain_inc_old(
        this->template getMaterialPropertyOld<Real>(this->_base_name + "eff_creep_strain_inc"))
{
}

template <bool is_ad>
void
RadialReturnCreepStressUpdateBaseTempl<is_ad>::initQpStatefulProperties()
{
  _creep_strain[_qp].zero();
  _eff_creep_strain_inc[_qp] = 0.;
  RadialReturnStressUpdateTempl<is_ad>::initQpStatefulProperties();
}

template <bool is_ad>
void
RadialReturnCreepStressUpdateBaseTempl<is_ad>::propagateQpStatefulProperties()
{
  _creep_strain[_qp] = _creep_strain_old[_qp];
  _eff_creep_strain_inc[_qp] = _eff_creep_strain_inc_old[_qp];
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
GenericReal<is_ad>
RadialReturnCreepStressUpdateBaseTempl<is_ad>::initialGuess(
    const GenericReal<is_ad> & /*effective_trial_stress*/)
{
  return _eff_creep_strain_inc_predictor_scale * _eff_creep_strain_inc_old[_qp];
}

template <bool is_ad>
void
RadialReturnCreepStressUpdateBaseTempl<is_ad>::computeStressFinalize(
    const GenericRankTwoTensor<is_ad> & plastic_strain_increment)
{
  _creep_strain[_qp] = _creep_strain_old[_qp] + plastic_strain_increment;
  // compute effective_strain_increment from tensor
  // plastic_strain_increment is already deviatoric.
  const Real deviatoric_strain_inc_squared =
      MetaPhysicL::raw_value(plastic_strain_increment)
          .doubleContraction(MetaPhysicL::raw_value(plastic_strain_increment));
  _eff_creep_strain_inc[_qp] = std::sqrt(2.0 / 3.0 * deviatoric_strain_inc_squared);
}

template class RadialReturnCreepStressUpdateBaseTempl<false>;
template class RadialReturnCreepStressUpdateBaseTempl<true>;
