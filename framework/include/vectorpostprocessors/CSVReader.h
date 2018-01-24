//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef CSVREADER_H
#define CSVREADER_H

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
  CSVReader(const InputParameters & parameters);
  void virtual initialize() override;
  void virtual execute() override;

protected:
  /// The MOOSE delimited file reader.
  MooseUtils::DelimitedFileReader _csv_reader;

  /// Data vectors, which are stored in a map to allow for late declarations to occur, i.e., it
  /// is possible for the file to change and add new vectors during the simulation.
  std::map<std::string, VectorPostprocessorValue *> _column_data;
};

#endif // CSVREADER_H
