#ifndef IDENTIFYLOOPSACTION_H
#define IDENTIFYLOOPSACTION_H

#include "THMAction.h"

class IdentifyLoopsAction;

template <>
InputParameters validParams<IdentifyLoopsAction>();

/**
 * Identifies the component loops.
 */
class IdentifyLoopsAction : public THMAction
{
public:
  IdentifyLoopsAction(InputParameters parameters);

  virtual void act();
};

#endif /* IDENTIFYLOOPSACTION_H */
