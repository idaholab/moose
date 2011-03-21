#ifndef KERNEL_H
#define KERNEL_H

#include "MooseObject.h"
#include "Coupleable.h"
#include "FunctionInterface.h"
#include "TransientInterface.h"
#include "MaterialPropertyInterface.h"
#include "MooseVariable.h"

// libMesh
#include "fe.h"
#include "quadrature.h"

class MooseVariable;
class SubProblem;
class SystemBase;

class Kernel :
  public MooseObject,
  public Coupleable,
  public FunctionInterface,
  public TransientInterface,
  public MaterialPropertyInterface
{
public:
  Kernel(const std::string & name, InputParameters parameters);
  virtual ~Kernel();

  virtual void computeResidual();
  virtual void computeJacobian(int i, int j);
  /**
   * Computes d-residual / d-jvar... storing the result in Ke.
   */
  virtual void computeOffDiagJacobian(DenseMatrix<Number> & Ke, unsigned int jvar);

  unsigned int coupledComponents(const std::string & varname);
  virtual unsigned int coupled(const std::string & var_name, unsigned int comp = 0);

  virtual VariableValue & coupledValue(const std::string & var_name, unsigned int comp = 0);
  virtual VariableValue & coupledValueOld(const std::string & var_name, unsigned int comp = 0);
  virtual VariableValue & coupledValueOlder(const std::string & var_name, unsigned int comp = 0);

  virtual VariableGradient  & coupledGradient(const std::string & var_name, unsigned int comp = 0);
  virtual VariableGradient  & coupledGradientOld(const std::string & var_name, unsigned int comp = 0);
  virtual VariableGradient  & coupledGradientOlder(const std::string & var_name, unsigned int comp = 0);

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

protected:
  SubProblem & _problem;
  SystemBase & _sys;

  THREAD_ID _tid;

  MooseVariable & _var;
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

  Real _start_time;                                     /// The time, after which this kernel will be active.
  Real _stop_time;                                      /// The time, after which this kernel will be inactive.

  // Single Instance Variables
  Real & _real_zero;
  Array<Real> & _zero;
  Array<RealGradient> & _grad_zero;
  Array<RealTensor> & _second_zero;

  virtual Real computeQpResidual() = 0;
  virtual Real computeQpJacobian();

  /**
   * This is the virtual that derived classes should override for computing an off-diagonal jacobian component.
   */
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  virtual void precalculateResidual();
};

template<>
InputParameters validParams<Kernel>();

#endif /* KERNEL_H_ */
