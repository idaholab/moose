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

#ifndef ADDUSEROBJECTACTION_H
#define ADDUSEROBJECTACTION_H

#include "MooseObjectAction.h"

class AddUserObjectAction;

template <>
InputParameters validParams<AddUserObjectAction>();

class AddUserObjectAction : public MooseObjectAction
{
public:
  AddUserObjectAction(InputParameters params);

  virtual void act() override;
};

#endif // ADDUSEROBJECTACTION_H
