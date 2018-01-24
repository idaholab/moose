//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef BODYFORCE_H
#define BODYFORCE_H

#include "Kernel.h"

// Forward Declarations
class BodyForce;
class Function;

template <>
InputParameters validParams<BodyForce>();

/**
 * This kernel implements a generic functional
 * body force term:
 * $ - c \cdof f \cdot \phi_i $
 *
 * The coefficient and function both have defaults
 * equal to 1.0.
 */
class BodyForce : public Kernel
{
public:
  BodyForce(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;

  /// Scale factor
  const Real & _scale;

  /// Optional function value
  Function & _function;

  /// Optional Postprocessor value
  const PostprocessorValue & _postprocessor;
};

#endif
