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
  params.addParam<Real>("default", 1.0, "The default eigenvalue");
  return params;
}

EigenValueReporter::EigenValueReporter(const std::string & name, InputParameters parameters) :
    GeneralPostprocessor(name, parameters),
    _default(getParam<Real>("default")),
    _eigenvalue(&_default)
{
}

void
EigenValueReporter::initialSetup()
{
  EigenExecutionerBase * exec = dynamic_cast<EigenExecutionerBase *>(_app.getExecutioner());
  if (exec)
    _eigenvalue = &exec->eigenValue();

}


PostprocessorValue
EigenValueReporter::getValue()
{
  return *_eigenvalue;
}
