/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#ifndef ELEMENTVARIABLESDIFFERENCEMAX_H
#define ELEMENTVARIABLESDIFFERENCEMAX_H

#include "ElementVectorPostprocessor.h"

// Forward Declarations
class ElementVariablesDifferenceMax;

template <>
InputParameters validParams<ElementVariablesDifferenceMax>();

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

#endif // ELEMENTVARIABLESDIFFERENCEMAX_H
