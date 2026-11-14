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

  addDeformationParams(params);

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
    _deformation(object._deformation),
    _thread(object._thread),
    _dimension(object._dimension),
    _t(object._t),
    _t_old(object._t_old),
    _t_step(object._t_step),
    _dt(object._dt),
    _dt_old(object._dt_old),
    _ad_layout(object._ad_layout),
    _displacement_component(object._displacement_component),
    _displacement_layout(object._displacement_layout),
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

namespace
{
/**
 * Shape the launch of a team operation whose teams hold groups of threads, one group per element or
 * side. A team of the target size holds as many groups as fit, within the scratch limit and the
 * item count. Registers and scratch may cap the team size below the target.
 * @param dispatcher The team dispatcher
 * @param n_items The number of elements or sides
 * @param threads The number of threads that one element or side needs
 * @param align Whether the groups align to half-warps
 * @param bytes The callable giving the scratch bytes of a team of a number of groups
 */
template <typename Bytes>
TeamLaunch
groupedTeamLaunch(const DispatcherBase & dispatcher,
                  const ThreadID n_items,
                  const unsigned int threads,
                  const bool align,
                  Bytes bytes)
{
  const auto scratch_max = TeamPolicy<ResidualObject::ResidualLoop>::scratch_size_max(0);
  const auto max_groups_scratch = std::max<std::size_t>(1, scratch_max / bytes(1));

  TeamLaunch launch;

  auto shape = [&](const unsigned int team_size)
  {
    const auto group_size = TeamGroup::groupSize(threads, align, team_size);
    const auto groups = static_cast<unsigned int>(std::min<std::size_t>(
        {std::max(1u, team_size / group_size), max_groups_scratch, n_items}));

    launch.team_size = group_size * groups;
    launch.league_size = (n_items + groups - 1) / groups;
    launch.scratch_bytes = bytes(groups);
  };

  shape(TeamGroup::target_team_size);

  if (const auto team_size_max = dispatcher.teamSizeMax(launch); launch.team_size > team_size_max)
    shape(team_size_max);

  return launch;
}
} // namespace

void
ResidualObject::dispatchADTeams(DispatcherBase & dispatcher,
                                const ThreadID n_items,
                                const unsigned int max_qps,
                                const unsigned int slots,
                                const ADSlotTable & table)
{
  if (!n_items)
    return;

  const auto scratch_max = TeamPolicy<ResidualLoop>::scratch_size_max(0);

  // The Jacobian contraction planes hold the trial functions of the widest column variable
  auto & layout = _ad_layout;
  layout = ADScratchLayout();
  layout.max_qps = max_qps;
  layout.slots = slots;

  if (slots)
  {
    const auto & assembly = kokkosAssembly();
    const auto n_elem_types = kokkosMesh().getNumLocalElementTypes();

    for (const auto & block : table.hostBlocks())
    {
      const auto jfe = kokkosSystem(block.sys).getFETypeID(block.var);
      for (const auto type : make_range(n_elem_types))
        layout.max_jdofs = std::max(layout.max_jdofs, assembly.getNumDofs(type, jfe));
    }

    // Chunk the quadrature points of the contraction only when one group does not fit
    layout.qp_chunk = max_qps;
    while (layout.qp_chunk > 1 && layout.bytes(1) > scratch_max)
      layout.qp_chunk = (layout.qp_chunk + 1) / 2;
  }

  if (layout.bytes(1) > scratch_max)
    mooseError("The pointwise AD assembly needs ",
               layout.bytes(1),
               " bytes of scratch memory per element or side, more than the limit of ",
               scratch_max,
               " bytes.");

  // The residual quadrature point sums of a group split into slices over the widest test
  // function set
  unsigned int max_idofs = 1;
  {
    const auto & assembly = kokkosAssembly();
    const auto ife = kokkosSystem(_kokkos_var.sys()).getFETypeID(_kokkos_var.var());
    for (const auto type : make_range(kokkosMesh().getNumLocalElementTypes()))
      max_idofs = std::max(max_idofs, assembly.getNumDofs(type, ife));
  }

  const auto launch =
      groupedTeamLaunch(dispatcher,
                        n_items,
                        layout.max_qps,
                        layout.max_jdofs != 0,
                        [&](const unsigned int groups) { return layout.bytes(groups); });
  const auto group_size = ADTeamGroup::groupSize(layout, launch.team_size);
  layout.residual_slices = std::max(1u, group_size / max_idofs);

  dispatcher.parallelForTeams(launch);
}

void
ResidualObject::dispatchDisplacementJacobian(const ThreadID n_items, const unsigned int max_qps)
{
  if (!n_items)
    return;

  if (!_displacement_jacobian_dispatcher)
    _displacement_jacobian_dispatcher =
        DispatcherRegistry::build<DisplacementJacobianLoop>(this, type());

  const auto scratch_max = TeamPolicy<ResidualLoop>::scratch_size_max(0);

  auto & layout = _displacement_layout;
  layout = DisplacementScratchLayout();
  layout.max_qps = max_qps;
  layout.max_idofs = 1;
  {
    const auto & assembly = kokkosAssembly();
    const auto ife = kokkosSystem(_kokkos_var.sys()).getFETypeID(_kokkos_var.var());
    for (const auto type : make_range(kokkosMesh().getNumLocalElementTypes()))
      layout.max_idofs = std::max(layout.max_idofs, assembly.getNumDofs(type, ife));
  }

  // Chunk the quadrature points only when one group does not fit
  layout.qp_chunk = max_qps;
  while (layout.qp_chunk > 1 && layout.bytes(1) > scratch_max)
    layout.qp_chunk = (layout.qp_chunk + 1) / 2;

  if (layout.bytes(1) > scratch_max)
    mooseError("The displacement Jacobian assembly needs ",
               layout.bytes(1),
               " bytes of scratch memory per element or side, more than the limit of ",
               scratch_max,
               " bytes.");

  const auto launch =
      groupedTeamLaunch(*_displacement_jacobian_dispatcher,
                        n_items,
                        max_qps,
                        DisplacementScratchLayout::align_groups,
                        [&](const unsigned int groups) { return layout.bytes(groups); });

  // Each nonlinear displacement component of the residual system that the coupling matrix couples
  // to the residual variable contributes a Jacobian block
  auto & ivar = const_cast<Variable &>(_kokkos_var);
  const auto & problem = ivar.mooseVar()->sys().feProblem();
  const auto residual_sys = _kokkos_var.sys();
  const auto & disp = _deformation.displacements();

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
