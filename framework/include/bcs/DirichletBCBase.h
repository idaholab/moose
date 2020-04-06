//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "NodalBC.h"

/**
 * Base boundary condition of a Dirichlet type
 */
class DirichletBCBase : public NodalBC
{
public:
  static InputParameters validParams();

  DirichletBCBase(const InputParameters & parameters);

  /**
   * Method to preset the nodal value if applicable
   */
  void computeValue(NumericVector<Number> & current_solution);

  bool preset() const { return _preset; }

protected:
  virtual Real computeQpResidual() override;

  /**
   * Compute the value of the DirichletBC at the current quadrature point
   */
  virtual Real computeQpValue() = 0;

  /// Whether or not the value is to be preset
  const bool _preset;
};
