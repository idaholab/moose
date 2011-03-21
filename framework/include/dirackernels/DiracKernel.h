#ifndef DIRACKERNEL_H_
#define DIRACKERNEL_H_

#include <string>

//MOOSE includes
#include "Moose.h"
#include "DiracKernelData.h"
#include "DiracKernelInfo.h"
#include "Object.h"
#include "Integrable.h"
#include "Coupleable.h"
#include "FunctionInterface.h"
#include "MaterialPropertyInterface.h"
#include "TransientInterface.h"
#include "Variable.h"

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
  public Object,
  public Moose::Coupleable,
  public FunctionInterface,
  public Moose::TransientInterface,
  public Moose::MaterialPropertyInterface
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
  
  Moose::SubProblem & _problem;

//  Moose::DiracKernelData & _dirac_kernel_data;
//  Moose::DiracKernelInfo & _dirac_kernel_info;
//
//  /**
//   * The name of the variable this DiracKernel acts on.
//   */
//  std::string _var_name;
//
  /**
   * The list of elements that need distributions.
   */
  std::set<const Elem *> _elements;

  /**
   * The list of physical xyz Points that need to be evaluated in each element.
   */
  std::map<const Elem *, std::set<Point> > _points;
//
//  ////////////////////////
//
//  DofData & _dof_data;                          /// Convenience reference to the DofData object inside of MooseSystem
//
//  Moose::Array<Real> & _u;                      /// Holds the current solution at the current quadrature point.
//  Real _u_node;                                 /// The value of _u at a nodal position.  Used by non-integrated boundaries.
//
//  Moose::Array<RealGradient> & _grad_u;         /// Holds the current solution gradient at the current quadrature point.
//  Moose::Array<RealTensor> & _second_u;         /// Holds the current solution second derivative at the current quadrature point.
//
//  Moose::Array<Real> & _u_dot;                  /// Time derivative of u
//  Moose::Array<Real> & _du_dot_du;              /// Derivative of u_dot wrt u
//
//  Moose::Array<Real> & _u_old;                  /// Holds the previous solution at the current quadrature point.
//  MooseArray<RealGradient> & _grad_u_old;       /// Holds the previous solution gradient at the current quadrature point.
//
//  MooseArray<Real> & _u_older;                  /// Holds the t-2 solution at the current quadrature point.
//  MooseArray<RealGradient> & _grad_u_older;     /// Holds the t-2 solution gradient at the current quadrature point.
//
//  /**
//   * Interior test function.
//   *
//   * These are non-const so they can be modified for stabilization.
//   */
//  std::vector<std::vector<Real> > & _test;
//
//  /**
//   * Gradient of interior test function.
//   */
//  const std::vector<std::vector<RealGradient> > & _grad_test;
//
//  /**
//   * Second derivative of interior test function.
//   */
//  const std::vector<std::vector<RealTensor> > & _second_test;
//
//  /**
//   * The points on the current element.
//   */
//  std::vector<Point> & _current_points;

  /**
   * The current point.
   */
  Point _current_point;
};
 
#endif
