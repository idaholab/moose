//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "Indicator.h"
#include "MaterialPropertyInterface.h"

#include "libmesh/quadrature.h"

/**
 * Computes the minimum element size.
 */
class MaterialTestIndicator : public Indicator
{
public:
  static InputParameters validParams();

  MaterialTestIndicator(const InputParameters & params);

protected:
  /// Computes the minimum element size based on the shear wave speed
  virtual void computeIndicator() override;

  /// Shear wave speed
  const MaterialProperty<Real> & _property;

  /// The current quadrature rule
  const QBase * const & _qrule;

  /// The variable for storing indicator value
  MooseVariable & _indicator_var;
};
