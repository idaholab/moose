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

#ifndef ADDLOTSOFVARIABLESACTION_H
#define ADDLOTSOFVARIABLESACTION_H

#include "Action.h"

class AddLotsOfVariablesAction;

template <>
InputParameters validParams<AddLotsOfVariablesAction>();

class AddLotsOfVariablesAction : public Action
{
public:
  AddLotsOfVariablesAction(const InputParameters & parameters);

  virtual void act();

private:
  static const Real _abs_zero_tol;
  std::string _variable_to_read;
};

#endif // ADDLOTSOFSCALARVARIABLESACTION_H
