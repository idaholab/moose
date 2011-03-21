#include "Kernel.h"
#include "Variable.h"
#include "Problem.h"
#include "SubProblem.h"
#include "System.h"

template<>
InputParameters validParams<Kernel>()
{
  InputParameters p = validParams<Object>();
  p.addRequiredParam<std::string>("variable", "The name of the variable that this kernel operates on");
  p.addParam<std::vector<unsigned int> >("block", "The list of ids of the blocks (subdomain) that this kernel will be applied to");
  p.addParam<Real>("start_time", -std::numeric_limits<Real>::max(), "The time that this kernel will be active after.");
  p.addParam<Real>("stop_time", std::numeric_limits<Real>::max(), "The time after which this kernel will no longer be active.");
  return p;
}


Kernel::Kernel(const std::string & name, InputParameters parameters) :
    Object(name, parameters),
    Moose::Coupleable(parameters),
    FunctionInterface(parameters),
    Moose::TransientInterface(parameters),
    Moose::MaterialPropertyInterface(parameters),
    _problem(*parameters.get<Moose::SubProblem *>("_problem")),
    _sys(*parameters.get<Moose::System *>("_sys")),
    _tid(parameters.get<THREAD_ID>("_tid")),
    _var(_sys.getVariable(_tid, parameters.get<std::string>("variable"))),
    _test_var(_sys.getVariable(_tid, parameters.get<std::string>("variable"))),
    _dim(_problem.mesh().dimension()),

    _current_elem(_var.currentElem()),
    _q_point(_problem.points(_tid)),
    _qrule(_problem.qRule(_tid)),
    _JxW(_problem.JxW(_tid)),

    _phi(_var.phi()),
    _grad_phi(_var.gradPhi()),

    _test(_test_var.test()),
    _grad_test(_test_var.gradTest()),

    _u(_var.sln()),
    _u_old(_var.slnOld()),
    _u_older(_var.slnOlder()),
    _grad_u(_var.gradSln()),
    _grad_u_old(_var.gradSlnOld()),
    _grad_u_older(_var.gradSlnOlder()),

    _u_dot(_var.uDot()),
    _du_dot_du(_var.duDotDu()),

    _start_time(parameters.get<Real>("start_time")),
    _stop_time(parameters.get<Real>("stop_time"))
{
}

Kernel::~Kernel()
{
}

Real
Kernel::startTime()
{
  return _start_time;
}

Real
Kernel::stopTime()
{
  return _stop_time;
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
  return Moose::Coupleable::getCoupled(var_name);
}

VariableValue &
Kernel::coupledValue(const std::string & var_name)
{
  return Moose::Coupleable::getCoupledValue(var_name);
}

VariableGradient &
Kernel::coupledGradient(const std::string & var_name)
{
  return Moose::Coupleable::getCoupledGradient(var_name);
}
