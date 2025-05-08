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

template<bool is_ad>
Real 
RadialReturnCreepStressUpdateBaseTempl<is_ad>::computeCreepStrainRate(const Real& stress_eq)
{
  mooseError("This is a base class. Developers need to write their own creep law");
}

template<bool is_ad>
Real 
RadialReturnCreepStressUpdateBaseTempl<is_ad>::computeStrainEnergyRateDensity(
      const GenericMaterialProperty<RankTwoTensor, is_ad> & stress,
      const GenericMaterialProperty<RankTwoTensor, is_ad> & strain_rate)
{
    // Gaussian quadrature weights and points for 5-point rule
    const Real weights[5] = {0.2369268851, 0.4786286705, 0.5688888889, 0.4786286705, 0.2369268851};
    const Real points[5] = {-0.9061798459, -0.5384693101, 0.0, 0.5384693101, 0.9061798459};
    const Real sigma_eq = std::sqrt(3.0 * MetaPhysicL::raw_value(stress[_qp].secondInvariant()));
    const Real eps_eq = std::sqrt( 2.0/3.0*MetaPhysicL::raw_value(strain_rate[_qp].doubleContraction(strain_rate[_qp])) );

    Real integral = sigma_eq*eps_eq;
    // Perform the integral using Gaussian quadrature
    for (unsigned int k = 0; k < 5; ++k)
    {
      //Transform Gaussian points to the interval [0, sigma]
      Real sigma_eq_tmp = 0.5 * (points[k] + 1) * sigma_eq; // Map to [0, sigma_eq]
      Real strain_rate_tmp = computeCreepStrainRate(sigma_eq_tmp); 
      integral -= 0.5*sigma_eq*weights[k]*strain_rate_tmp;
    }
    return integral;
}      

template class RadialReturnCreepStressUpdateBaseTempl<false>;
template class RadialReturnCreepStressUpdateBaseTempl<true>;
