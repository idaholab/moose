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

  MooseVariable & _var;
  MooseMesh & _mesh;

  int _dim;

  DiracKernelInfo & _dirac_kernel_info;


  /**
   * The list of elements that need distributions.
   */
  std::set<const Elem *> _elements;

  /**
   * The list of physical xyz Points that need to be evaluated in each element.
   */
  std::map<const Elem *, std::set<Point> > _points;
  
  /**
   * The points on the current element.
   */
  //std::vector<Point> & _current_points;

  /**
   * The current point.
   */
  Point _current_point;

  const Elem * & _current_elem;

  unsigned int _qp;
  const std::vector< Point > & _q_point;
  QBase * & _qrule;
  const std::vector<Real> & _JxW;

   unsigned int _i, _j;
  // shape functions
  const std::vector<std::vector<Real> > & _phi;
  const std::vector<std::vector<RealGradient> > & _grad_phi;
  const std::vector<std::vector<RealTensor> > & _second_phi;
  // test functions
  const std::vector<std::vector<Real> > & _test;
  const std::vector<std::vector<RealGradient> > & _grad_test;
  const std::vector<std::vector<RealTensor> > & _second_test;

  VariableValue & _u;                                   /// Holds the solution at current quadrature points
  VariableValue & _u_old;                               /// Holds the previous solution at the current quadrature point.
  VariableValue & _u_older;                             /// Holds the t-2 solution at the current quadrature point.

  VariableGradient & _grad_u;                               /// Holds the solution gradient at the current quadrature points
  VariableGradient & _grad_u_old;                           /// Holds the previous solution gradient at the current quadrature point.
  VariableGradient & _grad_u_older;                         /// Holds the t-2 solution gradient at the current quadrature point.

  VariableSecond & _second_u;
  VariableSecond & _second_u_old;
  VariableSecond & _second_u_older;

  VariableValue & _u_dot;                               /// Time derivative of u
  VariableValue & _du_dot_du;                           /// Derivative of u_dot wrt u

  // Single Instance Variables
  Real & _real_zero;
  Array<Real> & _zero;
  Array<RealGradient> & _grad_zero;
  Array<RealTensor> & _second_zero;
};
 
#endif
