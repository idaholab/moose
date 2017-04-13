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

#ifndef ADDMATANDKERNEL_H
#define ADDMATANDKERNEL_H

#include "AddVariableAction.h"

class AddMatAndKernel;

template <>
InputParameters validParams<AddMatAndKernel>();

/// This class creates a material-kernel with the kernel depending on the
/// material property.  This is meant to help diagnose/check for issues
/// relating to dynamically (in-code i.e. via actions) generated object
/// dependencies are handled correctly.
class AddMatAndKernel : public AddVariableAction
{
public:
  AddMatAndKernel(const InputParameters & params);

  virtual void act();
};

#endif // ADDMATANDKERNEL_H
