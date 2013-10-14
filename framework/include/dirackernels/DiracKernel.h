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

#ifndef DIRACKERNEL_H
#define DIRACKERNEL_H

#include <string>

//MOOSE includes
#include "DiracKernelData.h"
#include "DiracKernelInfo.h"
#include "MooseObject.h"
#include "SetupInterface.h"
#include "CoupleableMooseVariableDependencyIntermediateInterface.h"
#include "FunctionInterface.h"
#include "UserObjectInterface.h"
#include "MaterialPropertyInterface.h"
#include "TransientInterface.h"
#include "PostprocessorInterface.h"
#include "GeometricSearchInterface.h"
#include "MooseVariable.h"
#include "SubProblem.h"
#include "MooseMesh.h"
#include "Restartable.h"
#include "Reportable.h"

//libMesh includes
#include "libmesh/libmesh_common.h"
#include "libmesh/elem.h"
#include "libmesh/point.h"
#include "libmesh/sparse_matrix.h"
//#include "ValidParams.h"

//Forward Declarations
class Assembly;
class DiracKernel;

template<>
InputParameters validParams<DiracKernel>();

/**
 * A DiracKernel is used when you need to add contributions to the residual by means of
 * multiplying some number by the shape functions on an element and adding the value into
 * the residual vector at the places associated with that shape function.
 *
 * This is common in point sources / sinks and various other algorithms.
 */
class DiracKernel :
  public MooseObject,
  public SetupInterface,
  public CoupleableMooseVariableDependencyIntermediateInterface,
  public FunctionInterface,
  public UserObjectInterface,
  public TransientInterface,
  public MaterialPropertyInterface,
  public PostprocessorInterface,
  protected GeometricSearchInterface,
  public Restartable,
  public Reportable
{
public:
  DiracKernel(const std::string & name, InputParameters parameters);
  virtual ~DiracKernel(){}

  /**
   * Computes the residual for the current element.
   */
  virtual void computeResidual();

  /**
   * Computes the jacobian for the current element.
   */
  virtual void computeJacobian();

  /**
   * The variable number that this kernel operates on.
   */
  MooseVariable & variable();

  /**
   * Return a reference to the subproblem.
   */
  SubProblem & subProblem();

  /**
   * This is where the DiracKernel should call addPoint() for each point it needs to have a
   * value distributed at.
   */
  virtual void addPoints() = 0;

  /**
   * This is the virtual that derived classes should override for computing the residual.
   */
  virtual Real computeQpResidual()=0;

  /**
   * This is the virtual that derived classes should override for computing the Jacobian.
   */
  virtual Real computeQpJacobian();

  /**
   * Whether or not this DiracKernel has something to distribute on this element.
   */
  bool hasPointsOnElem(const Elem * elem);

  /**
   * Whether or not this DiracKernel has something to distribute at this Point.
   */
  bool isActiveAtPoint(const Elem * elem, const Point & p);

//  /**
//   * Get a reference to a copy of the residual vector.
//   */
//  NumericVector<Number> & residualCopy();
//
//  /**
//   * Get a reference to a copy of the jacoian vector.
//   */
//  SparseMatrix<Number> & jacobianCopy();

  /**
   * Remove all of the current points and elements.
   */
  void clearPoints();

protected:
  /**
   * Add the physical x,y,z point located in the element "elem" to the list of points
   * this DiracKernel will be asked to evaluate a value at.
   */
  void addPoint(const Elem * elem, Point p);

  /**
   * This is a highly inefficient way to add a point where this DiracKernel needs to be
   * evaluated.
   *
   * This spawns a search for the element containing that point!
   */
  const Elem * addPoint(Point p);

  SubProblem & _subproblem;
  SystemBase & _sys;

  THREAD_ID _tid;

  Assembly & _assembly;
  /// Variable this kernel acts on
  MooseVariable & _var;
  /// Mesh this kernels acts on
  MooseMesh & _mesh;

  /// Dimension of the problem
  unsigned int _dim;
  /// Coordinate system
  const Moose::CoordinateSystemType & _coord_sys;

  DiracKernelInfo & _dirac_kernel_info;

  /// The list of elements that need distributions
  std::set<const Elem *> _elements;
  /// The list of physical xyz Points that need to be evaluated in each element
  std::map<const Elem *, std::set<Point> > _points;

  //std::vector<Point> & _current_points;               ///< The points on the current element

  /// The current point
  Point _current_point;

  ///< Current element
  const Elem * & _current_elem;

  /// Quadrature point index
  unsigned int _qp;
  /// Quadrature points
  const MooseArray< Point > & _q_point;
  /// Physical points
  const MooseArray< Point > & _physical_point;
  /// Quadrature rule
  QBase * & _qrule;
  /// Transformed Jacobian weights
  const MooseArray<Real> & _JxW;

  /// i-th, j-th index for enumerating shape and test functions
  unsigned int _i, _j;

  // shape functions

  /// Values of shape functions at QPs
  const VariablePhiValue & _phi;
  /// Gradients of shape functions at QPs
  const VariablePhiGradient & _grad_phi;

  // test functions

  /// Values of test functions at QPs
  const VariableTestValue & _test;
  /// Gradients of test functions at QPs
  const VariableTestGradient & _grad_test;

  /// Holds the solution at current quadrature points
  VariableValue & _u;
  /// Holds the solution gradient at the current quadrature points
  VariableGradient & _grad_u;

  /// Time derivative of the solution
  VariableValue & _u_dot;
  /// Derivative of u_dot wrt u
  VariableValue & _du_dot_du;

  // Single Instance Variables

  /// Scalar zero
  Real & _real_zero;
  /// Scalar zero at QPs
  MooseArray<Real> & _zero;
  /// Zero gradient at QPs
  MooseArray<RealGradient> & _grad_zero;
  /// Zero second derivative at QPs
  MooseArray<RealTensor> & _second_zero;
};

#endif
