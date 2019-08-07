//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeMultiplePorousInelasticStress.h"

#include "RankTwoTensor.h"

registerMooseObject("TensorMechanicsApp", ComputeMultiplePorousInelasticStress);

template <>
InputParameters
validParams<ComputeMultiplePorousInelasticStress>()
{
  InputParameters params = validParams<ComputeMultipleInelasticStress>();
  params.addClassDescription("todo.");

  params.addParam<MaterialPropertyName>(
      "porosity_name", "porosity", "Name of porosity material property");
  params.addRequiredRangeCheckedParam<Real>(
      "initial_porosity", "initial_porosity>0.0 & initial_porosity<1.0", "Initial porosity");

  return params;
}

ComputeMultiplePorousInelasticStress::ComputeMultiplePorousInelasticStress(
    const InputParameters & parameters)
  : ComputeMultipleInelasticStress(parameters),
    _porosity(declareProperty<Real>(getParam<MaterialPropertyName>("porosity_name"))),
    _porosity_old(getMaterialPropertyOld<Real>(getParam<MaterialPropertyName>("porosity_name"))),
    _initial_porosity(getParam<Real>("initial_porosity"))
{
}

void
ComputeMultiplePorousInelasticStress::initQpStatefulProperties()
{
  _porosity[_qp] = _initial_porosity;

  ComputeStressBase::initQpStatefulProperties();
}

void
ComputeMultiplePorousInelasticStress::computeQpProperties()
{
  ComputeStressBase::computeQpProperties();

  _porosity[_qp] =
      (1.0 - _porosity_old[_qp]) * (_inelastic_strain[_qp] - _inelastic_strain_old[_qp]).trace() +
      _porosity_old[_qp];
}
