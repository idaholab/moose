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

#ifndef ADDINDICATORACTION_H
#define ADDINDICATORACTION_H

#include "MooseObjectAction.h"

class AddIndicatorAction;

template<>
InputParameters validParams<AddIndicatorAction>();


class AddIndicatorAction : public MooseObjectAction
{
public:
  AddIndicatorAction(const std::string & name, InputParameters params);

  virtual void act();

private:

};

#endif // ADDINDICATORACTION_H
