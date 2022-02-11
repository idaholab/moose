//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FlowJunction.h"

/**
 * Base class for junctions that have losses
 */
class JunctionWithLossesBase : public FlowJunction
{
public:
  JunctionWithLossesBase(const InputParameters & parameters);

protected:
  /// A vector to store user inputed K loss (form loss, minor loss) coefficients.
  std::vector<Real> _k_coeffs;
  /// A vector to store user inputed reverse K loss (form loss, minor loss) coefficients.
  std::vector<Real> _kr_coeffs;
  /// A reference area for this junction to calculate its reference velocity (User input)
  const Real & _ref_area;

public:
  static InputParameters validParams();
};
