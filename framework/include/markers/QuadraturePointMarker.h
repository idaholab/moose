//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Marker.h"
#include "Coupleable.h"
#include "MaterialPropertyInterface.h"

class QuadraturePointMarker : public Marker,
                              public MooseVariableInterface<Real>,
                              public MaterialPropertyInterface
{
public:
  static InputParameters validParams();

  QuadraturePointMarker(const InputParameters & parameters);

protected:
  virtual MarkerValue computeElementMarker() override;

  /**
   * Override this to compute a marker value at each quadrature point.
   *
   * The ultimate value for the element will be taken as the maximum
   * (most conservative) value for all quadrature points on the element.
   *
   * @return The MarkerValue at one quadrature point.
   */
  virtual MarkerValue computeQpMarker() = 0;

  /// Holds the solution at current quadrature points
  const VariableValue & _u;

  /// The quadrature rule for the system
  const QBase * const & _qrule;

  /// Position of the current quadrature point
  const MooseArray<Point> & _q_point;

  /// The current quadrature point
  unsigned int _qp;

  /// The behavior to use when "in-between" other states (what to do on the fringe)
  MarkerValue _third_state;
};
