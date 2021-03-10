//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADAnisotropicReturnCreepStressUpdateBase.h"

InputParameters
ADAnisotropicReturnCreepStressUpdateBase::validParams()
{
  InputParameters params = ADGeneralizedRadialReturnStressUpdate::validParams();

  params.set<std::string>("effective_inelastic_strain_name") = "effective_creep_strain";
  params.set<std::string>("inelastic_strain_rate_name") = "creep_strain_rate";

  return params;
}

ADAnisotropicReturnCreepStressUpdateBase::ADAnisotropicReturnCreepStressUpdateBase(
    const InputParameters & parameters)
  : ADGeneralizedRadialReturnStressUpdate(parameters),
    _creep_strain(declareADProperty<RankTwoTensor>(_base_name + "creep_strain")),
    _creep_strain_old(getMaterialPropertyOld<RankTwoTensor>(_base_name + "creep_strain"))
{
}

void
ADAnisotropicReturnCreepStressUpdateBase::initQpStatefulProperties()
{
  _creep_strain[_qp].zero();

  ADGeneralizedRadialReturnStressUpdate::initQpStatefulProperties();
}

void
ADAnisotropicReturnCreepStressUpdateBase::propagateQpStatefulProperties()
{
  _creep_strain[_qp] = _creep_strain_old[_qp];

  propagateQpStatefulPropertiesRadialReturn();
}

void
ADAnisotropicReturnCreepStressUpdateBase::computeStrainFinalize(
    ADRankTwoTensor & inelasticStrainIncrement,
    const ADRankTwoTensor & /*stress*/,
    const ADDenseVector & /*stress_dev*/,
    const ADReal & /*delta_gamma*/)
{
  _creep_strain[_qp] = _creep_strain_old[_qp] + inelasticStrainIncrement;
}
