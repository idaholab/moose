#ifndef RELAP7INITCOMPONENTSACTION_H
#define RELAP7INITCOMPONENTSACTION_H

#include "RELAP7Action.h"

class RELAP7InitComponentsAction;

template <>
InputParameters validParams<RELAP7InitComponentsAction>();

/**
 * Initialize components
 */
class RELAP7InitComponentsAction : public RELAP7Action
{
public:
  RELAP7InitComponentsAction(InputParameters parameters);

  virtual void act();

protected:
};

#endif /* RELAP7INITCOMPONENTSACTION_H */
