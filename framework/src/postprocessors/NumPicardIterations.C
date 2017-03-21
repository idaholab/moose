/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

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
