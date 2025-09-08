//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Kernel.h"

class Function;

/**
 * Bed slope source term for SWE momentum equations:
 *   Sx = -g h db/dx, Sy = -g h db/dy.
 * Apply to variable 'hu' or 'hv' by setting direction.
 */
class SWEBedSlopeSource : public Kernel
{
public:
  static InputParameters validParams();

  SWEBedSlopeSource(const InputParameters & parameters);
  virtual ~SWEBedSlopeSource();

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;
  virtual Real computeQpOffDiagJacobian(unsigned int jvar) override;

  /// gravitational acceleration (coupled variable or number)
  const VariableValue & _g;
  /// coupled depth
  const unsigned int _h_var;
  const VariableValue & _h;
  /// bed elevation function
  const Function & _bed;
  /// direction: 0 for x, 1 for y
  const unsigned int _dir;
};
