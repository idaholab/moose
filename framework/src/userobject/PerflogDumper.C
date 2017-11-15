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

#include "PerflogDumper.h"
#include "libmesh/perf_log.h"

#include <ostream>

template <>
InputParameters
validParams<PerflogDumper>()
{
  InputParameters params = validParams<GeneralUserObject>();
  params.set<MultiMooseEnum>("execute_on") = "final";
  params.addParam<std::string>("outfile", "perflog.csv", "name of perf log output file");
  params.addClassDescription("Dumps perlog information to a csv file for further analysis.");
  return params;
}

PerflogDumper::PerflogDumper(const InputParameters & parameters) : GeneralUserObject(parameters) {}

void
PerflogDumper::execute()
{
  auto & log = Moose::perf_log.get_log_raw();
  std::ofstream f(_pars.get<std::string>("outfile"));
  if (!f.good())
    mooseError("PerfLogDumper: error opening file '", _pars.get<std::string>("outfile"), "'");

  f << "category,subcategory,n_calls,tot_time_self,tot_time_cum\n";
  for (auto & entry : log)
  {
    const std::string & cat = entry.first.first;
    const std::string & subcat = entry.first.second;
    const PerfData & data = entry.second;

    f << "\"" << cat << "\""
      << ",\"" << subcat << "\""
      << "," << data.count << "," << data.tot_time << "," << data.tot_time_incl_sub << "\n";
  }
  if (!f.good())
    mooseError("PerfLogDumper: error writing file '", _pars.get<std::string>("outfile"), "'");
}
