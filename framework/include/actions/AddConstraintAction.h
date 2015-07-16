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

#ifndef ADDCONSTRAINTACTION_H
#define ADDCONSTRAINTACTION_H

#include "MooseObjectAction.h"

class AddConstraintAction;

template<>
InputParameters validParams<AddConstraintAction>();


class AddConstraintAction : public MooseObjectAction
{
public:
  AddConstraintAction(InputParameters params);
  AddConstraintAction(const std::string & deprecated_name, InputParameters params); // DEPRECATED CONSTRUCTOR

  virtual void act();
};

#endif // ADDCONSTRAINTACTION_H
