#ifndef RELAP7SETUPMESHACTION_H
#define RELAP7SETUPMESHACTION_H

#include "RELAP7Action.h"

class RELAP7SetupMeshAction;

template <>
InputParameters validParams<RELAP7SetupMeshAction>();

class RELAP7SetupMeshAction : public RELAP7Action
{
public:
  RELAP7SetupMeshAction(InputParameters params);

  virtual void act();
};

#endif /* RELAP7SETUPMESHACTION_H */
