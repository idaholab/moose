#ifndef SETUPDAMPERSACTION_H
#define SETUPDAMPERSACTION_H

#include "InputParameters.h"
#include "Moose.h"
#include "Action.h"

#include <string>

class SetupDampersAction : public Action
{
public:
  SetupDampersAction(const std::string & name, InputParameters params);

  virtual void act();
};

template<>
InputParameters validParams<SetupDampersAction>();

#endif // SETUPDAMPERSACTION_H
