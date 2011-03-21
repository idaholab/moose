#ifndef ADDICACTION_H
#define ADDICACTION_H

#include "InputParameters.h"
#include "Moose.h"
#include "Action.h"

#include <string>

class AddICAction : public Action
{
public:
  AddICAction(const std::string & name, InputParameters params);

  virtual void act();
};

template<>
InputParameters validParams<AddICAction>();

#endif // ADDICACTION_H
