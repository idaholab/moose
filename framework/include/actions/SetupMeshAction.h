#ifndef SETUPMESHACTION_H
#define SETUPMESHACTION_H

#include "InputParameters.h"
#include "Moose.h"
#include "Action.h"

#include <string>

class SetupMeshAction : public Action
{
public:
  SetupMeshAction(const std::string & name, InputParameters params);

  virtual void act();

  static const std::string no_file_supplied;
};

template<>
InputParameters validParams<SetupMeshAction>();

#endif // SETUPMESHACTION_H
