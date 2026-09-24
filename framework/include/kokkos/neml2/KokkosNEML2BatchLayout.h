//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosArray.h"
#include "KokkosAssembly.h"
#include "KokkosDatum.h"
#include "KokkosMesh.h"

namespace Moose::Kokkos
{

/**
 * Assignment of Kokkos quadrature points to NEML2 batch entries.
 *
 * A NEML2 model is evaluated pointwise along its batch dimension, so which quadrature point occupies
 * which batch entry is a free choice, constrained only by every object gathering an input and every
 * object retrieving an output agreeing on it. This class fixes that choice to the numbering Kokkos
 * material property storage already uses: contiguous subdomain ID first, then the assembly's
 * elemental quadrature point offset within the subdomain. A property stored with
 * PropertyConstantOption::NONE over a single subdomain consequently has the same layout as the
 * batch, and the mapping is the identity.
 *
 * The numbering covers only the subdomains passed to build(). An entry for any other subdomain is
 * invalid, so reading one is an out-of-bounds access rather than a silent alias of another
 * subdomain's data.
 */
class NEML2BatchLayout
{
public:
  /**
   * Compute the batch offset of each subdomain
   * @param assembly The Kokkos assembly holding the quadrature point counts
   * @param mesh The Kokkos mesh, for the contiguous subdomain IDs
   * @param blocks The subdomains the NEML2 model covers
   */
  void build(const Assembly & assembly, const Mesh & mesh, const std::set<SubdomainID> & blocks);

  /// Get whether the offsets have been computed
  bool built() const { return _built; }

  /// Get the total number of batch entries over the covered subdomains
  dof_id_type size() const { return _size; }

  /**
   * Get the NEML2 batch index of a quadrature point
   * @param datum The current datum
   * @param qp The local quadrature point index
   * @returns The batch index
   */
  KOKKOS_FUNCTION dof_id_type index(const Datum & datum, const unsigned int qp) const
  {
    return _subdomain_offset[datum.subdomain()] + datum.qpOffset() + qp;
  }

private:
  /// Batch index of the first quadrature point of each contiguous subdomain
  Array<dof_id_type> _subdomain_offset;
  /// Total number of batch entries
  dof_id_type _size = 0;
  /// Whether the offsets have been computed for the current mesh
  bool _built = false;
};

} // namespace Moose::Kokkos
