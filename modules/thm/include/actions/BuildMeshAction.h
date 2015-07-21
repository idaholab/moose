#ifndef BUILDMESHACTION_H
#define BUILDMESHACTION_H

#include "R7Action.h"

class BuildMeshAction;

template <>
InputParameters validParams<BuildMeshAction>();

class BuildMeshAction : public R7Action
{
public:
  BuildMeshAction(InputParameters params);
  virtual ~BuildMeshAction();

  virtual void act();
};

#endif /* BUILDMESHACTION_H */
