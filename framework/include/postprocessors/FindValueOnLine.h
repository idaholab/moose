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

#ifndef FINDVALUEONLINE_H
#define FINDVALUEONLINE_H

#include "GeneralPostprocessor.h"
#include "Coupleable.h"

class FindValueOnLine;

template<>
InputParameters validParams<FindValueOnLine>();

/**
 * Find a specific target value along a sampling line. The variable values along
 * the line should change monotonically. The target value is searched using a
 * bisection algorithm.
 * The Postprocessor reports the distance from the start_point along the line
 * between start_point and end_point.
 */
class FindValueOnLine :
    public GeneralPostprocessor,
    public Coupleable,
    public MooseVariableDependencyInterface
{
public:
  FindValueOnLine(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override;

  virtual PostprocessorValue getValue() override;

protected:
  Real getValueAtPoint(const Point & p);

  ///@{ line to sample along
  const Point _start_point;
  const Point _end_point;
  const Real _length;
  ///@}

  /// value to find along the line
  const Real _target;

  /// search depth
  const unsigned int _depth;

  /// tolerance for comparison to the target value
  const Real _tol;

  /// coupled variable
  MooseVariable * _coupled_var;

  /// detected interface location
  Real _position;

  /// The Mesh we're using
  MooseMesh & _mesh;

  /// So we don't have to create and destroy the dummy vector
  std::vector<Point> _point_vec;

  /// helper object to locate elements containing points
  std::unique_ptr<PointLocatorBase> _pl;
};

#endif // FINDVALUEONLINE_H
