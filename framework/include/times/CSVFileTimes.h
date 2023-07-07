//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// Moose includes
#include "Times.h"

/**
 * Times from a file
 */
class CSVFileTimes : public Times
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
