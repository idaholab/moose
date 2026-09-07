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

  return params;
}

PMultigrid::PMultigrid(const InputParameters & parameters)
  : MoosePreconditioner(parameters),
    _level_orders(getParam<std::vector<unsigned int>>("level_orders")),
    _verify_level_operators(getParam<bool>("verify_level_operators"))
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
  // preconditioner is constructed ahead of both
  for (const auto order : _level_orders)
    _levels.push_back(std::make_unique<Moose::Kokkos::PLevelSpace>(_nl, order));
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

    _console << "  p = " << level->order() << ": " << level->system().n_dofs() << " dofs\n";
  }

  _console << "  fine: " << _nl.system().n_dofs() << " dofs\n" << std::endl;
}

#endif
