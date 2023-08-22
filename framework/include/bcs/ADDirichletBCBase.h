//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "NodalBCBase.h"

/**
 * Base class for automatic differentiation Dirichlet BCs
 */
class ADDirichletBCBase : public NodalBCBase
{
public:
  ADDirichletBCBase(const InputParameters & parameters);

  /**
   * Method to preset the nodal value if applicable
   */
  virtual void computeValue(NumericVector<Number> & current_solution) = 0;

  static InputParameters validParams();

  bool preset() const { return _preset; }

protected:
  /// Whether or not the value is to be preset
  const bool _preset;
};
