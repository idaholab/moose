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
#include "AuxKernel.h"

/**
 * Compute an elemental field variable (single value per element)
 * equal to the Lp-norm of a coupled Variable.
 */
class ElementLpNormAux : public AuxKernel
{
public:
  static InputParameters validParams();

  /**
   * Class constructor
   * @param name Object name
   * @param parameters Object input parameters
   */
  ElementLpNormAux(const InputParameters & parameters);

  /**
   * Override the base class functionality to compute the element
   * integral withou scaling by element volume.
   */
  virtual void compute() override;

protected:
  /**
   * Called by compute() to get the value of the integrand at the
   * current qp.
   */
  virtual Real computeValue() override;

  // The exponent used in the norm
  Real _p;

  /// A reference to the variable to compute the norm of.
  const VariableValue & _coupled_var;
};
