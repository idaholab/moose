#ifndef BUILDMESHACTION_H
#define BUILDMESHACTION_H

#include "RELAP7Action.h"

class BuildMeshAction;

template <>
InputParameters validParams<BuildMeshAction>();

class BuildMeshAction : public RELAP7Action
{
public:
  BuildMeshAction(InputParameters params);

  virtual void act();
};

#endif /* BUILDMESHACTION_H */
