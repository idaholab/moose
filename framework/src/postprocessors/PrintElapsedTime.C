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

#include "PrintElapsedTime.h"

#include "FEProblem.h"
#include "SubProblem.h"

template<>
InputParameters validParams<PrintElapsedTime>()
{
  InputParameters params = validParams<GeneralPostprocessor>();
  return params;
}

PrintElapsedTime::PrintElapsedTime(const std::string & name, InputParameters parameters) :
  GeneralPostprocessor(name, parameters)
{}

Real
PrintElapsedTime::getValue()
{
  return Moose::perf_log.get_elapsed_time();
}
