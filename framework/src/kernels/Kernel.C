#include "Kernel.h"
#include "Variable.h"
#include "Problem.h"
#include "SubProblem.h"

template<>
InputParameters validParams<Kernel>()
{
  InputParameters p = validParams<Object>();
  p.addRequiredParam<std::string>("variable", "The name of the variable that this kernel operates on");
  p.addParam<std::vector<unsigned int> >("block", "The list of ids of the blocks (subdomain) that this kernel will be applied to");
  return p;
}


Kernel::Kernel(const std::string & name, InputParameters parameters) :
    Object(name, parameters),
    Coupleable(parameters),
    FunctionInterface(parameters),
    _problem(*parameters.get<Moose::Problem *>("_problem")),
    _sys(*parameters.get<Moose::SubProblem *>("_sys")),
    _tid(parameters.get<THREAD_ID>("_tid")),
    _var(_sys.getVariable(_tid, parameters.get<std::string>("variable"))),
    _test_var(_sys.getVariable(_tid, parameters.get<std::string>("variable"))),

    _current_elem(_var.currentElem()),
    _q_point(_var.points()),
    _qrule(_var.qRule()),
    _JxW(_var.JxW()),

    _t(_problem.time()),

    _phi(_var.phi()),
    _grad_phi(_var.gradPhi()),

    _test(_test_var.phi()),
    _grad_test(_test_var.gradPhi()),

    _u(_var.sln()),
    _u_old(_var.slnOld()),
    _u_older(_var.slnOlder()),
    _grad_u(_var.gradSln()),
    _grad_u_old(_var.gradSlnOld()),
    _grad_u_older(_var.gradSlnOlder()),

    _u_dot(_var.uDot()),
    _du_dot_du(_var.duDotDu())
{
}

Kernel::~Kernel()
{
}

void
Kernel::computeResidual()
{
  DenseVector<Number> & re = _var.residualBlock();

  for (_qp = 0; _qp < _qrule->n_points(); _qp++)
  {
    for (_i = 0; _i < _phi.size(); _i++)
      re(_i) += _JxW[_qp]*computeQpResidual();
  }
}

void
Kernel::computeJacobian(int /*i*/, int /*j*/)
{
  DenseMatrix<Number> & ke = _var.jacobianBlock();

  for (_qp = 0; _qp < _qrule->n_points(); _qp++)
  {
    for (_i = 0; _i < _phi.size(); _i++)
      for (_j = 0; _j < _phi.size(); _j++)
        ke(_i, _j) += _JxW[_qp]*computeQpJacobian();
  }
}

Real
Kernel::computeQpJacobian()
{
  return 0;
}

unsigned int
Kernel::coupled(const std::string & var_name)
{
  return Coupleable::getCoupled(var_name);
}

VariableValue &
Kernel::coupledValue(const std::string & var_name)
{
  return Coupleable::getCoupledValue(var_name);
}
