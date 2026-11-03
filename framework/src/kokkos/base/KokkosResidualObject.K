//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosResidualObject.h"

#include "FEProblemBase.h"

namespace Moose::Kokkos
{

InputParameters
ResidualObject::validParams()
{
  auto params = ::ResidualObject::validParams();

  addConfigurationParams(params);

  params.addPrivateParam<bool>(MooseBase::kokkos_object_param, true);

  return params;
}

ResidualObject::ResidualObject(const InputParameters & parameters,
                               Moose::VarFieldType field_type,
                               bool nodal)
  : ::ResidualObject(parameters, nodal),
    MeshHolder(*_fe_problem.mesh().getKokkosMesh()),
    AssemblyHolder(_fe_problem.kokkosAssembly()),
    FESystemHolder(_fe_problem.getKokkosFESystems()),
    _var(_subproblem.getVariable(_tid,
                                 getParam<NonlinearVariableName>("variable"),
                                 Moose::VarKindType::VAR_SOLVER,
                                 field_type)),
    _dimension(_fe_problem.mesh().dimension()),
    _t(TransientInterface::_t),
    _t_old(TransientInterface::_t_old),
    _t_step(TransientInterface::_t_step),
    _dt(TransientInterface::_dt),
    _dt_old(TransientInterface::_dt_old)
{
  if (_var.isVector() && field_type != Moose::VarFieldType::VAR_FIELD_VECTOR)
    paramError("variable", "Vector variable specified to a scalar Kokkos residual object.");

  _kokkos_var.init(_var);
}

ResidualObject::ResidualObject(const ResidualObject & object)
  : ::ResidualObject(object, {}),
    MeshHolder(object),
    AssemblyHolder(object),
    FESystemHolder(object),
    _var(object._var),
    _kokkos_var(object._kokkos_var),
    _configuration(object._configuration),
    _thread(object._thread),
    _dimension(object._dimension),
    _t(object._t),
    _t_old(object._t_old),
    _t_step(object._t_step),
    _dt(object._dt),
    _dt_old(object._dt_old),
    _displacement_component(object._displacement_component),
    _vector_tags(object._vector_tags),
    _matrix_tags(object._matrix_tags)
{
  // Tags may be added to the governing tag sets held by the TaggingInterface later than the time
  // that the primary constructor is run. So it's more robust to build our tag set, immediately
  // before being dispatched to device, from the governing TaggingInterface set instead of relying
  // on copying the tag set from object's \p _vector_tags and \p _matrix_tags arrays
  _vector_tags = object.getVectorTags({});
  _matrix_tags = object.getMatrixTags({});
}

void
ResidualObject::dispatchDisplacementJacobian(const ThreadID league_size, const bool vector)
{
  if (!_displacement_jacobian_dispatcher)
    _displacement_jacobian_dispatcher =
        DispatcherRegistry::build<DisplacementJacobianLoop>(this, type());

  TeamLaunch launch;
  launch.league_size = league_size;

  const auto max_qps = kokkosAssembly().getMaxQpsPerElem();
  launch.scratch_bytes = vector ? DisplacementScratchLayout<true>::bytes(max_qps)
                                : DisplacementScratchLayout<false>::bytes(max_qps);

  // Each nonlinear displacement component of the residual system that the coupling matrix couples
  // to the residual variable contributes a Jacobian block
  auto & ivar = const_cast<Variable &>(_kokkos_var);
  const auto & problem = ivar.mooseVar()->sys().feProblem();
  const auto residual_sys = _kokkos_var.sys();
  const auto & disp = _configuration.displacements();

  for (const auto c : make_range(disp.components()))
  {
    const auto * moose_var = const_cast<Variable &>(disp).mooseVar(c);

    if (disp.sys(c) != residual_sys || moose_var->kind() != Moose::VarKindType::VAR_SOLVER ||
        !problem.areCoupled(_kokkos_var.var(), disp.var(c), residual_sys))
      continue;

    _displacement_component = c;
    _displacement_jacobian_dispatcher->parallelForTeams(launch);
  }
}

} // namespace Moose::Kokkos
