//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADNodalBC.h"

/**
 * Base class for automatic differentiation Dirichlet BCs
 */
class ADDirichletBCBase : public ADNodalBC
{
public:
  ADDirichletBCBase(const InputParameters & parameters);

  /**
   * Method to preset the nodal value if applicable
   */
  void computeValue(NumericVector<Number> & current_solution);

  static InputParameters validParams();

  bool preset() const { return _preset; }

protected:
  virtual ADReal computeQpResidual() override;

  /**
   * Compute the value of the Dirichlet BC at the current quadrature point
   */
  virtual ADReal computeQpValue() = 0;

  /// Whether or not the value is to be preset
  const bool _preset;
};
