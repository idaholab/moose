//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// local includes
#include "Indicator.h"
#include "NeighborCoupleable.h"
#include "ScalarCoupleable.h"
#include "NeighborMooseVariableInterface.h"

template <typename>
class MooseVariableFE;
typedef MooseVariableFE<Real> MooseVariable;
typedef MooseVariableFE<VectorValue<Real>> VectorMooseVariable;

/**
 * The InternalSideIndicator class is responsible for calculating the residuals for various
 * physics on internal sides (edges/faces).
 *
 */
class InternalSideIndicator : public Indicator,
                              public NeighborCoupleable,
                              public ScalarCoupleable,
                              public NeighborMooseVariableInterface<Real>
{
public:
  /**
   * Factory constructor initializes all internal references needed for indicator computation.
   *
   */
  static InputParameters validParams();

  InternalSideIndicator(const InputParameters & parameters);

  /**
   * Computes the indicator for the current side.
   */
  virtual void computeIndicator() override;

  virtual void finalize() override;

protected:
  MooseVariable & _field_var;

  const Elem * const & _current_elem;
  /// The neighboring element
  const Elem * const & _neighbor_elem;

  /// Current side
  const unsigned int & _current_side;
  /// Current side element
  const Elem * const & _current_side_elem;

  /// Coordinate system
  const Moose::CoordinateSystemType & _coord_sys;
  unsigned int _qp;
  const MooseArray<Point> & _q_point;
  const QBase * const & _qrule;
  const MooseArray<Real> & _JxW;
  const MooseArray<Real> & _coord;

  BoundaryID _boundary_id;

  MooseVariableField<Real> & _var;

  bool _scale_by_flux_faces;

  /// Holds the current solution at the current quadrature point on the face.
  const VariableValue & _u;

  /// Holds the current solution gradient at the current quadrature point on the face.
  const VariableGradient & _grad_u;

  /// Normal vectors at the quadrature points
  const MooseArray<Point> & _normals;

  /// Holds the current solution at the current quadrature point
  const VariableValue & _u_neighbor;

  /// Holds the current solution gradient at the current quadrature point
  const VariableGradient & _grad_u_neighbor;

  /**
   * The virtual function you will want to override to compute error contributions.
   * This is called once per quadrature point on each interior side of every element.
   *
   * You should return the error^2
   */
  virtual Real computeQpIntegral() = 0;

public:
  // boundary id used for internal edges (all DG kernels lives on this boundary id -- a made-up
  // number)
  static const BoundaryID InternalBndId;
};
