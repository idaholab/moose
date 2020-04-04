//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CSVTimeSequenceStepper.h"

registerMooseObject("MooseApp", CSVTimeSequenceStepper);

InputParameters
CSVTimeSequenceStepper::validParams()
{
  InputParameters params = TimeSequenceStepperBase::validParams();
  params.addRequiredParam<FileName>("file_name",
                                    "name of the file in which the time sequence is read");
  params.addParam<bool>("header",
                        "indicates whether the file contains a header with the column names");
  params.addParam<std::string>("delimiter", ",", "delimiter used to parse the file");
  params.addParam<std::string>(
      "column_name", "time", "name of the column which contains the time sequence");
  params.addParam<unsigned int>("column_index",
                                "index of the column which contains the time sequence");
  params.addClassDescription(
      "Solves the Transient problem at a sequence of given time points read in a file.");
  return params;
}

CSVTimeSequenceStepper::CSVTimeSequenceStepper(const InputParameters & parameters)
  : TimeSequenceStepperBase(parameters),
    _file_name(getParam<FileName>("file_name")),
    _header(isParamValid("header")
                ? (getParam<bool>("header") ? MooseUtils::DelimitedFileReader::HeaderFlag::ON
                                            : MooseUtils::DelimitedFileReader::HeaderFlag::OFF)
                : MooseUtils::DelimitedFileReader::HeaderFlag::AUTO),
    _delimiter(getParam<std::string>("delimiter")),
    _column_name(getParam<std::string>("column_name")),
    _search_by_index(isParamValid("column_index")),
    _column_index(_search_by_index ? getParam<unsigned int>("column_index") : 0)
{
}

void
CSVTimeSequenceStepper::init()
{
  MooseUtils::DelimitedFileReader file(_file_name);

  file.setHeaderFlag(_header);
  file.setDelimiter(_delimiter);
  file.read();

  std::vector<Real> instants;

  if (_search_by_index)
  {
    std::vector<std::vector<double>> data = file.getData();
    if (_column_index >= data.size())
      mooseError("cannot find column ", _column_index, " in file ", _file_name);
    instants = data[_column_index];
  }
  else
    instants = file.getData(_column_name);

  if (instants.size() == 0)
    mooseError("empty sequence in file ", _file_name);

  setupSequence(instants);
}
