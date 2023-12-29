//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADComputeMultiplePorousInelasticStress.h"

#include "RankTwoTensor.h"

registerMooseObjectDeprecated("TensorMechanicsApp",
                              ADComputeMultiplePorousInelasticStress,
                              "01/30/2021 24:00");

InputParameters
ADComputeMultiplePorousInelasticStress::validParams()
{
  InputParameters params = ADComputeMultipleInelasticStress::validParams();
  params.addClassDescription(
      "Compute state (stress and internal parameters such as plastic "
      "strains and internal parameters) using an iterative process. A porosity material property "
      "is defined and is calculated from the trace of inelastic strain increment.");

  params.addParam<MaterialPropertyName>(
      "porosity_name", "porosity", "Name of porosity material property");
  params.addRequiredRangeCheckedParam<Real>(
      "initial_porosity", "initial_porosity>0.0 & initial_porosity<1.0", "Initial porosity");
  return params;
}

ADComputeMultiplePorousInelasticStress::ADComputeMultiplePorousInelasticStress(
    const InputParameters & parameters)
  : ADComputeMultipleInelasticStress(parameters),
    _porosity(declareADProperty<Real>(getParam<MaterialPropertyName>("porosity_name"))),
    _porosity_old(getMaterialPropertyOld<Real>(getParam<MaterialPropertyName>("porosity_name"))),
    _initial_porosity(getParam<Real>("initial_porosity"))
{
}

void
ADComputeMultiplePorousInelasticStress::initQpStatefulProperties()
{
  ADComputeStressBase::initQpStatefulProperties();

  _porosity[_qp] = _initial_porosity;
}

void
ADComputeMultiplePorousInelasticStress::computeQpProperties()
{
  ADComputeStressBase::computeQpProperties();

  _porosity[_qp] =
      (1.0 - _porosity_old[_qp]) * (_inelastic_strain[_qp] - _inelastic_strain_old[_qp]).trace() +
      _porosity_old[_qp];
  if (_porosity[_qp] < 0.0)
    _porosity[_qp] = 0.0;
}
