//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosDofSpace.h"
#include "KokkosVector.h"

namespace Moose::Kokkos
{

/**
 * One level of a p-multigrid hierarchy as the operator action and the operator diagonal see it: the
 * DOF layout the level's vectors are indexed by, the FE type each variable carries on the level,
 * the level's tagged vectors, and the rows the level holds fixed.
 *
 * Every level shares the mesh and the quadrature rule of the fine level, so one level differs from
 * another in the reference basis tables its FE types select and in the DOF layout its vectors
 * follow. Bundling those makes the level a parameter of the loops: the quadrature-point Jacobian
 * cache filled on the fine level is contracted against whichever level is handed to a loop, and the
 * fine level is the level whose FE types, DOF layout and vectors are the solver system's own.
 *
 * A level is copied to device along with the loop functor, so it carries device-resident views and
 * is copy-constructible. It holds no quadrature-point data of its own.
 */
class QpJacobianLevel : public DofSpace
{
public:
  /**
   * Constructor
   * @param dof_space The DOF layout of the level
   * @param fe_types The FE type ID each variable carries on the level, indexed by variable number
   * @param vectors The level's tagged vectors
   * @param constrained_dof Local-plus-ghost mask of the rows the level holds fixed, which may be
   * unallocated when the level constrains no row
   * @param eliminate_constrained_columns Whether a fixed row is left out of the trial space as well
   * as out of the test space, which the level's action may only do when the residual it linearizes
   * carries no dependence on a fixed row either
   */
  QpJacobianLevel(const DofSpace & dof_space,
                  const Array<unsigned int> & fe_types,
                  const Array<Vector> & vectors,
                  const Array<bool> & constrained_dof,
                  const bool eliminate_constrained_columns)
    : DofSpace(dof_space),
      _fe_types(fe_types),
      _vectors(vectors),
      _constrained_dof(constrained_dof),
      _eliminate_constrained_columns(eliminate_constrained_columns)
  {
  }

#ifdef MOOSE_KOKKOS_SCOPE
  /**
   * Get the FE type ID a variable carries on the level, which selects the level's basis tables
   * @param var The variable number
   * @returns The FE type ID
   */
  KOKKOS_FUNCTION unsigned int getFEType(const unsigned int var) const { return _fe_types[var]; }

  /**
   * Get the DOF value of one of the level's tagged vectors
   * @param dof The local DOF index
   * @param tag The vector tag
   * @returns The DOF value
   */
  KOKKOS_FUNCTION Real & getDofValue(const dof_id_type dof, const TagID tag) const
  {
    return _vectors[tag][dof];
  }

  /**
   * Get whether the level holds a row fixed, in which case the operator contributes nothing to it
   * @param dof The local DOF index
   * @returns Whether the row is fixed
   */
  KOKKOS_FUNCTION bool isConstrained(const dof_id_type dof) const
  {
    return _constrained_dof.isAlloc() && _constrained_dof[dof];
  }

  /**
   * Get whether the operator leaves a row out of the trial space it gathers, as well as out of the
   * test space it accumulates into, which is what makes its action symmetric whenever the
   * linearization it contracts is
   * @param dof The local DOF index
   * @returns Whether the column is eliminated
   */
  KOKKOS_FUNCTION bool isEliminatedColumn(const dof_id_type dof) const
  {
    return _eliminate_constrained_columns && isConstrained(dof);
  }
#endif

private:
  /**
   * FE type ID of each variable on the level
   */
  Array<unsigned int> _fe_types;

  /**
   * The level's tagged vectors
   */
  Array<Vector> _vectors;

  /**
   * Local-plus-ghost mask of the rows the level holds fixed
   */
  Array<bool> _constrained_dof;

  /**
   * Whether a fixed row is left out of the trial space as well as out of the test space
   */
  bool _eliminate_constrained_columns;
};

} // namespace Moose::Kokkos
