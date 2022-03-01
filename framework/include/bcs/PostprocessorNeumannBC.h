//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "IntegratedBC.h"

/**
 * Implements a constant Neumann BC where grad(u) is a equal to a postprocessor on the boundary.
 * Uses the term produced from integrating the diffusion operator by parts.
 */
class PostprocessorNeumannBC : public IntegratedBC
{
public:
  /**
   * Factory constructor, takes parameters so that all derived classes can be built using the same
   * constructor.
   */
  static InputParameters validParams();

  PostprocessorNeumannBC(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;

  /// Value of grad(u) on the boundary.
  const PostprocessorValue & _value;
};
