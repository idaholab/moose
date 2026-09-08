//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosAssembly.h"

namespace Moose::Kokkos
{

#ifdef MOOSE_KOKKOS_SCOPE

/**
 * The reference-element basis evaluations one level of a p-multigrid hierarchy contracts the
 * quadrature-point Jacobian cache against.
 *
 * A level is identified by the FE type its variables carry, and every level shares the fine
 * quadrature rule, so the values and reference gradients a level needs are the tables the assembly
 * already caches per (subdomain, element type, FE type, edge and face orientation). This class is
 * the operation interface over one such pair of tables: a consumer interpolates a level DOF vector
 * to a quadrature point and accumulates a quadrature-point value/flux pair back onto the level's
 * test functions without indexing the tables itself, which is what makes the level a loop
 * parameter.
 *
 * Operations are expressed one quadrature point at a time, which is what a dense contraction wants.
 * A sum-factorized specialization for tensor-product element types works on whole-element
 * quadrature-point buffers and would extend this interface accordingly.
 */
class LevelBasisTable
{
public:
  /**
   * Constructor
   * @param assembly The Kokkos assembly holding the cached reference tables
   * @param subdomain The contiguous subdomain ID, which selects the quadrature rule
   * @param elem_type The element type ID
   * @param fe_type The FE type ID of the level
   * @param orientation The contiguous edge and face orientation ID
   */
  KOKKOS_FUNCTION LevelBasisTable(const Assembly & assembly,
                                  const ContiguousSubdomainID subdomain,
                                  const unsigned int elem_type,
                                  const unsigned int fe_type,
                                  const unsigned int orientation)
    : _phi(assembly.getPhi(subdomain, elem_type, fe_type, orientation)),
      _grad_phi(assembly.getGradPhi(subdomain, elem_type, fe_type, orientation)),
      _n_dofs(assembly.getNumDofs(elem_type, fe_type))
  {
  }

  /**
   * Get the number of element DOFs the level carries on this element type
   * @returns The number of DOFs
   */
  KOKKOS_FUNCTION unsigned int numDofs() const { return _n_dofs; }

  /**
   * Interpolate a level DOF vector to a quadrature point
   * @param x A callable taking an element-local DOF index and returning that DOF's value
   * @param qp The element-local quadrature point index
   * @param value The interpolated value
   * @param grad The interpolated reference gradient
   */
  template <typename Gather>
  KOKKOS_INLINE_FUNCTION void
  applyB(const Gather & x, const unsigned int qp, Real & value, Real3 & grad) const
  {
    value = 0;
    grad = 0;

    for (unsigned int j = 0; j < _n_dofs; ++j)
    {
      const auto x_j = x(j);

      value += x_j * _phi(j, qp);
      grad += x_j * _grad_phi(j, qp);
    }
  }

  /**
   * Accumulate a quadrature point's value/flux pair onto a contiguous batch of the level's test
   * functions
   * @param qp The element-local quadrature point index
   * @param f The value contracted against the test function values
   * @param flux The flux contracted against the test function reference gradients
   * @param begin The first element-local test function index of the batch
   * @param end One past the last element-local test function index of the batch
   * @param y The accumulator, indexed from the first index of the batch
   */
  KOKKOS_INLINE_FUNCTION void applyBT(const unsigned int qp,
                                      const Real f,
                                      const Real3 & flux,
                                      const unsigned int begin,
                                      const unsigned int end,
                                      Real * const y) const
  {
    for (unsigned int i = begin; i < end; ++i)
      y[i - begin] += _phi(i, qp) * f + _grad_phi(i, qp) * flux;
  }

  /**
   * Get one basis function's value and reference gradient at a quadrature point
   * @param i The element-local DOF index
   * @param qp The element-local quadrature point index
   * @param value The value
   * @param grad The reference gradient
   */
  KOKKOS_INLINE_FUNCTION void
  basis(const unsigned int i, const unsigned int qp, Real & value, Real3 & grad) const
  {
    value = _phi(i, qp);
    grad = _grad_phi(i, qp);
  }

private:
  /// Reference shape function values, indexed by (DOF, quadrature point)
  const ShapeTable<Real> _phi;
  /// Reference shape function gradients, indexed by (DOF, quadrature point)
  const ShapeTable<Real3> _grad_phi;
  /// The number of element DOFs the level carries on this element type
  const unsigned int _n_dofs;
};

#endif

} // namespace Moose::Kokkos
