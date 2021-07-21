//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "AuxKernel.h"

/**
 * Computes a measure of element length.  A plane is constructed through the element's centroid,
 * with normal equal to the direction given.  The average of the distance of the nodal positions to
 * this plane is the 'length' returned.
 */
class PorousFlowElementLength : public AuxKernel
{
public:
  static InputParameters validParams();

  PorousFlowElementLength(const InputParameters & parameters);

protected:
  virtual Real computeValue() override;

  /// number of direction components provided (needs to be 3)
  const unsigned _num_direction;

  /// x component of direction along which the element length is calculated
  const VariableValue & _direction_x;
  /// y component of direction along which the element length is calculated
  const VariableValue & _direction_y;
  /// z component of direction along which the element length is calculated
  const VariableValue & _direction_z;
};
