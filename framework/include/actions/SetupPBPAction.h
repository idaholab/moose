#ifndef SETUPPBPACTION_H
#define SETUPPBPACTION_H

#include "Action.h"

class SetupPBPAction: public Action
{
public:
  SetupPBPAction(const std::string & name, InputParameters params);

  virtual void act();
};

template<>
InputParameters validParams<SetupPBPAction>();

#endif //SETUPPBPACTION_H
