#ifndef KERNEL_H_
#define KERNEL_H_

#include "Object.h"
#include "Integrable.h"
#include "Coupleable.h"
#include "FunctionInterface.h"

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
  public FunctionInterface
{
public:
  Kernel(const std::string & name, InputParameters parameters);
  virtual ~Kernel();

  virtual void computeResidual();
  virtual void computeJacobian(int i, int j);

  virtual unsigned int coupled(const std::string & var_name);
  virtual VariableValue & coupledValue(const std::string & var_name);

protected:
  Moose::SubProblem & _problem;
  Moose::System & _sys;

  THREAD_ID _tid;

  Moose::Variable & _var;
  Moose::Variable & _test_var;

  const Elem * & _current_elem;

  unsigned int _qp;
  const std::vector< Point > & _q_point;
  QBase * & _qrule;
  const std::vector<Real> & _JxW;

  Real & _t;

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

  VariableGrad & _grad_u;                               /// Holds the solution gradient at the current quadrature points
  VariableGrad & _grad_u_old;                           /// Holds the previous solution gradient at the current quadrature point.
  VariableGrad & _grad_u_older;                         /// Holds the t-2 solution gradient at the current quadrature point.

  VariableValue & _u_dot;                               /// Time derivative of u
  VariableValue & _du_dot_du;                           /// Derivative of u_dot wrt u


  virtual Real computeQpResidual() = 0;
  virtual Real computeQpJacobian();
};

template<>
InputParameters validParams<Kernel>();

#endif /* KERNEL_H_ */
