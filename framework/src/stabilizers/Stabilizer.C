#include "Stabilizer.h"
#include "SubProblem.h"
#include "SystemBase.h"
#include "MooseVariable.h"

template<>
InputParameters validParams<Stabilizer>()
{
  InputParameters params = validParams<MooseObject>();
  params.addRequiredParam<std::string>("variable", "The name of the variable this Stabilizer will act on.");
  return params;
}


Stabilizer::Stabilizer(const std::string & name, InputParameters parameters) :
  MooseObject(name, parameters),
  MaterialPropertyInterface(parameters),
  _problem(*parameters.get<SubProblem *>("_problem")),
  _sys(*parameters.get<SystemBase *>("_sys")),
  _tid(parameters.get<THREAD_ID>("_tid")),

  _var(_sys.getVariable(_tid, parameters.get<std::string>("variable"))),
  _current_elem(_var.currentElem()),

  _q_point(_problem.points(_tid)),
  _qrule(_problem.qRule(_tid)),
  _JxW(_problem.JxW(_tid)),

  _phi(_var.phi()),
  _grad_phi(_var.gradPhi()),
  _test(_var.test()),
  _grad_test(_var.gradTest())
{
}

Stabilizer::~Stabilizer()
{
}
