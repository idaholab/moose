//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef INSCOURANT_H
#define INSCOURANT_H

#include "AuxKernel.h"

// Forward Declarations
class INSCourant;

template <>
InputParameters validParams<INSCourant>();

/**
 * Computes h_min / |u|
 */
class INSCourant : public AuxKernel
{
public:
  INSCourant(const InputParameters & parameters);

protected:
  virtual Real computeValue();

  // Velocity
  const VariableValue & _u_vel;
  const VariableValue & _v_vel;
  const VariableValue & _w_vel;
};

#endif // INSCOURANT_H
