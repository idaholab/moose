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

#ifndef ADDBOUNDSVECTORSACTION_H
#define ADDBOUNDSVECTORSACTION_H

#include "MooseObjectAction.h"

class AddBoundsVectorsAction;

template<>
InputParameters validParams<AddBoundsVectorsAction>();

class AddBoundsVectorsAction : public Action
{
public:
  AddBoundsVectorsAction(const std::string & name, InputParameters params);

  virtual void act();
};

#endif // ADDBOUNDSVECTORSACTION_H
