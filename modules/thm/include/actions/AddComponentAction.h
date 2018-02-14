#ifndef ADDCOMPONENTACTION_H
#define ADDCOMPONENTACTION_H

#include "RELAP7ObjectAction.h"

class AddComponentAction;

template <>
InputParameters validParams<AddComponentAction>();

class AddComponentAction : public RELAP7ObjectAction
{
public:
  AddComponentAction(InputParameters params);

  virtual void act();
};

#endif /* ADDCOMPONENTACTION_H */
