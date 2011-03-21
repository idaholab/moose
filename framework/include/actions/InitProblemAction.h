#ifndef INITPROBLEMACTION_H
#define INITPROBLEMACTION_H

#include "InputParameters.h"
#include "Moose.h"
#include "Action.h"

#include <string>

class InitProblemAction : public Action
{
public:
  InitProblemAction(const std::string & name, InputParameters params);

  virtual void act();
};

template<>
InputParameters validParams<InitProblemAction>();

#endif // INITPROBLEMACTION_H
