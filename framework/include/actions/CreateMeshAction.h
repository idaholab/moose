#ifndef CREATEMESHACTION_H
#define CREATEMESHACTION_H

#include "InputParameters.h"
#include "Moose.h"
#include "Action.h"

#include <string>

class CreateMeshAction : public Action
{
public:
  CreateMeshAction(const std::string & name, InputParameters params);

  virtual void act();
};

template<>
InputParameters validParams<CreateMeshAction>();

#endif // CREATEMESHACTION_H
