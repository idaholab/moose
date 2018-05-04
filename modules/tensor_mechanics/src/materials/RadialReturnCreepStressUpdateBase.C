//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "RadialReturnCreepStressUpdateBase.h"

template <>
InputParameters
validParams<RadialReturnCreepStressUpdateBase>()
{
  InputParameters params = validParams<RadialReturnStressUpdate>();

  params.addDeprecatedParam<std::string>(
      "creep_prepend",
      "",
      "String that is prepended to the creep_strain Material Property",
      "This has been replaced by the 'base_name' parameter");
  params.set<std::string>("effective_inelastic_strain_name") = "effective_creep_strain";

  return params;
}

RadialReturnCreepStressUpdateBase::RadialReturnCreepStressUpdateBase(
    const InputParameters & parameters)
  : RadialReturnStressUpdate(parameters),
    _creep_prepend(getParam<std::string>("creep_prepend")),
    _creep_strain(declareProperty<RankTwoTensor>(_base_name + _creep_prepend + "creep_strain")),
    _creep_strain_old(
        getMaterialPropertyOld<RankTwoTensor>(_base_name + _creep_prepend + "creep_strain"))
{
}

void
RadialReturnCreepStressUpdateBase::initQpStatefulProperties()
{
  _creep_strain[_qp].zero();

  RadialReturnStressUpdate::initQpStatefulProperties();
}

void
RadialReturnCreepStressUpdateBase::propagateQpStatefulProperties()
{
  _creep_strain[_qp] = _creep_strain_old[_qp];

  propagateQpStatefulPropertiesRadialReturn();
}

Real
RadialReturnCreepStressUpdateBase::computeStressDerivative(const Real effective_trial_stress,
                                                           const Real scalar)
{
  return -(computeDerivative(effective_trial_stress, scalar) + 1.0) / _three_shear_modulus;
}

void
RadialReturnCreepStressUpdateBase::computeStressFinalize(
    const RankTwoTensor & plastic_strain_increment)
{
  _creep_strain[_qp] = _creep_strain_old[_qp] + plastic_strain_increment;
}
