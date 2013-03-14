// The libMesh Finite Element Library.
// Copyright (C) 2002-2012 Benjamin S. Kirk, John W. Peterson, Roy H. Stogner

// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.

// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.

// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA



#ifndef DTKINTERPOLATIONHELPER_H
#define DTKINTERPOLATIONHELPER_H

#include "libmesh/libmesh_config.h"

#ifdef LIBMESH_HAVE_DTK

#include "libmesh/solution_transfer.h"
#include "DTKInterpolationAdapter.h"

// DTK
#include <DTK_SharedDomainMap.hpp>

#include <string>

namespace libMesh {

/**
 * Helper object that uses DTK to interpolate between two libMesh based systems
 */
class DTKInterpolationHelper
{
public:
  DTKInterpolationHelper();
  virtual ~DTKInterpolationHelper();

  /**
   * Do an interpolation with possibly offsetting each of the domains (moving them).
   *
   * @param from A unique identifier for the source system.
   * @param to A unique identifier for the target system.
   * @param from_var The source variable.  Pass NULL if this processor doesn't own any of the source domain.
   * @param to_var The destination variable  Pass NULL if this processor doesn't own any of the destination domain.
   * @param from_offset How much to move the source domain.  This value will get added to each nodal position.
   * @param to_offset How much to move the destination domain.    This value will get added to each nodal position.
   * @param from_mpi_comm The MPI communicator the source domain lives on.  If NULL then this particular processor doesn't contain the source domain.
   * @param to_mpi_comm The MPI communicator the destination domain lives on.  If NULL then this particular processor doesn't contain the destination domain.
   */
  void transferWithOffset(unsigned int from, unsigned int to, const Variable * from_var, const Variable * to_var, const Point & from_offset, const Point & to_offset, MPI_Comm * from_mpi_comm, MPI_Comm * to_mpi_comm);

protected:
  typedef DataTransferKit::SharedDomainMap<DTKInterpolationAdapter::MeshContainerType,DTKInterpolationAdapter::MeshContainerType> shared_domain_map_type;

  /// The DTKAdapter associated with each EquationSystems
  std::map<EquationSystems *, DTKInterpolationAdapter *> adapters;

  /// The dtk shared domain maps for pairs of EquationSystems (from, to)
  std::map<std::pair<unsigned int, unsigned int>, shared_domain_map_type * > dtk_maps;
};

} // namespace libMesh

#endif // #ifdef LIBMESH_HAVE_DTK

#endif // #define DTKINTERPOLATIONHELPER_H
