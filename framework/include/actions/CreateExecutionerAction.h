#ifndef CREATEEXECUTIONERACTION_H
#define CREATEEXECUTIONERACTION_H

#include "MooseObjectAction.h"

class CreateExecutionerAction : public MooseObjectAction
{
public:
  CreateExecutionerAction(const std::string & name, InputParameters params);

  virtual void act();
};

template<>
InputParameters validParams<CreateExecutionerAction>();

#endif // CREATEEXECUTIONERACTION_H
