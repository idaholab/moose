#ifndef ADDCOMPONENTACTION_H
#define ADDCOMPONENTACTION_H

#include "R7Action.h"

class AddComponentAction;

template <>
InputParameters validParams<AddComponentAction>();

class AddComponentAction : public R7Action
{
public:
  AddComponentAction(const std::string & name, InputParameters params);

  virtual void act();
};


#endif /* ADDCOMPONENTACTION_H */
