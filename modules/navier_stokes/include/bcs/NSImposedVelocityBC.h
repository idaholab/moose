//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef NSIMPOSEDVELOCITYBC_H
#define NSIMPOSEDVELOCITYBC_H

// The base class definition (part of MOOSE)
#include "NodalBC.h"

// Forward Declarations
class NSImposedVelocityBC;

// Specialization required of all user-level Moose objects
template <>
InputParameters validParams<NSImposedVelocityBC>();

class NSImposedVelocityBC : public NodalBC
{
public:
  NSImposedVelocityBC(const InputParameters & parameters);

protected:
  // NodalBC's can (currently) only override the computeQpResidual function,
  // the computeQpJacobian() function automatically assembles a "1" onto the main
  // diagonal for this DoF.
  virtual Real computeQpResidual();

  // We need the density, since we are actually setting essential values of
  // *momentum* not essential values of velocity.
  const VariableValue & _rho;

  // The desired value for the velocity component
  Real _desired_velocity;
};

#endif // NSIMPOSEDVELOCITYBC_H
