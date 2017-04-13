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
#include "ExecutionerAttributeReporter.h"
#include "MooseApp.h"

template <>
InputParameters
validParams<ExecutionerAttributeReporter>()
{
  InputParameters params = validParams<GeneralPostprocessor>();

  // Parameter for passing in a pointer the attribute being reported (see
  // Executioner::addAttributeReporter)
  params.addPrivateParam<Real *>("value", NULL);
  return params;
}

ExecutionerAttributeReporter::ExecutionerAttributeReporter(const InputParameters & parameters)
  : GeneralPostprocessor(parameters),
    _value(parameters.getCheckedPointerParam<Real *>(
        "value",
        "Invalid pointer to an attribute, this object should only be created via "
        "Executioner::addAttributeReporter"))
{
}

PostprocessorValue
ExecutionerAttributeReporter::getValue()
{
  return *_value;
}
