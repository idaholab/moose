#include "Moose.h"
#include "ElementPostprocessor.h"
#include "MooseVariable.h"
#include "SubProblem.h"

template<>
InputParameters validParams<ElementPostprocessor>()
{
  InputParameters params = validParams<Postprocessor>();
  params.addRequiredParam<std::string>("variable", "The name of the variable that this kernel operates on");
  params.addParam<unsigned int>("block", Moose::ANY_BLOCK_ID, "block ID where the postprocessor works");
  return params;
}

ElementPostprocessor::ElementPostprocessor(const std::string & name, InputParameters parameters) :
    Postprocessor(name, parameters),
    Coupleable(parameters),
    TransientInterface(parameters),
    MaterialPropertyInterface(parameters),
    _block_id(parameters.get<unsigned int>("block")),
    _var(_problem.getVariable(_tid, parameters.get<std::string>("variable"))),
    _q_point(_subproblem.points(_tid)),
    _qrule(_subproblem.qRule(_tid)),
    _JxW(_subproblem.JxW(_tid)),
    _current_elem(_subproblem.elem(_tid)),
    _u(_var.sln()),
    _u_old(_var.slnOld()),
    _u_older(_var.slnOlder()),
    _grad_u(_var.gradSln()),
    _grad_u_old(_var.gradSlnOld()),
    _grad_u_older(_var.gradSlnOlder())
{
}

Real
ElementPostprocessor::computeIntegral()
{
  Real sum = 0;

  for (_qp=0; _qp<_qrule->n_points(); _qp++)
      sum += _JxW[_qp]*computeQpIntegral();
  return sum;
}

unsigned int
ElementPostprocessor::coupledComponents(const std::string & varname)
{
  return Coupleable::coupledComponents(varname);
}

unsigned int
ElementPostprocessor::coupled(const std::string & var_name, unsigned int comp)
{
  return Coupleable::getCoupled(var_name, comp);
}

VariableValue &
ElementPostprocessor::coupledValue(const std::string & var_name, unsigned int comp)
{
  return Coupleable::getCoupledValue(var_name, comp);
}

VariableValue &
ElementPostprocessor::coupledValueOld(const std::string & var_name, unsigned int comp)
{
  return Coupleable::getCoupledValueOld(var_name, comp);
}

VariableValue &
ElementPostprocessor::coupledValueOlder(const std::string & var_name, unsigned int comp)
{
  return Coupleable::getCoupledValueOlder(var_name, comp);
}

VariableGradient &
ElementPostprocessor::coupledGradient(const std::string & var_name, unsigned int comp)
{
  return Coupleable::getCoupledGradient(var_name, comp);
}

VariableGradient &
ElementPostprocessor::coupledGradientOld(const std::string & var_name, unsigned int comp)
{
  return Coupleable::getCoupledGradientOld(var_name, comp);
}

VariableGradient &
ElementPostprocessor::coupledGradientOlder(const std::string & var_name, unsigned int comp)
{
  return Coupleable::getCoupledGradientOlder(var_name, comp);
}
