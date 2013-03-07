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

#include "PrintRunTime.h"

#include "FEProblem.h"
#include "SubProblem.h"

template<>
InputParameters validParams<PrintRunTime>()
{
  InputParameters params = validParams<GeneralPostprocessor>();
  MooseEnum time_options("alive, active");
  params.addRequiredParam<MooseEnum>("time_type", time_options, "Whether to output the total elapsed or just the active time");
  return params;
}

PrintRunTime::PrintRunTime(const std::string & name, InputParameters parameters) :
    GeneralPostprocessor(name, parameters),
    _time_type(getParam<MooseEnum>("time_type"))
{}

Real
PrintRunTime::getValue()
{
  switch(_time_type)
  {
    case 0:
      return Moose::perf_log.get_elapsed_time();
    case 1:
      return Moose::perf_log.get_active_time();
  }

  mooseError("Invalid Type");
}
