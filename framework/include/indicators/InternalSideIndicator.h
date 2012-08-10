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

#ifndef INTERNALSIDEINDICATOR_H
#define INTERNALSIDEINDICATOR_H

// local includes
#include "Moose.h"
#include "MooseArray.h"
#include "Indicator.h"
#include "TwoMaterialPropertyInterface.h"

#include "Assembly.h"
#include "MooseVariable.h"

class MooseMesh;
class Problem;
class SubProblem;

//Forward Declarations
class InternalSideIndicator;

template<>
InputParameters validParams<InternalSideIndicator>();

/**
 * The InternalSideIndicator class is responsible for calculating the residuals for various
 * physics on internal sides (edges/faces).
 *
 */
class InternalSideIndicator :
  public Indicator,
  public NeighborCoupleable,
  public NeighborMooseVariableInterface,
  protected TwoMaterialPropertyInterface
{
public:

  /**
   * Factory constructor initializes all internal references needed for indicator computation.
   *
   */
  InternalSideIndicator(const std::string & name, InputParameters parameters);

  virtual ~InternalSideIndicator();

  /**
   * Computes the indicator for the current side.
   */
  virtual void computeIndicator();

protected:

  const Elem * & _current_elem;
  /// The neighboring element
  const Elem * & _neighbor_elem;

  /// Current side
  unsigned int & _current_side;
  /// Current side element
  const Elem * & _current_side_elem;

  /// Coordinate system
  const Moose::CoordinateSystemType & _coord_sys;
  unsigned int _qp;
  const MooseArray< Point > & _q_point;
  QBase * & _qrule;
  const MooseArray<Real> & _JxW;
  const MooseArray<Real> & _coord;

//  unsigned int _i, _j;

  BoundaryID _boundary_id;

  /// Holds the current solution at the current quadrature point on the face.
  VariableValue & _u;

  /// Holds the current solution gradient at the current quadrature point on the face.
  VariableGradient & _grad_u;

  /// Normal vectors at the quadrature points
  const MooseArray<Point>& _normals;

  /// Holds the current solution at the current quadrature point
  VariableValue & _u_neighbor;
  /// Holds the current solution gradient at the current quadrature point
  VariableGradient & _grad_u_neighbor;

  /**
   * This is the virtual that derived classes should override for computing the indicator on neighboring element.
   */
//  virtual Real computeQpIndicator(Moose::DGResidualType type) = 0;
  virtual Real computeQpIndicator() = 0;

public:
  // boundary id used for internal edges (all DG kernels lives on this boundary id -- a made-up number)
  static const BoundaryID InternalBndId;
};

#endif //INTERNALSIDEINDICATOR_H
