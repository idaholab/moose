//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosAuxKernel.h"

namespace Moose
{
namespace Kokkos
{

InputParameters
AuxKernel::validParams()
{
  InputParameters params = AuxKernelBase::validParams();
  params.addPrivateParam<bool>(MooseBase::kokkos_object_param, true);
  params.registerBase("KokkosAuxKernel");
  return params;
}

AuxKernel::AuxKernel(const InputParameters & parameters)
  : ::AuxKernelBase(parameters),
    MeshHolder(*_mesh.getKokkosMesh()),
    AssemblyHolder(_sys.feProblem().kokkosAssembly()),
    SystemHolder(_sys.feProblem().getKokkosSystems()),
    _nodal(_var.isNodal()),
    _test(),
    _u(_var, Moose::SOLUTION_TAG, _var.isNodal()),
    _t(TransientInterface::_t),
    _t_old(TransientInterface::_t_old),
    _t_step(TransientInterface::_t_step),
    _dt(TransientInterface::_dt),
    _dt_old(TransientInterface::_dt_old)
{
  if (!_nodal && _bnd)
    mooseError("Boundary restriction of a Kokkos AuxKernel is only supported for nodal variables.");

  auto tag = _sys.feProblem().addVectorTag("parallel_solution", Moose::VECTOR_TAG_SOLUTION);

  _kokkos_var.init(_var, tag);
}

AuxKernel::AuxKernel(const AuxKernel & object)
  : ::AuxKernelBase(object, {}),
    MeshHolder(object),
    AssemblyHolder(object),
    SystemHolder(object),
    _nodal(object._nodal),
    _kokkos_var(object._kokkos_var),
    _test(object._test),
    _u(object._u),
    _t(object._t),
    _t_old(object._t_old),
    _t_step(object._t_step),
    _dt(object._dt),
    _dt_old(object._dt_old)
{
}

void
AuxKernel::compute()
{
  if (_var.isNodal())
  {
    Policy policy = _bnd ? Policy(0, numKokkosBoundaryNodes()) : Policy(0, numKokkosBlockNodes());

    if (!_node_dispatcher)
      _node_dispatcher = DispatcherRegistry::build<NodeLoop>(this, type());

    _node_dispatcher->parallelFor(policy);
  }
  else
  {
    Policy policy = Policy(0, numKokkosBlockElements());

    if (!_element_dispatcher)
      _element_dispatcher = DispatcherRegistry::build<ElementLoop>(this, type());

    _element_dispatcher->parallelFor(policy);
  }
}

VariableValue
AuxKernel::uOld() const
{
  _var.sys().needSolutionState(1);

  return VariableValue(_var, Moose::OLD_SOLUTION_TAG, isNodal());
}

VariableValue
AuxKernel::uOlder() const
{
  _var.sys().needSolutionState(2);

  return VariableValue(_var, Moose::OLDER_SOLUTION_TAG, isNodal());
}

void
AuxKernel::getKokkosMaterialPropertyHook(const std::string & prop_name_in,
                                         const unsigned int /* state */)
{
  if (isNodal())
    mooseError("Nodal KokkosAuxKernel '",
               name(),
               "' attempted to reference material property '",
               prop_name_in,
               "'\nConsider using an elemental auxiliary variable for '",
               _var.name(),
               "'.");
}

} // namespace Kokkos
} // namespace Moose
