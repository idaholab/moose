//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ElementVectorPostprocessor.h"

/**
 * \brief     Finds the largest difference between two variable fields
 *
 * \details   The location of the associated quadrature point is also stored,
 *            along with the actual field values that resulted in the
 *            difference. The user may specify in the input file whether to find
 *            the largest absolute difference and, if so, whether to preserve
 *            the sign
 */
class ElementVariablesDifferenceMax : public ElementVectorPostprocessor
{
public:
  static InputParameters validParams();

  // Basic MooseObject cosntructor
  ElementVariablesDifferenceMax(const InputParameters & parameters);

  // Required overrides
  virtual void execute() override;
  virtual void finalize() override;
  virtual void initialize() override;
  virtual void threadJoin(const UserObject & s) override;

protected:
  /// The first variable, operated to produce a difference as: #_a - #_b
  const VariableValue & _a;

  /// The second variable, operated to produce a difference as: #_a - #_b
  const VariableValue & _b;

  /// The value of #_a that produced the maximum difference
  VectorPostprocessorValue & _a_value;

  /// The value of #_b that produced the maximum difference
  VectorPostprocessorValue & _b_value;

  /// The actual maximum difference
  VectorPostprocessorValue & _max_difference;

  /// The x position of the associated quadrature point
  VectorPostprocessorValue & _position_x;

  /// The y position of the associated quadrature point
  VectorPostprocessorValue & _position_y;

  /// The z position of the associated quadrature point
  VectorPostprocessorValue & _position_z;

  /// Internal flag to indicate we are seeking the largest absolute value
  const bool _furthest_from_zero;

  /**
   * Collection of all the items so only one MPI call is required, these will be
   * spread to the actual VectorPostprocessorValue containers in finalize()
   */
  std::vector<Real> _all;
};
