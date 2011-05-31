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
#include "Moose.h"
#include "DiracKernelData.h"
#include "DiracKernelInfo.h"
#include "MooseObject.h"
#include "Coupleable.h"
#include "FunctionInterface.h"
#include "MaterialPropertyInterface.h"
#include "TransientInterface.h"
#include "GeometricSearchInterface.h"
#include "MooseVariable.h"
#include "SubProblemInterface.h"
#include "MooseMesh.h"

//libMesh includes
#include "libmesh_common.h"
#include "elem.h"
#include "point.h"
#include "sparse_matrix.h"
//#include "ValidParams.h"

//Forward Declarations
class AsmBlock;
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
  public Coupleable,
  public FunctionInterface,
  public TransientInterface,
  public MaterialPropertyInterface,
  protected GeometricSearchInterface
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
   * The variable number that this object operates on.
   */
  MooseVariable & variable() { return _var; }

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
  void addPoint(Point p);  

  Problem & _problem;
  SubProblemInterface & _subproblem;
  SystemBase & _sys;

  THREAD_ID _tid;

  AsmBlock & _asmb;
  MooseVariable & _var;                                 ///< Variable this kernel acts on
  MooseMesh & _mesh;                                    ///< Mesh this kernels acts on

  int _dim;                                             ///< Dimension of the problem

  DiracKernelInfo & _dirac_kernel_info;

  std::set<const Elem *> _elements;                     ///< The list of elements that need distributions
  std::map<const Elem *, std::set<Point> > _points;     ///< The list of physical xyz Points that need to be evaluated in each element
  
  //std::vector<Point> & _current_points;               ///< The points on the current element

  Point _current_point;                                 ///< The current point

  const Elem * & _current_elem;                         ///< Current element

  unsigned int _qp;                                     ///< Quadrature point index
  const std::vector< Point > & _q_point;                ///< Quadrature points
  const std::vector< Point > & _physical_point;         ///< Physical points
  QBase * & _qrule;                                     ///< Quadrature rule
  const std::vector<Real> & _JxW;                       ///< Transformed Jacobian weights

   unsigned int _i, _j;                                         ///< i-th, j-th index for enumerating shape and test functions
  // shape functions
  const std::vector<std::vector<Real> > & _phi;                 ///< Values of shape functions at QPs
  const std::vector<std::vector<RealGradient> > & _grad_phi;    ///< Gradients of shape functions at QPs
  const std::vector<std::vector<RealTensor> > & _second_phi;    ///< Second derivatives of shape functions at QPs
  // test functions
  const std::vector<std::vector<Real> > & _test;                ///< Values of test functions at QPs
  const std::vector<std::vector<RealGradient> > & _grad_test;   ///< Gradients of test functions at QPs
  const std::vector<std::vector<RealTensor> > & _second_test;   ///< Second derivatives of shape functions at QPs

  VariableValue & _u;                                   ///< Holds the solution at current quadrature points
  VariableValue & _u_old;                               ///< Holds the previous solution at the current quadrature point.
  VariableValue & _u_older;                             ///< Holds the t-2 solution at the current quadrature point.

  VariableGradient & _grad_u;                           ///< Holds the solution gradient at the current quadrature points
  VariableGradient & _grad_u_old;                       ///< Holds the previous solution gradient at the current quadrature point.
  VariableGradient & _grad_u_older;                     ///< Holds the t-2 solution gradient at the current quadrature point.

  VariableSecond & _second_u;                           ///< Second derivatives of the solution at QPs
  VariableSecond & _second_u_old;                       ///< Second derivatives of the previous solution at QPs
  VariableSecond & _second_u_older;                     ///< Second derivatives of the t-2 solution at QPs

  VariableValue & _u_dot;                               ///< Time derivative of the solution
  VariableValue & _du_dot_du;                           ///< Derivative of u_dot wrt u

  // Single Instance Variables
  Real & _real_zero;                                    ///< Scalar zero
  MooseArray<Real> & _zero;                             ///< Scalar zero at QPs
  MooseArray<RealGradient> & _grad_zero;                ///< Zero gradient at QPs
  MooseArray<RealTensor> & _second_zero;                ///< Zero second derivative at QPs
};
 
#endif
