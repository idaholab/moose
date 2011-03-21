#ifndef MOOSEOBJECTACTION_H
#define MOOSEOBJECTACTION_H

#include "InputParameters.h"
#include "Moose.h"
#include "Action.h"

#include <string>

class MooseObjectAction : public Action
{
public:
  MooseObjectAction(const std::string & name, InputParameters params);

  virtual void act() = 0;

  inline InputParameters & getMooseObjectParams() { return _moose_object_pars; }
  
protected:
  virtual void addParamsPtrs(std::vector<InputParameters *> & param_ptrs);
  
  std::string _type;
  InputParameters _moose_object_pars;
};

template<>
InputParameters validParams<MooseObjectAction>();

#endif // MOOSEOBJECTACTION_H
