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

#ifndef DGKERNEL_H
#define DGKERNEL_H

// local includes
#include "MooseArray.h"
#include "MooseObject.h"
#include "SetupInterface.h"
#include "TransientInterface.h"
#include "UserObjectInterface.h"
#include "NeighborCoupleableMooseVariableDependencyIntermediateInterface.h"
#include "FunctionInterface.h"
#include "MooseVariableInterface.h"
#include "TwoMaterialPropertyInterface.h"
#include "Assembly.h"

class MooseMesh;
class Problem;
class SubProblem;

//Forward Declarations
class DGKernel;

template<>
InputParameters validParams<DGKernel>();

/**
 * The DGKernel class is responsible for calculating the residuals for various
 * physics on internal sides (edges/faces).
 *
 */
class DGKernel :
  public MooseObject,
  public SetupInterface,
  public TransientInterface,
  public FunctionInterface,
  public UserObjectInterface,
  public NeighborCoupleableMooseVariableDependencyIntermediateInterface,
  protected TwoMaterialPropertyInterface
{
public:

  /**
   * Factory constructor initializes all internal references needed for residual computation.
   *
   *
   * @param name The name of this kernel.
   * @param parameters The parameters object for holding additional parameters for kernels and derived kernels
   */
  DGKernel(const std::string & name, InputParameters parameters);

  virtual ~DGKernel();

  /**
   * The variable number that this kernel operates on.
   */
  MooseVariable & variable();

  /**
   * Return a reference to the subproblem.
   */
  SubProblem & subProblem();

  /**
   * Computes the residual for the current side.
   */
  virtual void computeResidual();

  /**
   * Computes the jacobian for the current side.
   */
  virtual void computeJacobian();

  /**
   * Computes d-residual / d-jvar...
   */
  virtual void computeOffDiagJacobian(unsigned int jvar);

protected:
  SubProblem & _subproblem;
  SystemBase & _sys;

  THREAD_ID _tid;

  Assembly & _assembly;
  MooseVariable & _var;
  MooseMesh & _mesh;
  unsigned int _dim;

  const Elem * & _current_elem;

  /// The volume (or length) of the current element
  const Real & _current_elem_volume;

  /// The neighboring element
  const Elem * & _neighbor_elem;

  /// The volume (or length) of the current neighbor
  const Real & _neighbor_elem_volume;

  /// Current side
  unsigned int & _current_side;
  /// Current side element
  const Elem * & _current_side_elem;

  /// The volume (or length) of the current side
  const Real & _current_side_volume;

  /// Coordinate system
  const Moose::CoordinateSystemType & _coord_sys;
  unsigned int _qp;
  const MooseArray< Point > & _q_point;
  QBase * & _qrule;
  const MooseArray<Real> & _JxW;
  const MooseArray<Real> & _coord;

  unsigned int _i, _j;

  BoundaryID _boundary_id;

  /// Holds the current solution at the current quadrature point on the face.
  VariableValue & _u;

  /// Holds the current solution gradient at the current quadrature point on the face.
  VariableGradient & _grad_u;
  // shape functions
  const VariablePhiValue & _phi;
  const VariablePhiGradient & _grad_phi;
  // test functions

  /// Side shape function.
  const VariableTestValue & _test;
  /// Gradient of side shape function
  const VariableTestGradient & _grad_test;
  /// Normal vectors at the quadrature points
  const MooseArray<Point>& _normals;

  /// Side shape function.
  const VariablePhiValue & _phi_neighbor;
  /// Gradient of side shape function
  const VariablePhiGradient & _grad_phi_neighbor;

  /// Side test function
  const VariableTestValue & _test_neighbor;
  /// Gradient of side shape function
  const VariableTestGradient & _grad_test_neighbor;

  /// Holds the current solution at the current quadrature point
  VariableValue & _u_neighbor;
  /// Holds the current solution gradient at the current quadrature point
  VariableGradient & _grad_u_neighbor;

  /**
   * This is the virtual that derived classes should override for computing the residual on neighboring element.
   */
  virtual Real computeQpResidual(Moose::DGResidualType type) = 0;

  /**
   * This is the virtual that derived classes should override for computing the Jacobian on neighboring element.
   */
  virtual Real computeQpJacobian(Moose::DGJacobianType type) = 0;

  /**
   * This is the virtual that derived classes should override for computing the off-diag Jacobian.
   */
  virtual Real computeQpOffDiagJacobian(Moose::DGJacobianType type, unsigned int jvar);

public:
  // boundary id used for internal edges (all DG kernels lives on this boundary id -- a made-up number)
  static const BoundaryID InternalBndId;
};

#endif //DGKERNEL_H
