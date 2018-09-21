//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DiscreteNucleationData.h"

registerMooseObject("PhaseFieldApp", DiscreteNucleationData);

template <>
InputParameters
validParams<DiscreteNucleationData>()
{
  InputParameters params = validParams<GeneralPostprocessor>();
  params.addClassDescription("Output diagnostic data on a DiscreteNucleationInserter");
  params.addRequiredParam<UserObjectName>("inserter", "DiscreteNucleationInserter user object");
  MooseEnum valueType("COUNT UPDATE RATE", "COUNT");
  params.addRequiredParam<MooseEnum>(
      "value",
      valueType,
      "Select to output number of active nuclei, wether a change to "
      "the nucleus list occurred, or the total rate over the entire domain");
  return params;
}

DiscreteNucleationData::DiscreteNucleationData(const InputParameters & parameters)
  : GeneralPostprocessor(parameters),
    _inserter(getUserObject<DiscreteNucleationInserter>("inserter")),
    _nucleus_list(_inserter.getNucleusList()),
    _value_type(getParam<MooseEnum>("value").getEnum<ValueType>())
{
}

Real
DiscreteNucleationData::getValue()
{
  switch (_value_type)
  {
    case ValueType::COUNT:
      return _nucleus_list.size();

    case ValueType::UPDATE:
      return _inserter.isMapUpdateRequired();

    case ValueType::RATE:
      return _inserter.getRate();

    default:
      mooseError("Invalid value type");
  }
}
