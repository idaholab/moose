#pragma once

#include "THMControl.h"

/**
 * Control the solve based on a postprocessor value
 *
 * If the postprocessor indicates a zero, no solve it performed, otherwise the solve is performed.
 */
class THMSolvePostprocessorControl : public THMControl
{
public:
  THMSolvePostprocessorControl(const InputParameters & parameters);

  virtual void execute();

protected:
  /// The postprocessor that determines if the solve should be done or not
  const PostprocessorValue & _solve_pps;

public:
  static InputParameters validParams();
};
