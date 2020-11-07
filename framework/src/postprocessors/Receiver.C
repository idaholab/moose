//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "Receiver.h"

registerMooseObject("MooseApp", Receiver);

defineLegacyParams(Receiver);

InputParameters
Receiver::validParams()
{
  InputParameters params = GeneralPostprocessor::validParams();
  params.addParam<Real>("default", 0, "The default value");
  params.addParam<bool>(
      "initialize_old", true, "Initialize the old postprocessor value with the default value");

  params.addClassDescription("Reports the value stored in this processor, which is usually filled "
                             "in by another object. The Receiver does not compute its own value.");
  return params;
}

Receiver::Receiver(const InputParameters & params)
  : GeneralPostprocessor(params),
    _initialize_old(getParam<bool>("initialize_old")),
    _my_value(getPostprocessorValueByName(name())) // use FEProblem to avoid cyclic dependency
{
  // Initialize old/older data
  getPostprocessorValueOldByName(name());
  getPostprocessorValueOlderByName(name());

  const Real & value = getParam<Real>("default");
  _fe_problem.setPostprocessorValueByName(_pp_name, value, 0);
  if (_initialize_old)
  {
    _fe_problem.setPostprocessorValueByName(_pp_name, value, 1);
    _fe_problem.setPostprocessorValueByName(_pp_name, value, 2);
  }
}

Real
Receiver::getValue()
{
  // Return the stored value (references stored value in getPostprocessorData)
  return _my_value;
}
