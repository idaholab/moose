//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MandelConverter.h"

registerMooseObject("SolidMechanicsApp", RankTwoTensorToSymmetricRankTwoTensor);
registerMooseObject("SolidMechanicsApp", SymmetricRankTwoTensorToRankTwoTensor);
registerMooseObject("SolidMechanicsApp", RankFourTensorToSymmetricRankFourTensor);
registerMooseObject("SolidMechanicsApp", SymmetricRankFourTensorToRankFourTensor);

template <typename T, bool symmetrize>
InputParameters
MandelConverter<T, symmetrize>::validParams()
{
  InputParameters params = Material::validParams();
  params.addClassDescription("Converts material property of type " +
                             demangle(typeid(FromType).name()) + " to type " +
                             demangle(typeid(ToType).name()));
  params.addRequiredParam<MaterialPropertyName>("from", "Material property to convert from");
  params.addRequiredParam<MaterialPropertyName>("to", "Material property to convert to");
  return params;
}

template <typename T, bool symmetrize>
MandelConverter<T, symmetrize>::MandelConverter(const InputParameters & params)
  : Material(params),
    _from(getMaterialProperty<FromType>("from")),
    _to(declareProperty<ToType>("to"))
{
}

template <typename T, bool symmetrize>
void
MandelConverter<T, symmetrize>::initQpStatefulProperties()
{
  computeQpProperties();
}

template <typename T, bool symmetrize>
void
MandelConverter<T, symmetrize>::computeQpProperties()
{
  _to[_qp] = ToType(_from[_qp]);
}

template class MandelConverter<RankTwoTensor, true>;
template class MandelConverter<RankTwoTensor, false>;
template class MandelConverter<RankFourTensor, true>;
template class MandelConverter<RankFourTensor, false>;
