#ifndef CREATEEXECUTIONERACTION_H
#define CREATEEXECUTIONERACTION_H

#include "InputParameters.h"
#include "Moose.h"
#include "Action.h"

#include <string>

class CreateExecutionerAction : public Action
{
public:
  CreateExecutionerAction(const std::string & name, InputParameters params);

  virtual void act();
};

template<>
InputParameters validParams<CreateExecutionerAction>();

#endif // CREATEEXECUTIONERACTION_H
