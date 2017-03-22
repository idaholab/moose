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

#include "PerformanceData.h"

#include "FEProblem.h"
#include "SubProblem.h"

template <>
InputParameters
validParams<PerformanceData>()
{
  InputParameters params = validParams<GeneralPostprocessor>();

  MooseEnum column_options("n_calls total_time average_time total_time_with_sub "
                           "average_time_with_sub percent_of_active_time "
                           "percent_of_active_time_with_sub",
                           "total_time_with_sub");

  params.addParam<MooseEnum>(
      "column", column_options, "The column you want the value of (Default: total_time_with_sub).");
  params.addParam<std::string>("category", "Execution", "The category or \"Header\" for the event");
  params.addRequiredParam<std::string>("event",
                                       "The name or \"label\" of the event (\"ALIVE\" and "
                                       "\"ACTIVE\" are also valid events, category and "
                                       "column are ignored for these cases).");

  return params;
}

PerformanceData::PerformanceData(const InputParameters & parameters)
  : GeneralPostprocessor(parameters),
    _column(getParam<MooseEnum>("column").getEnum<PerfLogCols>()),
    _category(getParam<std::string>("category")),
    _event(getParam<std::string>("event"))
{
  // Notify the OutputWarehouse that logging has been requested
  _app.getOutputWarehouse().setLoggingRequested();
}

Real
PerformanceData::getValue()
{
  if (_event == "ALIVE")
    return Moose::perf_log.get_elapsed_time();

  Real total_time = Moose::perf_log.get_active_time();
  if (_event == "ACTIVE")
    return total_time;

  PerfData perf_data = Moose::perf_log.get_perf_data(_event, _category);
  if (perf_data.count == 0)
    return 0.0;

  switch (_column)
  {
    case N_CALLS:
      return perf_data.count;
    case TOTAL_TIME:
      return perf_data.tot_time;
    case AVERAGE_TIME:
      return perf_data.tot_time / static_cast<double>(perf_data.count);
    case TOTAL_TIME_WITH_SUB:
      return perf_data.tot_time_incl_sub;
    case AVERAGE_TIME_WITH_SUB:
      return perf_data.tot_time_incl_sub / static_cast<double>(perf_data.count);
    case PERCENT_OF_ACTIVE_TIME:
      return (total_time != 0.) ? perf_data.tot_time / total_time * 100. : 0.;
    case PERCENT_OF_ACTIVE_TIME_WITH_SUB:
      return (total_time != 0.) ? perf_data.tot_time_incl_sub / total_time * 100. : 0.;
    default:
      mooseError("Invalid column!");
  }

  return 0;
}
