/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
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
