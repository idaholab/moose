#include "IntegratedBC.h"
#include "SubProblem.h"
#include "Variable.h"


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

    _test(_test_var.phiFace()),
    _grad_test(_test_var.gradPhiFace()),

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
      std::cerr << "qp = " << _qp << " " << _JxW[_qp] << ", " << computeQpResidual() << std::endl;
    }
}

void
IntegratedBC::computeJacobian(int /*i*/, int /*j*/)
{
//  DenseMatrix<Number> & ke = _var.jacobianBlock();
//
//  for (_qp = 0; _qp < _qrule->n_points(); _qp++)
//  {
//    for (_i = 0; _i < _phi.size(); _i++)
//      for (_j = 0; _j < _phi.size(); _j++)
//        ke(_i, _j) += _JxW[_qp]*computeQpJacobian();
//  }
}

Real
IntegratedBC::computeQpJacobian()
{
  return 0;
}

unsigned int
IntegratedBC::coupled(const std::string & var_name)
{
  return Moose::Coupleable::getCoupled(var_name);
}

VariableValue &
IntegratedBC::coupledValue(const std::string & var_name)
{
  return Moose::Coupleable::getCoupledValue(var_name);
}
