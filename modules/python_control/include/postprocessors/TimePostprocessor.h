#ifndef TIMEPOSTPROCESSOR_H
#define TIMEPOSTPROCESSOR_H

#include "GeneralPostprocessor.h"

class TimePostprocessor;

template <>
InputParameters validParams<TimePostprocessor>();

/**
 * This is a postprocessor that can get the time of the simulation.
 */
class TimePostprocessor : public GeneralPostprocessor
{
public:
  /**
   * Constructor
   *
   * @param parameters Just the postprocessor parameters, nothing else needed.
   */
  TimePostprocessor(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override;

  /**
   * Gets the current time _t
   *
   * @return The current time.
   */
  virtual Real getValue() override;
};

#endif
