//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "GeneralVectorPostprocessor.h"
#include "DelimitedFileReader.h"

// Forward declarations
class CSVReader;

template <>
InputParameters validParams<CSVReader>();

class CSVReader : public GeneralVectorPostprocessor
{
public:
  static InputParameters validParams();

  CSVReader(const InputParameters & parameters);
  void virtual initialize() override;
  void virtual execute() override;

protected:
  /// The MOOSE delimited file reader.
  MooseUtils::DelimitedFileReader _csv_reader;

  /// The vector variables storing the data read from the csv
  std::map<std::string, VectorPostprocessorValue *> _column_data;
};
