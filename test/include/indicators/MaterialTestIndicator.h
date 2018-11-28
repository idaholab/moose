//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef MATERIALTESTINDICATOR_H
#define MATERIALTESTINDICATOR_H

// MOOSE includes
#include "Indicator.h"
#include "MaterialPropertyInterface.h"

#include "libmesh/quadrature.h"

// Forward Declarations
class MaterialTestIndicator;

template <>
InputParameters validParams<MaterialTestIndicator>();

/**
 * Computes the minimum element size.
 */
class MaterialTestIndicator : public Indicator
{
public:
  MaterialTestIndicator(const InputParameters & params);

protected:
  /// Computes the minimum element size based on the shear wave speed
  virtual void computeIndicator() override;

  /// Shear wave speed
  const MaterialProperty<Real> & _property;

  /// The current quadrature rule
  QBase *& _qrule;

  /// The variable for storing indicator value
  MooseVariable & _indicator_var;
};

#endif // MATERIALTESTINDICATOR_H
