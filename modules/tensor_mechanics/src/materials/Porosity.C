//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "Porosity.h"

registerMooseObject("TensorMechanicsApp", Porosity);
registerMooseObject("TensorMechanicsApp", ADPorosity);

template <bool is_ad>
InputParameters
PorosityTempl<is_ad>::validParams()
{
  InputParameters params = Material::validParams();

  params.addClassDescription("Porosity calculation from the inelastic strain.");

  params.addParam<MaterialPropertyName>(
      "porosity_name", "porosity", "Name of porosity material property");
  params.addRequiredRangeCheckedParam<Real>(
      "initial_porosity", "initial_porosity>0.0 & initial_porosity<1.0", "Initial porosity");
  params.addRequiredParam<MaterialPropertyName>("inelastic_strain",
                                                "Name of the combined inelastic strain");

  return params;
}

template <bool is_ad>
PorosityTempl<is_ad>::PorosityTempl(const InputParameters & parameters)
  : Material(parameters),
    _porosity(declareGenericProperty<Real, is_ad>(getParam<MaterialPropertyName>("porosity_name"))),
    _porosity_old(getMaterialPropertyOld<Real>("porosity_name")),
    _initial_porosity(getParam<Real>("initial_porosity")),
    _inelastic_strain(getGenericMaterialProperty<RankTwoTensor, is_ad>("inelastic_strain")),
    _inelastic_strain_old(getMaterialPropertyOld<RankTwoTensor>("inelastic_strain"))
{
}

template <bool is_ad>
void
PorosityTempl<is_ad>::initQpStatefulProperties()
{
  _porosity[_qp] = _initial_porosity;
}

template <bool is_ad>
void
PorosityTempl<is_ad>::computeQpProperties()
{
  _porosity[_qp] =
      (1.0 - _porosity_old[_qp]) * (_inelastic_strain[_qp] - _inelastic_strain_old[_qp]).trace() +
      _porosity_old[_qp];

  if (_porosity[_qp] < 0.0)
    _porosity[_qp] = 0.0;
}
