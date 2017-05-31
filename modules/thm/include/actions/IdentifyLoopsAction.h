#ifndef IDENTIFYLOOPSACTION_H
#define IDENTIFYLOOPSACTION_H

#include "R7Action.h"

class IdentifyLoopsAction;

template <>
InputParameters validParams<IdentifyLoopsAction>();

/**
 * Identifies the component loops.
 */
class IdentifyLoopsAction : public R7Action
{
public:
  IdentifyLoopsAction(InputParameters parameters);

  virtual void act();
};

#endif /* IDENTIFYLOOPSACTION_H */
