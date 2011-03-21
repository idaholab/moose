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
  InputParameters params = validParams<MooseObject>();
  params.addRequiredParam<std::string>("variable", "The name of the variable that this object applies to");
  params.addPrivateParam<bool>("use_displaced_mesh", false);
  // For use on the boundary only
  params.addParam<std::vector<unsigned int> >("boundary", "The list of variable names this Material is coupled to.");
  params.addParam<std::vector<unsigned int> >("block", "The list of ids of the blocks (subdomain) that this aux kernel will be applied to");
  
  return params;
}

AuxKernel::AuxKernel(const std::string & name, InputParameters parameters) :
    MooseObject(name, parameters),
    Coupleable(parameters),
    FunctionInterface(parameters),
    TransientInterface(parameters),
    MaterialPropertyInterface(parameters),
    GeometricSearchInterface(parameters),
    _problem(*parameters.get<SubProblem *>("_problem")),
    _sys(*parameters.get<SystemBase *>("_sys")),
    _aux_sys(*parameters.get<AuxiliarySystem *>("_aux_sys")),
    _tid(parameters.get<THREAD_ID>("_tid")),
    _var(_sys.getVariable(_tid, parameters.get<std::string>("variable"))),
    _mesh(_problem.mesh()),
    _dim(_mesh.dimension()),
    _q_point(_problem.points(_tid)),
    _qrule(_problem.qRule(_tid)),
    _JxW(_problem.JxW(_tid)),

    _u(_var.sln()),
    _u_old(_var.slnOld()),
    _u_older(_var.slnOlder()),

    _current_elem(_var.currentElem()),
    _current_node(_var.node()),
    _current_volume(_aux_sys._data[_tid]._current_volume),
    _nodal(_var.feType().family == LAGRANGE),

    _real_zero(_problem._real_zero[_tid]),
    _zero(_problem._zero[_tid]),
    _grad_zero(_problem._grad_zero[_tid]),
    _second_zero(_problem._second_zero[_tid])
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

VariableGradient &
AuxKernel::coupledGradient(const std::string & var_name)
{
  if (isNodal())
    mooseError("Nodal variables do not have gradients");
  else
    return Coupleable::getCoupledGradient(var_name);
}
