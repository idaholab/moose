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

// STL includes
#include <fstream>

// MOOSE includes
#include "CSVReader.h"
#include "MooseUtils.h"

template <>
InputParameters
validParams<CSVReader>()
{
  InputParameters params = validParams<GeneralVectorPostprocessor>();
  params.addClassDescription(
      "Converts columns of a CSV file into vectors of a VectorPostprocessor.");
  params.addRequiredParam<FileName>("csv_file",
                                    "The name of the CSV file to read. Currently, with "
                                    "the exception of the header row, only numeric "
                                    "values are supported.");
  params.addParam<bool>(
      "header",
      true,
      "When true it is assumed that the first row contains column headers, these "
      "headers are used as the VectorPostprocessor vector names. If false the "
      "file is assumed to contain only numbers and the vectors are named "
      "automatically based on the column number (e.g., 'column_0000', 'column_0001').");
  params.addParam<std::string>("delimiter",
                               ",",
                               "The column delimiter. Despite the name this can read files "
                               "separated by delimiter other than a comma.");
  params.addParam<bool>(
      "ignore_empty_lines", true, "When true new empty lines in the file are ignored.");
  params.set<MultiMooseEnum>("execute_on") = "initial";
  return params;
}

CSVReader::CSVReader(const InputParameters & params)
  : GeneralVectorPostprocessor(params),
    _csv_reader(getParam<FileName>("csv_file"),
                getParam<bool>("header"),
                getParam<std::string>("delimiter"),
                &_communicator)
{
  _csv_reader.setIgnoreEmptyLines(getParam<bool>("ignore_empty_lines"));
}

void
CSVReader::initialize()
{
  _csv_reader.read();
  for (auto & name : _csv_reader.getColumnNames())
    if (_column_data.find(name) == _column_data.end())
      _column_data[name] = &declareVector(name);
}

void
CSVReader::execute()
{
  const std::vector<std::string> & names = _csv_reader.getColumnNames();
  const std::vector<std::vector<double>> & data = _csv_reader.getColumnData();
  for (std::size_t i = 0; i < _column_data.size(); ++i)
    _column_data[names[i]]->assign(data[i].begin(), data[i].end());
}
