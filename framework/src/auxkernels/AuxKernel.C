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
  params.addParam<bool>("ts", false, "Set to one to execute only at the end of the time step");
  // For use on the boundary only
  params.addParam<std::vector<unsigned int> >("boundary", "The list of variable names this Material is coupled to.");
  params.addParam<std::vector<unsigned int> >("block", "The list of ids of the blocks (subdomain) that this aux kernel will be applied to");

//  params.addPrivateParam<std::string>("built_by_action", "add_aux_kernel");
  return params;
}

AuxKernel::AuxKernel(const std::string & name, InputParameters parameters) :
    MooseObject(name, parameters),
    Coupleable(parameters, parameters.get<AuxiliarySystem *>("_aux_sys")->getVariable(parameters.get<THREAD_ID>("_tid"), parameters.get<std::string>("variable")).feType().family == LAGRANGE), // horrible
    FunctionInterface(parameters),
    TransientInterface(parameters),
    MaterialPropertyInterface(parameters),
    PostprocessorInterface(parameters),
    GeometricSearchInterface(parameters),
    _problem(*parameters.get<Problem *>("_problem")),
    _subproblem(*parameters.get<SubProblemInterface *>("_subproblem")),
    _sys(*parameters.get<SystemBase *>("_sys")),
    _nl_sys(*parameters.get<SystemBase *>("_nl_sys")),
    _aux_sys(*parameters.get<AuxiliarySystem *>("_aux_sys")),
    _tid(parameters.get<THREAD_ID>("_tid")),

    _var(_aux_sys.getVariable(_tid, parameters.get<std::string>("variable"))),
    _mesh(_subproblem.mesh()),
    _dim(_mesh.dimension()),

    _q_point(_subproblem.points(_tid)),
    _qrule(_subproblem.qRule(_tid)),
    _JxW(_subproblem.JxW(_tid)),

    _u(_var.sln()),
    _u_old(_var.slnOld()),
    _u_older(_var.slnOlder()),

    _current_elem(_var.currentElem()),
    _current_node(_var.node()),
    _current_volume(_aux_sys._data[_tid]._current_volume),
    _nodal(_var.feType().family == LAGRANGE),

    _solution(_sys.solution()),

    _ts(getParam<bool>("ts")),

    _real_zero(_problem._real_zero[_tid]),
    _zero(_problem._zero[_tid]),
    _grad_zero(_problem._grad_zero[_tid]),
    _second_zero(_problem._second_zero[_tid])
{
}

void
AuxKernel::compute()
{
  if (isNodal())
  {
    unsigned int & dof_idx = _var.nodalDofIndex();
    _qp = 0;
    _solution.set(dof_idx, computeValue());
  }
  else
  {
    Real value = 0.;
    for (_qp=0; _qp<_qrule->n_points(); _qp++)
      value += _JxW[_qp]*computeValue();

    unsigned int & dof_idx = _var.nodalDofIndex();
    _solution.set(dof_idx, value / _current_volume);
  }
}

bool
AuxKernel::isNodal()
{
  return _nodal;
}
