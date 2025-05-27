//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
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

class InternalSideIndicatorBase : public Indicator,
                                  public NeighborCoupleable,
                                  public ScalarCoupleable
{
public:
  static InputParameters validParams();

  InternalSideIndicatorBase(const InputParameters & parameters);

  /**
   * Computes the indicator for the current side.
   */
  virtual void computeIndicator() override;

  virtual void finalize() override;

protected:
  /**
   * Whether or not the derived classes are acting on a finite volume variable or not.
   */
  virtual bool isVarFV() const = 0;

  MooseVariableFE<Real> & _field_var;

  /// Current element under consideration
  const Elem * const & _current_elem;
  /// The neighbor element across from the current side
  const Elem * const & _neighbor_elem;

  /// Current side
  const unsigned int & _current_side;
  /// Current side element
  const Elem * const & _current_side_elem;

  /// Coordinate system
  const Moose::CoordinateSystemType & _coord_sys;
  /// The current quadrature point
  unsigned int _qp;
  /// The quadrature rule being used
  const QBase * const & _qrule;
  /// The spatial location of the current quadrature point
  const MooseArray<Point> & _q_point;
  /// The quadrature weight multiplied by the element Jacobian
  const MooseArray<Real> & _JxW;
  /// The coordinate transformation
  const MooseArray<Real> & _coord;

  /// The bounday (if any) that this indicator is operating on
  BoundaryID _boundary_id;

  /// Whether to scale the indicator value by the number of flux
  /// faces in an attempt to avoid penalizing internal faces
  bool _scale_by_flux_faces;

  /// Normal vectors at the quadrature points
  const MooseArray<Point> & _normals;

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
