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

  params.addDeprecatedParam<std::string>(
      "creep_prepend",
      "",
      "String that is prepended to the creep_strain Material Property",
      "This has been replaced by the 'base_name' parameter");
  params.set<std::string>("effective_inelastic_strain_name") = "effective_creep_strain";

  return params;
}

ADAnisotropicReturnCreepStressUpdateBase::ADAnisotropicReturnCreepStressUpdateBase(
    const InputParameters & parameters)
  : ADGeneralizedRadialReturnStressUpdate(parameters),
    _creep_prepend(getParam<std::string>("creep_prepend")),
    _creep_strain(declareADProperty<RankTwoTensor>(_base_name + _creep_prepend + "creep_strain")),
    _creep_strain_old(
        getMaterialPropertyOld<RankTwoTensor>(_base_name + _creep_prepend + "creep_strain"))
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
ADAnisotropicReturnCreepStressUpdateBase::computeStressFinalize(
    const ADRankTwoTensor & creep_strain_increment)
{
  _creep_strain[_qp] = _creep_strain_old[_qp] + creep_strain_increment;
}
