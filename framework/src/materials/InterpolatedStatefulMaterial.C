//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "InterpolatedStatefulMaterial.h"
#include "SerialAccess.h"

registerMooseObject("MooseApp", InterpolatedStatefulMaterialReal);
registerMooseObject("MooseApp", InterpolatedStatefulMaterialRealVectorValue);
registerMooseObject("MooseApp", InterpolatedStatefulMaterialRankTwoTensor);
registerMooseObject("MooseApp", InterpolatedStatefulMaterialRankFourTensor);

template <typename T>
InputParameters
InterpolatedStatefulMaterialTempl<T>::validParams()
{
  InputParameters params = Material::validParams();
  params.addClassDescription("Access old state from projected data.");
  params.addRequiredCoupledVar("old_state", "The AuxVars for the coupled components");
  params.addRequiredParam<MaterialPropertyName>("prop_name", "Name to emit");
  return params;
}

template <typename T>
InterpolatedStatefulMaterialTempl<T>::InterpolatedStatefulMaterialTempl(
    const InputParameters & parameters)
  : Material(parameters),
    _old_state(coupledValuesOld("old_state")),
    _older_state(coupledValuesOlder("old_state")),
    _size(Moose::SerialAccess<T>::size()),
    _prop_name(getParam<MaterialPropertyName>("prop_name")),
    _prop_old(declareProperty<T>(_prop_name + _interpolated_old)),
    _prop_older(declareProperty<T>(_prop_name + _interpolated_older))
{
  if (_old_state.size() != _size)
    paramError("old_state", "Wrong number of component AuxVariables passed in.");
  mooseAssert(_old_state.size() == _older_state.size(),
              "Internal error. Old and older coupled variable vectors should have the same size.");
}

template <typename T>
void
InterpolatedStatefulMaterialTempl<T>::computeQpProperties()
{
  std::size_t index = 0;
  for (auto & v : Moose::serialAccess(_prop_old[_qp]))
    v = (*_old_state[index++])[_qp];

  index = 0;
  for (auto & v : Moose::serialAccess(_prop_older[_qp]))
    v = (*_older_state[index++])[_qp];
}

template class InterpolatedStatefulMaterialTempl<Real>;
template class InterpolatedStatefulMaterialTempl<RealVectorValue>;
template class InterpolatedStatefulMaterialTempl<RankTwoTensor>;
template class InterpolatedStatefulMaterialTempl<RankFourTensor>;
