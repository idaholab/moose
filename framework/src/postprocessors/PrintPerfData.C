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

#include "PrintPerfData.h"

#include "FEProblem.h"
#include "SubProblem.h"

template<>
InputParameters validParams<PrintPerfData>()
{
  InputParameters params = validParams<GeneralPostprocessor>();

  MooseEnum column_options("n_calls, total_time, average_time, total_time_with_sub, average_time_with_sub, percent_of_active_time, percent_of_active_time_with_sub");

  params.addRequiredParam<MooseEnum>("column", column_options, "The column you want the value of.");
  params.addRequiredParam<std::string>("event", "The name of the event.");

  return params;
}

PrintPerfData::PrintPerfData(const std::string & name, InputParameters parameters) :
    GeneralPostprocessor(name, parameters),
    _column(getParam<MooseEnum>("column")),
    _event(getParam<std::string>("event"))
{}

Real
PrintPerfData::getValue()
{
  PerfData perf_data = Moose::perf_log.get_perf_data(_event, "Solve");
  double total_time = Moose::perf_log.get_active_time();

  if(perf_data.count == 0)
    return 0.0;

  if(_column == "n_calls")
    return perf_data.count;
  else if(_column == "total_time")
    return perf_data.tot_time;
  else if(_column == "average_time")
    return perf_data.tot_time / static_cast<double>(perf_data.count);
  else if(_column == "total_time_with_sub")
    return perf_data.tot_time_incl_sub;
  else if(_column == "average_time_with_sub")
    return perf_data.tot_time_incl_sub / static_cast<double>(perf_data.count);
  else if(_column == "percent_of_active_time")
    return (total_time != 0.) ? perf_data.tot_time / total_time * 100. : 0.;
  else if(_column == "percent_of_active_time_with_sub")
    return (total_time != 0.) ? perf_data.tot_time_incl_sub / total_time * 100. : 0.;

  mooseError("Invalid column!");
}
