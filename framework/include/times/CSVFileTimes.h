//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// Moose includes
#include "TimesReporter.h"

/**
 * Times from a file
 */
class CSVFileTimes : public TimesReporter
{
public:
  static InputParameters validParams();
  CSVFileTimes(const InputParameters & parameters);
  virtual ~CSVFileTimes() = default;

protected:
  virtual void initialize() override {}

  /// Column in (all) the CSV file(s) where the time is
  const unsigned int _time_column_index;
};
