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
#include "EigenValueReporter.h"
#include "EigenExecutionerBase.h"
#include "MooseApp.h"

template<>
InputParameters validParams<EigenValueReporter>()
{
  InputParameters params = validParams<GeneralPostprocessor>();
  return params;
}

EigenValueReporter::EigenValueReporter(const InputParameters & parameters) :
    GeneralPostprocessor(parameters),
    _eigen_executioner(NULL)
{
}

void
EigenValueReporter::initialSetup()
{
  _eigen_executioner = dynamic_cast<EigenExecutionerBase *>(_app.getExecutioner());
  if (_eigen_executioner == NULL)
    mooseError("EigenValueReporter requires an EigenExeuctioner.");
}


PostprocessorValue
EigenValueReporter::getValue()
{
  return _eigen_executioner->eigenValue();
}
