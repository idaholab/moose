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

#ifndef ADDMORTARINTERFACEACTION_H
#define ADDMORTARINTERFACEACTION_H

#include "Action.h"

class AddMortarInterfaceAction;

template<>
InputParameters validParams<AddMortarInterfaceAction>();

/**
 *
 */
class AddMortarInterfaceAction : public Action
{
public:
  AddMortarInterfaceAction(const std::string & name, InputParameters parameters);
  virtual ~AddMortarInterfaceAction();

  virtual void act();

protected:

};

#endif /* ADDMORTARINTERFACEACTION_H */
