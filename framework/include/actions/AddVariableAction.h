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

#ifndef ADDVARIABLEACTION_H
#define ADDVARIABLEACTION_H

#include "Action.h"

class AddVariableAction;

template<>
InputParameters validParams<AddVariableAction>();


class AddVariableAction : public Action
{
public:
  AddVariableAction(const std::string & name, InputParameters params);

  virtual void act();

private:
  static const Real _abs_zero_tol;
  std::string _variable_to_read;
  unsigned int _timestep_to_read;
};

#endif // ADDVARIABLEACTION_H
