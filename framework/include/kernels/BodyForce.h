//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GenericKernel.h"

class Function;

/**
 * This kernel implements a generic functional
 * body force term:
 * $ - c \cdof f \cdot \phi_i $
 *
 * The coefficient and function both have defaults
 * equal to 1.0.
 */
template <bool is_ad>
class BodyForceTempl : public GenericKernel<is_ad>
{
public:
  static InputParameters validParams();

  BodyForceTempl(const InputParameters & parameters);

protected:
  virtual GenericReal<is_ad> computeQpResidual() override;

  /// Scale factor
  const Real & _scale;

  /// Optional function value
  const Function & _function;

  /// Optional Postprocessor value
  const PostprocessorValue & _postprocessor;

  // AD/non-AD version of the quadrature point coordinates
  const MooseArray<MooseADWrapper<Point, is_ad>> * _generic_q_point;

  usingGenericKernelMembers;
};

typedef BodyForceTempl<false> BodyForce;
typedef BodyForceTempl<true> ADBodyForce;
