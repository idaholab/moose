#ifndef ACTION_H
#define ACTION_H

#include "InputParameters.h"
#include "Moose.h"

#include <string>

class Action 
{
public:
  Action(const std::string & name, InputParameters params);

  virtual void act() = 0;

  const std::string & getAction() { return _action; }

protected:
  std::string _name;
  std::string _action;
  InputParameters _params;
};

template<>
InputParameters validParams<Action>();

#endif // ACTION_H
