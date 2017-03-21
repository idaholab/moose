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

#ifndef ADDLOTSOFDIFFUSION_H
#define ADDLOTSOFDIFFUSION_H

#include "AddVariableAction.h"

class AddLotsOfDiffusion;

template <>
InputParameters validParams<AddLotsOfDiffusion>();

class AddLotsOfDiffusion : public AddVariableAction
{
public:
  AddLotsOfDiffusion(const InputParameters & params);

  virtual void act();
};

#endif // ADDLOTSOFDIFFUSION_H
