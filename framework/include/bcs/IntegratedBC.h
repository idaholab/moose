#ifndef INTEGRATEDBC_H_
#define INTEGRATEDBC_H_

#include "BoundaryCondition.h"
#include "Variable.h"

// libMesh
#include "fe.h"
#include "quadrature.h"

class IntegratedBC :
  public BoundaryCondition,
  public Moose::Integrable
{
public:
  IntegratedBC(const std::string & name, InputParameters parameters);
  virtual ~IntegratedBC();

  virtual void computeResidual();
  virtual void computeJacobian(int i, int j);

  virtual unsigned int coupled(const std::string & var_name);
  virtual VariableValue & coupledValue(const std::string & var_name);

protected:
  Moose::Variable & _test_var;

  unsigned int _qp;
  QBase * & _qrule;
  const std::vector< Point > & _q_point;
  const std::vector<Real> & _JxW;
  unsigned int _i, _j;

  // shape functions
  const std::vector<std::vector<Real> > & _phi;
  const std::vector<std::vector<RealGradient> > & _grad_phi;
  // test functions
  const std::vector<std::vector<Real> > & _test;
  const std::vector<std::vector<RealGradient> > & _grad_test;
  // unknown
  const VariableValue & _u;
  const VariableGrad & _grad_u;

  virtual Real computeQpResidual() = 0;
  virtual Real computeQpJacobian();

};

template<>
InputParameters validParams<IntegratedBC>();

#endif /* INTEGRATEDBC_H_ */
