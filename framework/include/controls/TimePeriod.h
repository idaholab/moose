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

#ifndef TIMEPERIOD_H
#define TIMEPERIOD_H

// MOOSE includes
#include "Control.h"

// Forward declarations
class TimePeriod;
class Function;

template <>
InputParameters validParams<TimePeriod>();

/**
 * A basic control for disabling objects for a portion of the simulation.
 */
class TimePeriod : public Control
{
public:
  /**
   * Class constructor
   * @param parameters Input parameters for this Control object
   */
  TimePeriod(const InputParameters & parameters);

  virtual void execute() override;

protected:
  /**
   * If enabled, this injects the start/end times into the TimeStepper sync times.
   */
  void initialSetup() override;

private:
  /// List of objects to enable
  const std::vector<std::string> & _enable;

  /// List of objects to disable
  const std::vector<std::string> & _disable;

  /// The time to begin enabling the supplied object tags (defaults to the simulation start time)
  std::vector<Real> _start_time;

  /// The time to stop enabling the supplied object tags (defaults to the end of the simulation)
  std::vector<Real> _end_time;

  /// Flag for setting value outside of time range
  bool _set_outside_of_range;
};

#endif // TIMEPERIOD_H
