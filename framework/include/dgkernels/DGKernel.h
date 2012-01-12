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
#include "Moose.h"
#include "MooseArray.h"
#include "MooseObject.h"
#include "SetupInterface.h"
#include "MooseVariable.h"
#include "Coupleable.h"
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
  public Coupleable,
  public NeighborCoupleable,
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
  MooseVariable & variable() { return _var; }

  SubProblem & subProblem() { return _subproblem; }

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
  Problem & _problem;
  SubProblem & _subproblem;
  SystemBase & _sys;

  THREAD_ID _tid;

  Assembly & _assembly;
  MooseVariable & _var;
  MooseMesh & _mesh;
  unsigned int _dim;

  const Elem * & _current_elem;
  const Elem * & _neighbor_elem;        ///< The neighboring element

  unsigned int & _current_side;         ///< Current side
  const Elem * & _current_side_elem;    ///< Current side element

  Moose::CoordinateSystemType & _coord_sys;                     ///< Coordinate system
  unsigned int _qp;
  const std::vector< Point > & _q_point;
  QBase * & _qrule;
  const std::vector<Real> & _JxW;
  const std::vector<Real> & _coord;

  unsigned int _i, _j;

  unsigned int _boundary_id;                                    ///<

  VariableValue & _u;                                           ///< Holds the current solution at the current quadrature point on the face.
  VariableValue & _u_old;                                       ///< Holds the previous solution at the current quadrature point.
  VariableValue & _u_older;                                     ///< Holds the t-2 solution at the current quadrature point.

  VariableGradient & _grad_u;                                   ///< Holds the current solution gradient at the current quadrature point on the face.
  VariableGradient & _grad_u_old;                               ///< Holds the previous solution gradient at the current quadrature point.
  VariableGradient & _grad_u_older;                             ///< Holds the t-2 solution gradient at the current quadrature point.

  VariableSecond & _second_u;                                   ///< Holds the current solution second derivative at the current quadrature point on the face
  VariableSecond & _second_u_old;
  VariableSecond & _second_u_older;
  // shape functions
  const std::vector<std::vector<Real> > & _phi;
  const std::vector<std::vector<RealGradient> > & _grad_phi;
  const std::vector<std::vector<RealTensor> > & _second_phi;
  // test functions
  const std::vector<std::vector<Real> > & _test;                        ///< Side shape function.
  const std::vector<std::vector<RealGradient> > & _grad_test;           ///< Gradient of side shape function
  const std::vector<std::vector<RealTensor> > & _second_test;           ///< Second derivative of side shape function
  const std::vector<Point>& _normals;                                   ///< Normal vectors at the quadrature points

  const std::vector<std::vector<Real> > & _phi_neighbor;                ///< Side shape function.
  const std::vector<std::vector<RealGradient> > & _grad_phi_neighbor;   ///< Gradient of side shape function
  const std::vector<std::vector<RealTensor> > & _second_phi_neighbor;   ///< Second derivative of side shape function

  const std::vector<std::vector<Real> > & _test_neighbor;               ///< Side test function
  const std::vector<std::vector<RealGradient> > & _grad_test_neighbor;  ///< Gradient of side shape function
  const std::vector<std::vector<RealTensor> > & _second_test_neighbor;  ///< Second derivative of side shape function

  VariableValue & _u_neighbor;                                          ///< Holds the current solution at the current quadrature point
  VariableValue & _u_old_neighbor;                                      ///< Holds the previous solution at the current quadrature point.
  VariableValue & _u_older_neighbor;                                    ///< Holds the t-2 solution at the current quadrature point.

  VariableGradient & _grad_u_neighbor;                                  ///< Holds the current solution gradient at the current quadrature point
  VariableGradient & _grad_u_old_neighbor;                              ///< Holds the previous solution gradient at the current quadrature point
  VariableGradient & _grad_u_older_neighbor;                            ///< Holds the t-2 solution gradient at the current quadrature point

  VariableSecond  & _second_u_neighbor;                                 ///< Holds the current solution second derivative at the current quadrature point


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
  // boundary id used for internal edges (all DG kernels lives on this boundary id)
  static const unsigned int InternalBndId;                              ///< Made-up number for internal edges
};

#endif //DGKERNEL_H
