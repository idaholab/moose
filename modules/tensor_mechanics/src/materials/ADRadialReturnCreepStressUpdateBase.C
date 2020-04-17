//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADRadialReturnCreepStressUpdateBase.h"
#include "RankTwoTensor.h"

InputParameters
ADRadialReturnCreepStressUpdateBase::validParams()
{
  InputParameters params = ADRadialReturnStressUpdate::validParams();
  params.set<std::string>("effective_inelastic_strain_name") = "effective_creep_strain";
  return params;
}

ADRadialReturnCreepStressUpdateBase::ADRadialReturnCreepStressUpdateBase(
    const InputParameters & parameters)
  : ADRadialReturnStressUpdate(parameters),
    _creep_strain(declareADProperty<RankTwoTensor>(_base_name + "creep_strain")),
    _creep_strain_old(getMaterialPropertyOld<RankTwoTensor>(_base_name + "creep_strain"))
{
}

void
ADRadialReturnCreepStressUpdateBase::initQpStatefulProperties()
{
  _creep_strain[_qp].zero();

  ADRadialReturnStressUpdate::initQpStatefulProperties();
}

void
ADRadialReturnCreepStressUpdateBase::propagateQpStatefulProperties()
{
  _creep_strain[_qp] = _creep_strain_old[_qp];

  propagateQpStatefulPropertiesRadialReturn();
}

void
ADRadialReturnCreepStressUpdateBase::computeStressFinalize(
    const ADRankTwoTensor & plastic_strain_increment)
{
  _creep_strain[_qp] = _creep_strain_old[_qp] + plastic_strain_increment;
}
