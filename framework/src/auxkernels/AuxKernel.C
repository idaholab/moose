#include "AuxKernel.h"

//local includes
#include "Moose.h"
#include "SubProblem.h"
#include "AuxiliarySystem.h"

//libmesh includes
#include "numeric_vector.h"
#include "dof_map.h"

template<>
InputParameters validParams<AuxKernel>()
{
  InputParameters params = validParams<Object>();
  params.addRequiredParam<std::string>("variable", "The name of the variable that this object applies to");
  // For use on the boundary only
  params.addParam<std::vector<unsigned int> >("boundary", "The list of variable names this Material is coupled to.");
  params.addParam<std::vector<unsigned int> >("block", "The list of ids of the blocks (subdomain) that this aux kernel will be applied to");
  
  return params;
}

AuxKernel::AuxKernel(const std::string & name, InputParameters parameters) :
    Object(name, parameters),
    Coupleable(parameters),
    FunctionInterface(parameters),
    Moose::TransientInterface(parameters),
    Moose::GeometricSearchInterface(parameters),
    _problem(*parameters.get<Moose::SubProblem *>("_problem")),
    _aux_sys(*parameters.get<Moose::AuxiliarySystem *>("_aux_sys")),
    _tid(parameters.get<THREAD_ID>("_tid")),
    _var(_problem.getVariable(_tid, parameters.get<std::string>("variable"))),
    _dim(_problem.mesh().dimension()),
    _qrule(_problem.qRule(_tid)),
    _JxW(_problem.JxW(_tid)),
    _current_elem(_var.currentElem()),
    _current_node(_var.node()),
    _current_volume(_aux_sys._data[_tid]._current_volume),
    _nodal(_var.feType().family == LAGRANGE)
{
}

void
AuxKernel::compute(NumericVector<Number> & sln)
{
  if (isNodal())
  {
    unsigned int & dof_idx = _var.nodalDofIndex();
    _qp = 0;
    sln.set(dof_idx, computeValue());
  }
  else
  {
    Real value = 0.;
    for (_qp=0; _qp<_qrule->n_points(); _qp++)
      value += _JxW[_qp]*computeValue();

    unsigned int & dof_idx = _var.nodalDofIndex();
    sln.set(dof_idx, value / _current_volume);
  }
}

bool
AuxKernel::isNodal()
{
  return _nodal;
}

unsigned int
AuxKernel::coupled(const std::string & var_name)
{
  return Coupleable::getCoupled(var_name);
}

VariableValue &
AuxKernel::coupledValue(const std::string & var_name)
{
  if (isNodal())
    return Coupleable::getCoupledNodalValue(var_name);
  else
    return Coupleable::getCoupledValue(var_name);
}
