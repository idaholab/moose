#pragma once

#include "Action.h"

class AddIterationCountPostprocessorsAction;

template <>
InputParameters validParams<AddIterationCountPostprocessorsAction>();

/**
 * Action that adds postprocessors for linear and nonlinear iterations
 */
class AddIterationCountPostprocessorsAction : public Action
{
public:
  AddIterationCountPostprocessorsAction(InputParameters parameters);

  virtual void act();

protected:
  /// True if iteration count postprocessors should be added
  bool _add_pps;
};
