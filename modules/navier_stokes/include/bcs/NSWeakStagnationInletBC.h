//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef NSWEAKSTAGNATIONINLETBC_H
#define NSWEAKSTAGNATIONINLETBC_H

#include "MooseObject.h"

class NSWeakStagnationInletBC;

template <>
InputParameters validParams<NSWeakStagnationInletBC>();

/**
 * This class facilitates adding weak stagnation inlet BCs via an
 * Action by setting up the required parameters.  See also
 * AddNavierStokesBCsAction.
 */
class NSWeakStagnationInletBC : public MooseObject
{
public:
  NSWeakStagnationInletBC(const InputParameters & parameters);
  virtual ~NSWeakStagnationInletBC();

protected:
};

#endif
