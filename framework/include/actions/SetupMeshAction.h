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

#ifndef SETUPMESHACTION_H
#define SETUPMESHACTION_H

#include "MooseObjectAction.h"

//Forward Declaration
class SetupMeshAction;
class MooseMesh;

template<>
InputParameters validParams<SetupMeshAction>();


class SetupMeshAction : public MooseObjectAction
{
public:
  SetupMeshAction(const std::string & name, InputParameters params);

  virtual void act();

private:

  void setupMesh(MooseMesh *mesh);
};

#endif // SETUPMESHACTION_H
