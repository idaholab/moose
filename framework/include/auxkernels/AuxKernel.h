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

#ifndef AUXKERNEL_H
#define AUXKERNEL_H

#include "MooseObject.h"
#include "Coupleable.h"
#include "MaterialPropertyInterface.h"
#include "FunctionInterface.h"
#include "TransientInterface.h"
#include "PostprocessorInterface.h"
#include "GeometricSearchInterface.h"
#include "MooseMesh.h"

//forward declarations
class Problem;
class SubProblem;
class AuxKernel;
class AuxiliarySystem;
class SystemBase;

template<>
InputParameters validParams<AuxKernel>();

/**
 * Base class for creating new auxiliary kernels and auxiliary boundary conditions.
 *
 */
class AuxKernel :
  public MooseObject,
  public Coupleable,
  public FunctionInterface,
  public TransientInterface,
  public MaterialPropertyInterface,
  public PostprocessorInterface,
  protected GeometricSearchInterface
{
public:
  AuxKernel(const std::string & name, InputParameters parameters);

  virtual ~AuxKernel() {}

  /**
   * Computes the value and stores it in the solution vector
   */
  virtual void compute();

  /**
   * Get a reference to a variable this kernel is action on
   * @return reference to a variable this kernel is action on
   */
  MooseVariable & variable() { return _var; }

  /**
   * Nodal or elemental kernel?
   * @return true if this is a nodal kernel, otherwise false
   */
  bool isNodal();

protected:
  virtual Real computeValue() = 0;

  Problem & _problem;                                   ///< Problem this kernel is part of
  SubProblem & _subproblem;                             ///< Subproblem this kernel is part of
  SystemBase & _sys;                                    ///< System this kernel is part of
  SystemBase & _nl_sys;
  AuxiliarySystem & _aux_sys;
  THREAD_ID _tid;                                       ///< Thread ID
  MooseVariable & _var;                                 ///< Variable this kernel is acting on
  bool _nodal;                                          ///< true if the kernel nodal, false if it is elemental
  MooseMesh & _mesh;                                    ///< Mesh this kernel is active on
  unsigned int _dim;                                    ///< Dimension of the problem being solved

  const std::vector< Point > & _q_point;                ///< Active quadrature points
  QBase * & _qrule;                                     ///< Quadrature rule being used
  const std::vector<Real> & _JxW;                       ///< Transformed Jacobian weights

  VariableValue & _u;                                   ///< Holds the solution at current quadrature points
  VariableValue & _u_old;                               ///< Holds the previous solution at the current quadrature point.
  VariableValue & _u_older;                             ///< Holds the t-2 solution at the current quadrature point.

  const Elem * & _current_elem;                         ///< Current element (valid only for elemental kernels)
  const Node * & _current_node;                         ///< Current node (valid only for nodal kernels)

  Real & _current_volume;                               ///< Volume of the current element

  NumericVector<Number> & _solution;                    ///< reference to the solution vector of auxiliary system

  unsigned int _qp;                                     ///< Quadrature point index

  // Single Instance Variables
  Real & _real_zero;                                    ///< Scalar zero
  MooseArray<Real> & _zero;                             ///< Scalar zero in quadrature points
  MooseArray<RealGradient> & _grad_zero;                ///< Zero gradient in quadrature points
  MooseArray<RealTensor> & _second_zero;                ///< Zero second derivative in quadrature points
};

#endif //AUXKERNEL_H
