#ifndef ADDBCACTION_H
#define ADDBCACTION_H

#include "InputParameters.h"
#include "Moose.h"
#include "Action.h"

#include <string>

class AddBCAction : public Action
{
public:
  AddBCAction(const std::string & name, InputParameters params);

  virtual void act();
};

template<>
InputParameters validParams<AddBCAction>();

#endif // ADDBCACTION_H
