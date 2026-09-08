//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PMultigrid.h"

#ifdef MOOSE_KOKKOS_ENABLED

#include "FEProblemBase.h"
#include "NonlinearSystemBase.h"

registerMooseObjectAliased("MooseApp", PMultigrid, "PMG");

InputParameters
PMultigrid::validParams()
{
  InputParameters params = MoosePreconditioner::validParams();

  params.addClassDescription(
      "p-multigrid preconditioner for the Kokkos matrix-free partial-assembly Jacobian. Each "
      "coarse level contracts the fine level's quadrature-point Jacobian cache against a "
      "reduced-order basis on the same mesh and the same quadrature rule.");

  params.addRequiredParam<std::vector<unsigned int>>(
      "level_orders",
      "The polynomial order of each coarse level, ascending. The fine level is the solver system's "
      "own order and is not listed.");

  params.addParam<bool>(
      "verify_level_operators",
      false,
      "Whether to check each level's operator against its diagonal after every linearization, by "
      "applying the operator to a unit vector per degree of freedom. This costs one operator "
      "application per degree of freedom of every level, so it is a verification aid for small "
      "inputs.");

  params.addParam<bool>(
      "verify_level_transfers",
      false,
      "Whether to check that each level transfer restricts by the transpose of its prolongation, "
      "which is what makes the operator the hierarchy realizes on a coarse level the Galerkin "
      "operator of the fine linearization. The check costs one application of each direction of "
      "each transfer, at initial setup only.");

  params.addParam<bool>(
      "verify_level_galerkin",
      false,
      "Whether to check each level's operator against P^T A P after every linearization, where A "
      "is the operator of the next finer level and P is the transfer between the two. At the "
      "finest pair A is the matrix-free Jacobian of the solver system, so the check chains down "
      "the hierarchy and establishes that every level is consistent with the fine linearization. "
      "It costs one application of the next finer level's operator and one of each direction of "
      "the transfer, per level per linearization.");

  params.addParam<bool>(
      "verify_level_matrices",
      false,
      "Whether to check the assembled operator of each level that assembles one against the "
      "operator the level applies without a matrix, after every linearization. The check costs one "
      "application of each of the two, per assembled level per linearization.");

  return params;
}

PMultigrid::PMultigrid(const InputParameters & parameters)
  : MoosePreconditioner(parameters),
    _level_orders(getParam<std::vector<unsigned int>>("level_orders")),
    _verify_level_operators(getParam<bool>("verify_level_operators")),
    _verify_level_transfers(getParam<bool>("verify_level_transfers")),
    _verify_level_galerkin(getParam<bool>("verify_level_galerkin")),
    _verify_level_matrices(getParam<bool>("verify_level_matrices"))
{
  if (_level_orders.empty())
    paramError("level_orders", "At least one coarse level is required.");

  for (const auto i : index_range(_level_orders))
    if (i && _level_orders[i] <= _level_orders[i - 1])
      paramError("level_orders", "The level orders must be strictly ascending.");

  if (!_fe_problem.solverParams(_nl_sys_num)._kokkos_matrix_free)
    mooseError(
        "The p-multigrid preconditioner's operators are the quadrature-point Jacobian cache "
        "contracted against each level's basis, so the solver system must be run in Kokkos "
        "matrix-free mode. Set 'use_kokkos_matrix_free_jacobian = true' in the Executioner.");

  // The levels' systems have to exist before the equation systems are initialized, and their FE
  // types have to be registered before the Kokkos assembly caches reference shape data; the
  // preconditioner is constructed ahead of both. The coarsest level assembles its operator, which
  // is what a coarse solver of the cycle is applied to; the orders ascend, so that is the first.
  for (const auto i : index_range(_level_orders))
    _levels.push_back(std::make_unique<Moose::Kokkos::PLevelSpace>(_nl, _level_orders[i], !i));
}

void
PMultigrid::initialSetup()
{
  MoosePreconditioner::initialSetup();

  _console << "\np-multigrid levels for system '" << _nl.name() << "':\n";

  for (const auto & level : _levels)
  {
    // The levels' DOFs are distributed by now, so each level can build the device DOF layout its
    // vectors and its gather/scatter are indexed by
    level->init();

    _console << "  p = " << level->order() << ": " << level->system().n_dofs() << " dofs, "
             << level->numConstrainedDofs() << " constrained by nodal boundary conditions\n";
  }

  _console << "  fine: " << _nl.system().n_dofs() << " dofs\n" << std::endl;

  // A transfer indexes the DOF layout of both of its sides, so the transfers are built once every
  // level has one
  for (const auto i : index_range(_levels))
    _levels[i]->initTransfer(i + 1 < _levels.size() ? _levels[i + 1].get() : nullptr);

  if (_verify_level_transfers)
  {
    _console << "p-multigrid level transfers for system '" << _nl.name() << "':\n";

    for (const auto & level : _levels)
      level->verifyTransfer(_console);

    _console << std::endl;
  }
}

#endif
