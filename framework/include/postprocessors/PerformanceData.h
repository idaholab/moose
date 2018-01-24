//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef PERFORMANCEDATA_H
#define PERFORMANCEDATA_H

#include "GeneralPostprocessor.h"

// Forward Declarations
class PerformanceData;

template <>
InputParameters validParams<PerformanceData>();

class PerformanceData : public GeneralPostprocessor
{
public:
  PerformanceData(const InputParameters & parameters);

  virtual void initialize() override {}
  virtual void execute() override {}

  /**
   * This will return the elapsed wall time.
   */
  virtual Real getValue() override;

  enum PerfLogCols
  {
    N_CALLS,
    TOTAL_TIME,
    AVERAGE_TIME,
    TOTAL_TIME_WITH_SUB,
    AVERAGE_TIME_WITH_SUB,
    PERCENT_OF_ACTIVE_TIME,
    PERCENT_OF_ACTIVE_TIME_WITH_SUB
  };

protected:
  PerfLogCols _column;

  MooseEnum _category;
  MooseEnum _event;
};

#endif // PERFORMANCEDATA_H
