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

#ifndef ADDCOUPLEDVARIABLEACTION_H
#define ADDCOUPLEDVARIABLEACTION_H

#include "Action.h"


class AddCoupledVariableAction;

template<>
InputParameters validParams<AddCoupledVariableAction>();

/**
 *
 */
class AddCoupledVariableAction : public Action
{
public:
  AddCoupledVariableAction(const std::string & name, InputParameters parameters);
  virtual ~AddCoupledVariableAction();

  virtual void act();

protected:
  std::string _from;
  std::string _var_name;
};

#endif /* ADDCOUPLEDVARIABLEACTION_H */
