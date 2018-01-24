//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "NumPicardIterations.h"
#include "Transient.h"
#include "MooseApp.h"

template <>
InputParameters
validParams<NumPicardIterations>()
{
  InputParameters params = validParams<GeneralPostprocessor>();
  return params;
}

NumPicardIterations::NumPicardIterations(const InputParameters & parameters)
  : GeneralPostprocessor(parameters), _transient_executioner(NULL)
{
}

void
NumPicardIterations::initialize()
{
  _transient_executioner = dynamic_cast<Transient *>(_app.getExecutioner());
  if (!_transient_executioner)
  {
    mooseError(
        "The NumPicardIterations Postprocessor can only be used with a Transient Executioner");
  }
}

Real
NumPicardIterations::getValue()
{
  return _transient_executioner->numPicardIts();
}
