#ifndef CONTROLDATAINTEGRITYCHECKACTION_H
#define CONTROLDATAINTEGRITYCHECKACTION_H

#include "RELAP7Action.h"

class ControlDataIntegrityCheckAction;

template <>
InputParameters validParams<ControlDataIntegrityCheckAction>();

/**
 * Action to trigger the check of control data integrity
 */
class ControlDataIntegrityCheckAction : public RELAP7Action
{
public:
  ControlDataIntegrityCheckAction(InputParameters parameters);

  virtual void act();

protected:
};

#endif /* CONTROLDATAINTEGRITYCHECKACTION_H */
