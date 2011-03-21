#ifndef KERNEL_H_
#define KERNEL_H_

#include "Object.h"
#include "Integrable.h"
#include "Coupleable.h"
#include "FunctionInterface.h"
#include "TransientInterface.h"
#include "MaterialPropertyInterface.h"
#include "Variable.h"

// libMesh
#include "fe.h"
#include "quadrature.h"


namespace Moose {
class Variable;
class SubProblem;
class System;
}

class Kernel :
  public Object,
  public Moose::Integrable,
  public Moose::Coupleable,
  public FunctionInterface,
  public Moose::TransientInterface,
  public Moose::MaterialPropertyInterface
{
public:
  Kernel(const std::string & name, InputParameters parameters);
  virtual ~Kernel();

  virtual void computeResidual();
  virtual void computeJacobian(int i, int j);

  virtual unsigned int coupled(const std::string & var_name);
  virtual VariableValue & coupledValue(const std::string & var_name);
  virtual VariableGradient  & coupledGradient(const std::string & var_name);

  /**
   * The variable number that this kernel operates on.
   */
  Moose::Variable & variable() { return _var; }

  /**
   * The time, after which this kernel will be active.
   */
  Real startTime();

  /**
   * The time, after which this kernel will be inactive.
   */
  Real stopTime();

protected:
  Moose::SubProblem & _problem;
  Moose::System & _sys;

  THREAD_ID _tid;

  Moose::Variable & _var;
  Moose::Variable & _test_var;
  int _dim;

  const Elem * & _current_elem;

  unsigned int _qp;
  const std::vector< Point > & _q_point;
  QBase * & _qrule;
  const std::vector<Real> & _JxW;

   unsigned int _i, _j;
  // shape functions
  const std::vector<std::vector<Real> > & _phi;
  const std::vector<std::vector<RealGradient> > & _grad_phi;
  // test functions
  const std::vector<std::vector<Real> > & _test;
  const std::vector<std::vector<RealGradient> > & _grad_test;

  VariableValue & _u;                                   /// Holds the solution at current quadrature points
  VariableValue & _u_old;                               /// Holds the previous solution at the current quadrature point.
  VariableValue & _u_older;                             /// Holds the t-2 solution at the current quadrature point.

  VariableGradient & _grad_u;                               /// Holds the solution gradient at the current quadrature points
  VariableGradient & _grad_u_old;                           /// Holds the previous solution gradient at the current quadrature point.
  VariableGradient & _grad_u_older;                         /// Holds the t-2 solution gradient at the current quadrature point.

  VariableValue & _u_dot;                               /// Time derivative of u
  VariableValue & _du_dot_du;                           /// Derivative of u_dot wrt u

  Real _start_time;                                     /// The time, after which this kernel will be active.
  Real _stop_time;                                      /// The time, after which this kernel will be inactive.

  virtual Real computeQpResidual() = 0;
  virtual Real computeQpJacobian();
};

template<>
InputParameters validParams<Kernel>();

#endif /* KERNEL_H_ */
