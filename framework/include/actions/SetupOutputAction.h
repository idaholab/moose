#ifndef SETUPOUTPUTACTION_H
#define SETUPOUTPUTACTION_H

#include "InputParameters.h"
#include "Moose.h"
#include "Action.h"

#include <string>

class SetupOutputAction : public Action
{
public:
  SetupOutputAction(const std::string & name, InputParameters params);

  virtual void act();
};

template<>
InputParameters validParams<SetupOutputAction>();

#endif // SETUPOUTPUTACTION_H
