//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GeneralReporter.h"

InputParameters
GeneralReporter::validParams()
{
  InputParameters params = GeneralUserObject::validParams();
  params += Reporter::validParams();

  // Whether or not to always store this object's value
  // See the override for shouldStore() for more information
  params.addPrivateParam<bool>("_always_store", true);

  return params;
}

GeneralReporter::GeneralReporter(const InputParameters & parameters)
  : GeneralUserObject(parameters), Reporter(this), _always_store(getParam<bool>("_always_store"))
{
}

bool
GeneralReporter::shouldStore() const
{
  // Either we always store, or we store if the current execution flag matches
  // a flag that is within this GeneralReporter's 'execute_on'
  return _always_store || getExecuteOnEnum().contains(_fe_problem.getCurrentExecuteOnFlag());
}
