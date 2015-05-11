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
#include "EigenExecutionerBase.h"
#include "MooseApp.h"

template<>
InputParameters validParams<ExecutionerAttributeReporter>()
{
  InputParameters params = validParams<GeneralPostprocessor>();

  // Parameter for passing in a pointer the attribute being reported (see Executioner::addAttributeReporter)
  params.addPrivateParam<Real *>("value", NULL);

  MooseEnum aggregation("none=0 sum=1 min=2 max=3", "none");
  params.addParam<MooseEnum>("aggregation", aggregation, "The type of parallel aggregation to perform (none | sum | min | max)");
  return params;
}

ExecutionerAttributeReporter::ExecutionerAttributeReporter(const std::string & name, InputParameters parameters) :
    GeneralPostprocessor(name, parameters),
    _aggregation(getParam<MooseEnum>("aggregation")),
    _value(parameters.getCheckedPointerParam<Real *>("value", "Invalid pointer to an attribute, this object should only be created via Executioner::addAttributeReporter")),
    _return_value(0.0)
{
}

void
ExecutionerAttributeReporter::execute()
{
  _return_value = *_value;
}

void
ExecutionerAttributeReporter::finalize()
{
  if (_aggregation == 1)
    gatherSum(_return_value);
  else if (_aggregation == 2)
    gatherMin(_return_value);
  else if (_aggregation == 3)
    gatherMax(_return_value);
}

PostprocessorValue
ExecutionerAttributeReporter::getValue()
{
  return *_value;
}
