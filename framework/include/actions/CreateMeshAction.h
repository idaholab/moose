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
