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

// MOOSE includes
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
#include "Restartable.h"
#include "ZeroInterface.h"
#include "MeshChangedInterface.h"

// Forward Declarations
class Assembly;
class DiracKernel;
class SubProblem;
class MooseMesh;

template <>
InputParameters validParams<DiracKernel>();

/**
 * A DiracKernel is used when you need to add contributions to the residual by means of
 * multiplying some number by the shape functions on an element and adding the value into
 * the residual vector at the places associated with that shape function.
 *
 * This is common in point sources / sinks and various other algorithms.
 */
class DiracKernel : public MooseObject,
                    public SetupInterface,
                    public CoupleableMooseVariableDependencyIntermediateInterface,
                    public FunctionInterface,
                    public UserObjectInterface,
                    public TransientInterface,
                    public MaterialPropertyInterface,
                    public PostprocessorInterface,
                    protected GeometricSearchInterface,
                    public Restartable,
                    public ZeroInterface,
                    public MeshChangedInterface
{
public:
  DiracKernel(const InputParameters & parameters);
  virtual ~DiracKernel() {}

  /**
   * Computes the residual for the current element.
   */
  virtual void computeResidual();

  /**
   * Computes the jacobian for the current element.
   */
  virtual void computeJacobian();

  /**
   * This gets called by computeOffDiagJacobian() at each quadrature point.
   */
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  /**
   * Computes the off-diagonal Jacobian for variable jvar.
   */
  virtual void computeOffDiagJacobian(unsigned int jvar);

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
   * Whether or not this DiracKernel has something to distribute on this element.
   */
  bool hasPointsOnElem(const Elem * elem);

  /**
   * Whether or not this DiracKernel has something to distribute at this Point.
   */
  bool isActiveAtPoint(const Elem * elem, const Point & p);

  /**
   * Remove all of the current points and elements.
   */
  void clearPoints();

protected:
  /**
   * This is the virtual that derived classes should override for computing the residual.
   */
  virtual Real computeQpResidual() = 0;

  /**
   * This is the virtual that derived classes should override for computing the Jacobian.
   */
  virtual Real computeQpJacobian();

  /**
   * Add the physical x,y,z point located in the element "elem" to the list of points
   * this DiracKernel will be asked to evaluate a value at.
   */
  void addPoint(const Elem * elem, Point p, unsigned id = libMesh::invalid_uint);

  /**
   * This is a highly inefficient way to add a point where this DiracKernel needs to be
   * evaluated.
   *
   * This spawns a search for the element containing that point!
   */
  const Elem * addPoint(Point p, unsigned id = libMesh::invalid_uint);

  /**
   * Returns the user-assigned ID of the current Dirac point if it
   * exits, and libMesh::invalid_uint otherwise.  Can be used e.g. in
   * the computeQpResidual() function to determine the cached ID of
   * the current point, in case this information is relevant.
   */
  unsigned currentPointCachedID();

  SubProblem & _subproblem;
  SystemBase & _sys;

  THREAD_ID _tid;

  Assembly & _assembly;

  /// Variable this kernel acts on
  MooseVariable & _var;

  /// Mesh this kernels acts on
  MooseMesh & _mesh;

  /// Coordinate system
  const Moose::CoordinateSystemType & _coord_sys;

  /// Place for storing Point/Elem information shared across all
  /// DiracKernel objects.
  DiracKernelInfo & _dirac_kernel_info;

  /// Place for storing Point/Elem information only for this DiracKernel
  DiracKernelInfo _local_dirac_kernel_info;

  /// The current point
  Point _current_point;

  ///< Current element
  const Elem *& _current_elem;

  /// Quadrature point index
  unsigned int _qp;
  /// Quadrature points
  const MooseArray<Point> & _q_point;
  /// Physical points
  const MooseArray<Point> & _physical_point;
  /// Quadrature rule
  QBase *& _qrule;
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
  const VariableValue & _u;
  /// Holds the solution gradient at the current quadrature points
  const VariableGradient & _grad_u;

  /// Time derivative of the solution
  const VariableValue & _u_dot;
  /// Derivative of u_dot wrt u
  const VariableValue & _du_dot_du;

  /// drop duplicate points or consider them in residual and Jacobian
  const bool _drop_duplicate_points;

private:
  /// Data structure for caching user-defined IDs which can be mapped to
  /// specific std::pair<const Elem*, Point> and avoid the PointLocator Elem lookup.
  typedef std::map<unsigned, std::pair<const Elem *, Point>> point_cache_t;
  point_cache_t _point_cache;

  /// Map from Elem* to a list of (Dirac point, id) pairs which can be used
  /// in a user's computeQpResidual() routine to determine the user-defined ID for
  /// the current Dirac point, if one exists.
  typedef std::map<const Elem *, std::vector<std::pair<Point, unsigned>>> reverse_cache_t;
  reverse_cache_t _reverse_point_cache;

  /// This function is used internally when the Elem for a
  /// locally-cached point needs to be updated.  You must pass in a
  /// pointer to the old_elem whose data is to be updated, the
  /// new_elem to which the Point belongs, and the Point and id
  /// information.
  void updateCaches(const Elem * old_elem, const Elem * new_elem, Point p, unsigned id);

  /// A helper function for addPoint(Point, id) for when
  /// id != invalid_uint.
  const Elem * addPointWithValidId(Point p, unsigned id);
};

#endif
