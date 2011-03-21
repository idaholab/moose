#ifndef ACTION_H
#define ACTION_H

#include "InputParameters.h"
#include "Moose.h"
#include "MooseObject.h"

#include <string>

class Action : public MooseObject
{
public:
  Action(const std::string & name, InputParameters params);

  virtual void act() = 0;

  const std::string & getAction() { return _action; }

  inline bool isParamValid(const std::string &name) const { return _pars.isParamValid(name); }

  inline InputParameters & getParams() { return _pars; }
  
protected:
  std::string _action;
  Parser & _parser_handle;
//  Action & _parent;
};

template<>
InputParameters validParams<Action>();

#endif // ACTION_H
