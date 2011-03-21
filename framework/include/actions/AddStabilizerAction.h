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

#ifndef ADDSTABILIZERACTION_H
#define ADDSTABILIZERACTION_H

#include "MooseObjectAction.h"

class AddStabilizerAction: public MooseObjectAction
{
public:
  AddStabilizerAction(const std::string & name, InputParameters params);

  virtual void act();
};

template<>
InputParameters validParams<AddStabilizerAction>();

#endif //ADDSTABILIZERACTION_H
