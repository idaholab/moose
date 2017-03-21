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
#include "MooseApp.h"

template <>
InputParameters
validParams<CSV>()
{
  // Get the parameters from the parent object
  InputParameters params = validParams<TableOutput>();

  // Options for aligning csv output with whitespace padding
  params.addParam<bool>(
      "align",
      false,
      "Align the outputted csv data by padding the numbers with trailing whitespace");
  params.addParam<std::string>(
      "delimiter", "Assign the delimiter (default is ','"); // default not included because peacock
                                                            // didn't parse ','
  params.addParam<unsigned int>("precision", 14, "Set the output precision");

  // Suppress unused parameters
  params.suppressParameter<unsigned int>("padding");

  // Done
  return params;
}

CSV::CSV(const InputParameters & parameters)
  : TableOutput(parameters),
    _align(getParam<bool>("align")),
    _precision(getParam<unsigned int>("precision")),
    _set_delimiter(isParamValid("delimiter")),
    _delimiter(_set_delimiter ? getParam<std::string>("delimiter") : ""),
    _write_all_table(false),
    _write_vector_table(false)
{
}

void
CSV::initialSetup()
{
  // Call the base class method
  TableOutput::initialSetup();

  // Set the delimiter
  if (_set_delimiter)
    _all_data_table.setDelimiter(_delimiter);

  // Set the precision
  _all_data_table.setPrecision(_precision);
}

std::string
CSV::filename()
{
  return _file_base + ".csv";
}

void
CSV::outputScalarVariables()
{
  TableOutput::outputScalarVariables();
  _write_all_table = true;
}

void
CSV::outputPostprocessors()
{
  TableOutput::outputPostprocessors();
  _write_all_table = true;
}

void
CSV::outputVectorPostprocessors()
{
  TableOutput::outputVectorPostprocessors();
  _write_vector_table = true;
}

void
CSV::output(const ExecFlagType & type)
{
  // Start the performance log
  Moose::perf_log.push("CSV::output()", "Output");

  // Call the base class output (populates tables)
  TableOutput::output(type);

  // Print the table containing all the data to a file
  if (_write_all_table && !_all_data_table.empty() && processor_id() == 0)
    _all_data_table.printCSV(filename(), 1, _align);

  // Output each VectorPostprocessor's data to a file
  if (_write_vector_table && processor_id() == 0)
  {
    for (auto & it : _vector_postprocessor_tables)
    {
      std::ostringstream output;
      output << _file_base << "_" << MooseUtils::shortName(it.first);
      output << "_" << std::setw(_padding) << std::setprecision(0) << std::setfill('0')
             << std::right << timeStep() << ".csv";

      if (_set_delimiter)
        it.second.setDelimiter(_delimiter);
      it.second.setPrecision(_precision);
      it.second.printCSV(output.str(), 1, _align);

      if (_time_data)
      {
        std::ostringstream filename;
        filename << _file_base << "_" << MooseUtils::shortName(it.first) << "_time.csv";
        _vector_postprocessor_time_tables[it.first].printCSV(filename.str());
      }
    }
  }

  // Re-set write flags
  _write_all_table = false;
  _write_vector_table = false;

  Moose::perf_log.pop("CSV::output()", "Output");
}
