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

#ifndef TIMEEXTREMEVALUE_H
#define TIMEEXTREMEVALUE_H

#include "GeneralPostprocessor.h"

// Forward Declarations
class TimeExtremeValue;

// Input parameters
template <>
InputParameters validParams<TimeExtremeValue>();

/// A postprocessor for reporting the max/min value of another postprocessor over time
class TimeExtremeValue : public GeneralPostprocessor
{
public:
  /// What type of extreme value we are going to compute
  enum ExtremeType
  {
    MAX,
    MIN,
    ABS_MAX,
    ABS_MIN
  };

  /**
   * Class constructor
   * @param parameters The input parameters
   */
  TimeExtremeValue(const InputParameters & parameters);
  virtual void initialize() override {}
  virtual void execute() override;
  virtual Real getValue() override;

protected:
  const PostprocessorValue & _postprocessor;

  /// The extreme value type ("max", "min", etc.)
  ExtremeType _type;

  /// The extreme value
  Real & _value;
};

#endif
