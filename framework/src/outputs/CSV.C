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

// Moose includes
#include "CSV.h"
#include "FEProblem.h"

template<>
InputParameters validParams<CSV>()
{
  // Get the parameters from the parent object
  InputParameters params = validParams<TableOutput>();

  // Add option for appending file on restart
  params.addParam<bool>("append_restart", false, "Append existing file on restart");

  // Suppress unused parameters
  params.suppressParameter<unsigned int>("padding");

  return params;
}

CSV::CSV(const std::string & name, InputParameters & parameters) :
    TableOutput(name, parameters)
{
}

CSV::~CSV()
{
}

void
CSV::initialSetup()
{
  if (_problem_ptr->isRestarting() && !getParam<bool>("append_restart"))
    _all_data_table.clear();
}

std::string
CSV::filename()
{
  return _file_base + ".csv";
}

void
CSV::output()
{
  // Call the base class output (populates tables)
  TableOutput::output();

  // Print the table containing all the data to a file
  if (!_all_data_table.empty() && processor_id() == 0)
    _all_data_table.printCSV(filename());

  // Output each VectorPostprocessor's data to a file
  for (std::map<std::string, FormattedTable>::iterator it = _vector_postprocessor_tables.begin(); it != _vector_postprocessor_tables.end(); ++it)
  {
    std::ostringstream output;
    output << _file_base << "_" << it->first;

    output << "_"
           << std::setw(_padding)
           << std::setprecision(0)
           << std::setfill('0')
           << std::right
           << _t_step;

    output << ".csv";

    it->second.printCSV(output.str());
  }
}
