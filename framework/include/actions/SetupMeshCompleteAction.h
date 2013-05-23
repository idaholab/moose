/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#ifndef SETUPMESHCOMPLETEACTION_H
#define SETUPMESHCOMPLETEACTION_H

#include "Action.h"

class SetupMeshCompleteAction;

template<>
InputParameters validParams<SetupMeshCompleteAction>();


class SetupMeshCompleteAction : public Action
{
public:
  SetupMeshCompleteAction(const std::string & name, InputParameters params);

  bool completeSetup(MooseMesh *mesh);

  virtual void act();
};

#endif // SETUPMESHCOMPLETEACTION_H
