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

#ifndef POINTVALUE_H
#define POINTVALUE_H

#include "GeneralPostprocessor.h"

// Forward Declarations
class PointValue;

template <>
InputParameters validParams<PointValue>();

/**
 * Compute the value of a variable at a specified location.
 *
 * Warning: This postprocessor may result in undefined behavior if utilized with
 * non-continuous elements and the point being located lies on an element boundary.
 */
class PointValue : public GeneralPostprocessor
{
public:
  /**
   * Constructor.
   * @param parameters The input file parameters for this object
   */
  PointValue(const InputParameters & parameters);

  /**
   * Empty method, no initialization needed
   */
  virtual void initialize() override {}

  /**
   * Determines what element contains the specified point
   */
  virtual void execute() override;

  /**
   * Returns the value of the variable at the specified location
   */
  virtual Real getValue() override;

  /**
   * Performs the necessary parallel communication as well as computes
   * the value to return in the getValue method.
   */
  virtual void finalize() override;

protected:
  /// The variable from which a values is to be extracted
  MooseVariable & _var;

  /// The value of the desired variable
  const VariableValue & _u;

  /// A convenience reference to the Mesh this object operates on
  MooseMesh & _mesh;

  /// The point to locate, stored as a vector for use with reinitElemPhys
  std::vector<Point> _point_vec;

  /// The value of the variable at the desired location
  Real _value;

  /// The processor id that owns the element that the point is located
  processor_id_type _root_id;

  /// The element that contains the located point
  dof_id_type _elem_id;
};

#endif /* POINTVALUE_H */
