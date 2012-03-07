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
  params += validParams<SetupInterface>();
  params.addRequiredParam<std::string>("variable", "The name of the variable that this object applies to");
  params.addPrivateParam<bool>("use_displaced_mesh", false);
  // For use on the boundary only
  params.addParam<std::vector<unsigned int> >("boundary", "The list of boundary IDs from the mesh where this AuxBC applies");
  params.addParam<std::vector<unsigned int> >("block", "The list of ids of the blocks (subdomain) that this aux kernel will be applied to");

  params.addPrivateParam<std::string>("built_by_action", "add_aux_kernel");
  return params;
}

AuxKernel::AuxKernel(const std::string & name, InputParameters parameters) :
    MooseObject(name, parameters),
    SetupInterface(parameters),
    Coupleable(parameters, parameters.get<AuxiliarySystem *>("_aux_sys")->getVariable(parameters.get<THREAD_ID>("_tid"), parameters.get<std::string>("variable")).feType().family == LAGRANGE), // horrible
    ScalarCoupleable(parameters),
    FunctionInterface(parameters),
    TransientInterface(parameters),
    MaterialPropertyInterface(parameters),
    PostprocessorInterface(parameters),
    GeometricSearchInterface(parameters),
    _problem(*parameters.get<Problem *>("_problem")),
    _subproblem(*parameters.get<SubProblem *>("_subproblem")),
    _sys(*parameters.get<SystemBase *>("_sys")),
    _nl_sys(*parameters.get<SystemBase *>("_nl_sys")),
    _aux_sys(*parameters.get<AuxiliarySystem *>("_aux_sys")),
    _tid(parameters.get<THREAD_ID>("_tid")),

    _var(_aux_sys.getVariable(_tid, parameters.get<std::string>("variable"))),
    _nodal(_var.feType().family == LAGRANGE),

    _mesh(_subproblem.mesh()),
    _dim(_mesh.dimension()),

    _q_point(_subproblem.points(_tid)),
    _qrule(_subproblem.qRule(_tid)),
    _JxW(_subproblem.JxW(_tid)),
    _coord(_subproblem.coords(_tid)),

    _u(_nodal ? _var.nodalSln() : _var.sln()),
    _u_old(_nodal ? _var.nodalSlnOld() : _var.slnOld()),
    _u_older(_nodal ? _var.nodalSlnOlder() : _var.slnOlder()),

    _current_elem(_var.currentElem()),
    _current_elem_volume(_subproblem.elemVolume(_tid)),

    _current_node(_var.node()),

    _solution(_aux_sys.solution()),

    _real_zero(_problem._real_zero[_tid]),
    _zero(_problem._zero[_tid]),
    _grad_zero(_problem._grad_zero[_tid]),
    _second_zero(_problem._second_zero[_tid])
{
}

void
AuxKernel::compute()
{
  Real value = 0;
  if (isNodal())
  {
    if (_var.isNodalDefined())
    {
      _qp = 0;
      value = computeValue();
      _var.setNodalValue(value);                  // update variable data, which is referenced by other kernels, so the value is up-to-date
    }
  }
  else
  {
    for (_qp=0; _qp<_qrule->n_points(); _qp++)
      value += _JxW[_qp]*_coord[_qp]*computeValue();
    value /= _current_elem_volume;
    _var.setNodalValue(value);                  // update variable data, which is referenced by other kernels, so the value is up-to-date
  }
}

bool
AuxKernel::isNodal()
{
  return _nodal;
}
