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

#ifndef KERNEL_H
#define KERNEL_H

#include "MooseObject.h"
#include "SetupInterface.h"
#include "Coupleable.h"
#include "MooseVariableInterface.h"
#include "FunctionInterface.h"
#include "TransientInterface.h"
#include "MaterialPropertyInterface.h"
#include "PostprocessorInterface.h"
#include "GeometricSearchInterface.h"
#include "Assembly.h"
#include "MooseVariable.h"
#include "SubProblem.h"

// libMesh
#include "fe.h"
#include "quadrature.h"

class MooseMesh;
class Problem;
class SubProblem;


class Kernel;

template<>
InputParameters validParams<Kernel>();

class Kernel :
  public MooseObject,
  public SetupInterface,
  public Coupleable,
  public ScalarCoupleable,
  public MooseVariableInterface,
  public FunctionInterface,
  public TransientInterface,
  public PostprocessorInterface,
  public MaterialPropertyInterface,
  protected GeometricSearchInterface
{
public:
  Kernel(const std::string & name, InputParameters parameters);

  virtual void computeResidual();
  virtual void computeJacobian();
  /**
   * Computes d-residual / d-jvar... storing the result in Ke.
   */
  virtual void computeOffDiagJacobian(unsigned int jvar);

  /**
   * The variable number that this kernel operates on.
   */
  MooseVariable & variable() { return _var; }

  /**
   * The time, after which this kernel will be active.
   */
  Real startTime();

  /**
   * The time, after which this kernel will be inactive.
   */
  Real stopTime();

  SubProblem & subProblem() { return _subproblem; }

  // materials
  template<typename T>
  MaterialProperty<T> & getMaterialProperty(const std::string & name);


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

  unsigned int _qp;
  const std::vector< Point > & _q_point;
  QBase * & _qrule;
  const std::vector<Real> & _JxW;
  const std::vector<Real> & _coord;

  unsigned int _i, _j;
  // shape functions
  const std::vector<std::vector<Real> > & _phi;
  const std::vector<std::vector<RealGradient> > & _grad_phi;
  const std::vector<std::vector<RealTensor> > & _second_phi;
  // test functions
  const std::vector<std::vector<Real> > & _test;
  const std::vector<std::vector<RealGradient> > & _grad_test;
  const std::vector<std::vector<RealTensor> > & _second_test;

  VariableValue & _u;                                   ///< Holds the solution at current quadrature points
  VariableGradient & _grad_u;                           ///< Holds the solution gradient at the current quadrature points

  VariableValue & _u_dot;                               ///< Time derivative of u
  VariableValue & _du_dot_du;                           ///< Derivative of u_dot wrt u

  Real _start_time;                                     ///< The time, after which this kernel will be active.
  Real _stop_time;                                      ///< The time, after which this kernel will be inactive.

  // Single Instance Variables
  Real & _real_zero;
  MooseArray<Real> & _zero;
  MooseArray<RealGradient> & _grad_zero;
  MooseArray<RealTensor> & _second_zero;

  virtual Real computeQpResidual() = 0;
  virtual Real computeQpJacobian();

  /**
   * This is the virtual that derived classes should override for computing an off-diagonal jacobian component.
   */
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  virtual void precalculateResidual();
};


template<typename T>
MaterialProperty<T> &
Kernel::getMaterialProperty(const std::string & name)
{
  if (parameters().isParamValid("block"))
  {
    // check blocks where the kernel is defined
    std::vector<unsigned int> blocks = parameters().get<std::vector<unsigned int> >("block");
    for (std::vector<unsigned int>::iterator it = blocks.begin(); it != blocks.end(); ++it)
      _subproblem.checkMatProp(*it, name);
  }
  else
  {
    // no kernel blocks specified, check all blocks that are in the mesh
    const std::set<subdomain_id_type> & blocks = _mesh.meshSubdomains();
    for (std::set<subdomain_id_type>::const_iterator it = blocks.begin(); it != blocks.end(); ++it)
      _subproblem.checkMatProp(*it, name);
  }

  return MaterialPropertyInterface::getMaterialProperty<T>(name);
}

#endif /* KERNEL_H */
