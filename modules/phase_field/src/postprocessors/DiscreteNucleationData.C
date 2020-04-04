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

InputParameters
DiscreteNucleationData::validParams()
{
  InputParameters params = GeneralPostprocessor::validParams();
  params.addClassDescription("Output diagnostic data on a DiscreteNucleationInserter");
  params.addRequiredParam<UserObjectName>("inserter", "DiscreteNucleationInserter user object");
  MooseEnum valueType("COUNT UPDATE RATE INSERTIONS DELETIONS", "COUNT");
  params.addRequiredParam<MooseEnum>("value",
                                     valueType,
                                     "Select to output number of active nuclei, wether a change to "
                                     "the nucleus list occurred, the total rate over the entire "
                                     "domain, and numbers of insertions and deletions applied to "
                                     "the nucleus list.");
  return params;
}

DiscreteNucleationData::DiscreteNucleationData(const InputParameters & parameters)
  : GeneralPostprocessor(parameters),
    _inserter(getUserObject<DiscreteNucleationInserterBase>("inserter")),
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

    case ValueType::INSERTIONS:
      return _inserter.getInsertionsAndDeletions().first;

    case ValueType::DELETIONS:
      return _inserter.getInsertionsAndDeletions().second;

    default:
      mooseError("Invalid value type");
  }
}
