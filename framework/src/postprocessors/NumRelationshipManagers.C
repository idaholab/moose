//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "NumRelationshipManagers.h"
#include "RelationshipManager.h"
#include "MooseApp.h"

registerMooseObject("MooseApp", NumRelationshipManagers);

InputParameters
NumRelationshipManagers::validParams()
{
  InputParameters params = GeneralPostprocessor::validParams();
  MooseEnum rm_type("GEOMETRIC ALGEBRAIC COUPLING ALL", "ALL");
  params.addParam<MooseEnum>(
      "rm_type",
      rm_type,
      "The type of relationship managers to include in the relationship manager count");

  params.addClassDescription("Return the number of relationship managers active.");
  return params;
}

NumRelationshipManagers::NumRelationshipManagers(const InputParameters & parameters)
  : GeneralPostprocessor(parameters), _rm_type(getParam<MooseEnum>("rm_type"))
{
}

Real
NumRelationshipManagers::getValue()
{
  const auto & rms = _app.relationshipManagers();

  if (_rm_type == "ALL")
    return rms.size();

  unsigned int count = 0;
  if (_rm_type == "GEOMETRIC")
  {
    for (const auto & rm : rms)
      if (rm->isType(Moose::RelationshipManagerType::GEOMETRIC))
        ++count;
  }
  else if (_rm_type == "ALGEBRAIC")
  {
    for (const auto & rm : rms)
      if (rm->isType(Moose::RelationshipManagerType::ALGEBRAIC))
        ++count;
  }
  else if (_rm_type == "COUPLING")
  {
    for (const auto & rm : rms)
      if (rm->isType(Moose::RelationshipManagerType::COUPLING))
        ++count;
  }
  else
    mooseError("Invalid relationship manager type ", _rm_type);

  return count;
}
