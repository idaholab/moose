//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "TimeSequenceStepperBase.h"
#include "DelimitedFileReader.h"

/**
 * Solves the PDEs at a sequence of time points given as a column in a
 * text table file (such as a *.csv file). This class uses a
 * DelimitedFileReader to read the input file.
 *
 * The column can either be identified by name (if the input file contains
 * an appropriate header) or by its index (1st column corresponds to index 0).
 */
class CSVTimeSequenceStepper : public TimeSequenceStepperBase
{
public:
  static InputParameters validParams();

  CSVTimeSequenceStepper(const InputParameters & parameters);

  virtual void init() override;

protected:
  /// name of the file where the data is read
  const std::string _file_name;

  /// whether the file contains a header with the column names
  const MooseUtils::DelimitedFileReader::HeaderFlag _header;

  /// string used as a delimiter
  const std::string _delimiter;

  /// name of the column containing the time data
  const std::string _column_name;

  /// indicates whether to access a column using its index or its name
  const bool _search_by_index;

  /// index of the column containing the time data
  const unsigned int _column_index;
};
