//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AnisotropicReturnCreepStressUpdateBase.h"

template <bool is_ad>
InputParameters
AnisotropicReturnCreepStressUpdateBaseTempl<is_ad>::validParams()
{
  InputParameters params = GeneralizedRadialReturnStressUpdateTempl<is_ad>::validParams();

  params.set<std::string>("effective_inelastic_strain_name") = "effective_creep_strain";
  params.set<std::string>("inelastic_strain_rate_name") = "creep_strain_rate";

  return params;
}

template <bool is_ad>
AnisotropicReturnCreepStressUpdateBaseTempl<is_ad>::AnisotropicReturnCreepStressUpdateBaseTempl(
    const InputParameters & parameters)
  : GeneralizedRadialReturnStressUpdateTempl<is_ad>(parameters),
    _creep_strain(this->template declareGenericProperty<RankTwoTensor, is_ad>(this->_base_name +
                                                                              "creep_strain")),
    _creep_strain_old(
        this->template getMaterialPropertyOld<RankTwoTensor>(this->_base_name + "creep_strain"))
{
}

template <bool is_ad>
void
AnisotropicReturnCreepStressUpdateBaseTempl<is_ad>::initQpStatefulProperties()
{
  _creep_strain[this->_qp].zero();

  GeneralizedRadialReturnStressUpdateTempl<is_ad>::initQpStatefulProperties();
}

template <bool is_ad>
void
AnisotropicReturnCreepStressUpdateBaseTempl<is_ad>::propagateQpStatefulProperties()
{
  _creep_strain[this->_qp] = _creep_strain_old[this->_qp];

  this->propagateQpStatefulPropertiesRadialReturn();
}

template <bool is_ad>
void
AnisotropicReturnCreepStressUpdateBaseTempl<is_ad>::computeStrainFinalize(
    GenericRankTwoTensor<is_ad> & inelasticStrainIncrement,
    const GenericRankTwoTensor<is_ad> & /*stress*/,
    const GenericDenseVector<is_ad> & /*stress_dev*/,
    const GenericReal<is_ad> & /*delta_gamma*/)
{
  _creep_strain[this->_qp] = _creep_strain_old[this->_qp] + inelasticStrainIncrement;
}

template class AnisotropicReturnCreepStressUpdateBaseTempl<false>;
template class AnisotropicReturnCreepStressUpdateBaseTempl<true>;
