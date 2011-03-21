#ifndef EMPTYACTION_H
#define EMPTYACTION_H

#include "InputParameters.h"
#include "Moose.h"
#include "Action.h"

#include <string>

class EmptyAction : public Action
{
public:
  EmptyAction(const std::string & name, InputParameters params);

  virtual void act();
};

template<>
InputParameters validParams<EmptyAction>();

#endif // EMPTYACTION_H
