//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef DTKINTERPOLATIONHELPER_H
#define DTKINTERPOLATIONHELPER_H

#include "libmesh/libmesh_config.h"

#ifdef LIBMESH_TRILINOS_HAVE_DTK

// MOOSE includes
#include "DTKInterpolationAdapter.h"

// DTK includes
#include "libmesh/ignore_warnings.h"
#include <DTK_SharedDomainMap.hpp>
#include "libmesh/restore_warnings.h"

namespace libMesh
{

// Forward declarations
class Variable;

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
   * @param from_var The source variable.  Pass NULL if this processor doesn't own any of the source
   * domain.
   * @param to_var The destination variable  Pass NULL if this processor doesn't own any of the
   * destination domain.
   * @param from_offset How much to move the source domain.  This value will get added to each nodal
   * position.
   * @param to_offset How much to move the destination domain.    This value will get added to each
   * nodal position.
   * @param from_mpi_comm The MPI communicator the source domain lives on.  If NULL then this
   * particular processor doesn't contain the source domain.
   * @param to_mpi_comm The MPI communicator the destination domain lives on.  If NULL then this
   * particular processor doesn't contain the destination domain.
   */
  void transferWithOffset(unsigned int from,
                          unsigned int to,
                          const Variable * from_var,
                          const Variable * to_var,
                          const Point & from_offset,
                          const Point & to_offset,
                          MPI_Comm * from_mpi_comm,
                          MPI_Comm * to_mpi_comm);

protected:
  typedef DataTransferKit::SharedDomainMap<DTKInterpolationAdapter::MeshContainerType,
                                           DTKInterpolationAdapter::MeshContainerType>
      shared_domain_map_type;

  /// The DTKAdapter associated with each EquationSystems
  std::map<EquationSystems *, DTKInterpolationAdapter *> adapters;

  /// The dtk shared domain maps for pairs of EquationSystems (from, to)
  std::map<std::pair<unsigned int, unsigned int>, shared_domain_map_type *> dtk_maps;
};

} // namespace libMesh

#endif // #ifdef LIBMESH_TRILINOS_HAVE_DTK

#endif // #define DTKINTERPOLATIONHELPER_H
