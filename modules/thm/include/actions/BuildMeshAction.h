#ifndef BUILDMESHACTION_H
#define BUILDMESHACTION_H

#include "THMAction.h"

class BuildMeshAction;

template <>
InputParameters validParams<BuildMeshAction>();

class BuildMeshAction : public THMAction
{
public:
  BuildMeshAction(InputParameters params);

  virtual void act();
};

#endif /* BUILDMESHACTION_H */
