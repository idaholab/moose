#include "IntegratedBC.h"
#include "SubProblem.h"
#include "MooseVariable.h"


template<>
InputParameters validParams<IntegratedBC>()
{
  return validParams<BoundaryCondition>();
}


IntegratedBC::IntegratedBC(const std::string & name, InputParameters parameters) :
    BoundaryCondition(name, parameters),
    _test_var(_problem.getVariable(0, parameters.get<std::string>("variable"))),

    _qrule(_problem.qRuleFace(_tid)),
    _q_point(_problem.pointsFace(_tid)),
    _JxW(_problem.JxWFace(_tid)),

    _phi(_var.phiFace()),
    _grad_phi(_var.gradPhiFace()),

    _test(_test_var.testFace()),
    _grad_test(_test_var.gradTestFace()),

    _u(_var.sln()),
    _grad_u(_var.gradSln())
{
}

IntegratedBC::~IntegratedBC()
{
}

void
IntegratedBC::computeResidual()
{
  DenseVector<Number> &re = _var.residualBlock();

  for (_qp = 0; _qp < _qrule->n_points(); _qp++)
    for (_i = 0; _i < _phi.size(); _i++)
    {
      re(_i) += _JxW[_qp]*computeQpResidual();
    }
}

void
IntegratedBC::computeJacobian(int /*i*/, int /*j*/)
{
  DenseMatrix<Number> & ke = _var.jacobianBlock();

  for (_qp = 0; _qp < _qrule->n_points(); _qp++)
  {
    for (_i = 0; _i < _phi.size(); _i++)
      for (_j = 0; _j < _phi.size(); _j++)
        ke(_i, _j) += _JxW[_qp]*computeQpJacobian();
  }
}

void
IntegratedBC::computeJacobianBlock(DenseMatrix<Number> & Ke, unsigned int ivar, unsigned int jvar)
{
//  Moose::perf_log.push("computeJacobianBlock()","IntegratedBC");

  for (_qp=0; _qp<_qrule->n_points(); _qp++)
    for (_i=0; _i<_phi.size(); _i++)
      for (_j=0; _j<_phi.size(); _j++)
      {
        if (ivar == jvar)
          Ke(_i,_j) += _JxW[_qp]*computeQpJacobian();
        else
          Ke(_i,_j) += _JxW[_qp]*computeQpOffDiagJacobian(jvar);
      }

//  Moose::perf_log.pop("computeJacobianBlock()","IntegratedBC");
}

Real
IntegratedBC::computeQpJacobian()
{
  return 0;
}

Real
IntegratedBC::computeQpOffDiagJacobian(unsigned int /*jvar*/)
{
  return 0;
}

unsigned int
IntegratedBC::coupled(const std::string & var_name)
{
  return Coupleable::getCoupled(var_name);
}

VariableValue &
IntegratedBC::coupledValue(const std::string & var_name)
{
  return Coupleable::getCoupledValue(var_name);
}

VariableValue &
IntegratedBC::coupledDot(const std::string & var_name)
{
  return Coupleable::getCoupledDot(var_name);
}

VariableGradient &
IntegratedBC::coupledGradient(const std::string & var_name)
{
  return Coupleable::getCoupledGradient(var_name);
}
