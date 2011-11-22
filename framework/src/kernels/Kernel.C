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

#include "Kernel.h"
#include "AsmBlock.h"
#include "MooseVariable.h"
#include "Problem.h"
#include "SubProblem.h"
#include "SystemBase.h"

template<>
InputParameters validParams<Kernel>()
{
  InputParameters params = validParams<MooseObject>();
  params.addRequiredParam<std::string>("variable", "The name of the variable that this kernel operates on");
  params.addParam<std::vector<unsigned int> >("block", "The list of ids of the blocks (subdomain) that this kernel will be applied to");
  params.addParam<Real>("start_time", -std::numeric_limits<Real>::max(), "The time that this kernel will be active after.");
  params.addParam<Real>("stop_time", std::numeric_limits<Real>::max(), "The time after which this kernel will no longer be active.");

  // testing, dude
  params.addPrivateParam<bool>("use_displaced_mesh", false);
  params.addPrivateParam<std::string>("built_by_action", "add_kernel");
  return params;
}


Kernel::Kernel(const std::string & name, InputParameters parameters) :
    MooseObject(name, parameters),
    Coupleable(parameters, false),
    FunctionInterface(parameters),
    TransientInterface(parameters),
    PostprocessorInterface(parameters),
    MaterialPropertyInterface(parameters),
    GeometricSearchInterface(parameters),
    _problem(*parameters.get<Problem *>("_problem")),
    _subproblem(*parameters.get<SubProblem *>("_subproblem")),
    _sys(*parameters.get<SystemBase *>("_sys")),
    _tid(parameters.get<THREAD_ID>("_tid")),
    _asmb(_subproblem.asmBlock(_tid)),
    _var(_sys.getVariable(_tid, parameters.get<std::string>("variable"))),
    _mesh(_subproblem.mesh()),
    _dim(_mesh.dimension()),

    _current_elem(_var.currentElem()),
    _q_point(_subproblem.points(_tid)),
    _qrule(_subproblem.qRule(_tid)),
    _JxW(_subproblem.JxW(_tid)),

    _phi(_asmb.phi()),
    _grad_phi(_asmb.gradPhi()),
    _second_phi(_asmb.secondPhi()),

    _test(_var.phi()),
    _grad_test(_var.gradPhi()),
    _second_test(_var.secondPhi()),

    _u(_var.sln()),
    _u_old(_var.slnOld()),
    _u_older(_var.slnOlder()),
    _grad_u(_var.gradSln()),
    _grad_u_old(_var.gradSlnOld()),
    _grad_u_older(_var.gradSlnOlder()),
    _second_u(_var.secondSln()),
    _second_u_old(_var.secondSlnOld()),
    _second_u_older(_var.secondSlnOlder()),

    _u_dot(_var.uDot()),
    _du_dot_du(_var.duDotDu()),

    _start_time(parameters.get<Real>("start_time")),
    _stop_time(parameters.get<Real>("stop_time")),

    _real_zero(_problem._real_zero[_tid]),
    _zero(_problem._zero[_tid]),
    _grad_zero(_problem._grad_zero[_tid]),
    _second_zero(_problem._second_zero[_tid])
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
  DenseVector<Number> & re = _asmb.residualBlock(_var.number());

  precalculateResidual();
  for (_i = 0; _i < _test.size(); _i++)
    for (_qp = 0; _qp < _qrule->n_points(); _qp++)
    {
      re(_i) += _JxW[_qp]*computeQpResidual();
    }
}

void
Kernel::computeJacobian()
{
  DenseMatrix<Number> & ke = _asmb.jacobianBlock(_var.number(), _var.number());

  for (_i = 0; _i < _test.size(); _i++)
    for (_j = 0; _j < _phi.size(); _j++)
      for (_qp = 0; _qp < _qrule->n_points(); _qp++)
      {
        ke(_i, _j) += _JxW[_qp]*computeQpJacobian();
      }
}

void
Kernel::computeOffDiagJacobian(unsigned int jvar)
{
//  Moose::perf_log.push("computeOffDiagJacobian()",_name);

  DenseMatrix<Number> & ke = _asmb.jacobianBlock(_var.number(), jvar);

  for (_i=0; _i<_test.size(); _i++)
    for (_j=0; _j<_phi.size(); _j++)
      for (_qp=0; _qp<_qrule->n_points(); _qp++)
      {
        if(jvar == _var.number())
          ke(_i,_j) += _JxW[_qp]*computeQpJacobian();
        else
          ke(_i,_j) += _JxW[_qp]*computeQpOffDiagJacobian(jvar);
      }

//  Moose::perf_log.pop("computeOffDiagJacobian()",_name);
}

Real
Kernel::computeQpJacobian()
{
  return 0;
}

Real
Kernel::computeQpOffDiagJacobian(unsigned int /*jvar*/)
{
  return 0;
}


void
Kernel::precalculateResidual()
{
}
