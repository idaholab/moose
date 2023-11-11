//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "InterpolatedStatefulMaterial.h"

registerMooseObject("TensorMechanicsApp", InterpolatedStatefulMaterial);

InputParameters
InterpolatedStatefulMaterial::validParams()
{
  InputParameters params = Material::validParams();
  params.addClassDescription("Access old state from projected data.");
  params.addRequiredCoupledVar("old_state", "The AuxVars for the coupled components");
  params.addParam<MooseEnum>(
      "prop_type", MooseEnum("REAL REALVECTORVALUE RANKTWOTENSOR RANKFOURTENSOR"), "Property type");
  params.addRequiredParam<MaterialPropertyName>("prop_name", "Name to emit");
  return params;
}

InterpolatedStatefulMaterial::InterpolatedStatefulMaterial(const InputParameters & parameters)
  : Material(parameters),
    _old_state(coupledValuesOld("old_state")),
    _prop_name(getParam<MaterialPropertyName>("prop_name")),
    _prop_type(getParam<MooseEnum>("prop_type").getEnum<PropType>()),
    _prop_real(_prop_type == PropType::REAL ? &declareProperty<Real>(_prop_name) : nullptr),
    _prop_realvectorvalue(_prop_type == PropType::REALVECTORVALUE
                              ? &declareProperty<RealVectorValue>(_prop_name)
                              : nullptr),
    _prop_ranktwotensor(_prop_type == PropType::RANKTWOTENSOR
                            ? &declareProperty<RankTwoTensor>(_prop_name)
                            : nullptr),
    _prop_rankfourtensor(_prop_type == PropType::RANKFOURTENSOR
                             ? &declareProperty<RankFourTensor>(_prop_name)
                             : nullptr)
{
}

void
InterpolatedStatefulMaterial::computeQpProperties()
{
  switch (_prop_type)
  {
    case PropType::REAL:
      (*_prop_real)[_qp] = (*_old_state[0])[_qp];
      return;

    case PropType::REALVECTORVALUE:
    {
      std::size_t index = 0;
      for (const auto i : make_range(Moose::dim))
        (*_prop_realvectorvalue)[_qp](i) = (*_old_state[index++])[_qp];
      return;
    }

    case PropType::RANKTWOTENSOR:
    {
      std::size_t index = 0;
      for (const auto i : make_range(Moose::dim))
        for (const auto j : make_range(Moose::dim))
          (*_prop_ranktwotensor)[_qp](i, j) = (*_old_state[index++])[_qp];
      return;
    }

    case PropType::RANKFOURTENSOR:
    {
      std::size_t index = 0;
      for (const auto i : make_range(Moose::dim))
        for (const auto j : make_range(Moose::dim))
          for (const auto k : make_range(Moose::dim))
            for (const auto l : make_range(Moose::dim))
              (*_prop_rankfourtensor)[_qp](i, j, k, l) = (*_old_state[index++])[_qp];
      return;
    }
  }
}
