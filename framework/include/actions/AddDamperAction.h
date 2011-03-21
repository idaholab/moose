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

#ifndef ADDDAMPERACTION_H
#define ADDDAMPERACTION_H

#include "MooseObjectAction.h"

class AddDamperAction: public MooseObjectAction
{
public:
  AddDamperAction(const std::string & name, InputParameters params);

  virtual void act();
};

template<>
InputParameters validParams<AddDamperAction>();

#endif //ADDDAMPERACTION_H
