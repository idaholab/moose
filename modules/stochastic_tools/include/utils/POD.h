//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ParallelSolutionStorage.h"
#include "MooseTypes.h"
#include "libmesh/petsc_vector.h"
#include "libmesh/libmesh_config.h"

#include <slepcsvd.h>

namespace StochasticTools
{
/**
 * Class which computes a Proper Orthogonal Decomposition (POD) for snapshots
 * stored in ParallelSolutionStorage.
 */
class POD
{
public:
  POD(const ParallelSolutionStorage * const parallel_storage,
      const std::string & extra_slepc_options,
      const Parallel::Communicator & comm);
  /**
   * @param vname Variable name to extract snapshot data
   * @param left_basis_functions Vector for left basis functions
   * @param right_basis_functions Vector for right basis functions
   * @param singular_values Vector for singular values
   * @param num_modes Max number of modes to compute
   * @param energy Energy threshold to determine the minimum number of modes
   */
  void computePOD(const VariableName & vname,
                  std::vector<DenseVector<Real>> & left_basis_functions,
                  std::vector<DenseVector<Real>> & right_basis_functions,
                  std::vector<Real> & singular_values,
                  const dof_id_type num_modes,
                  const Real energy) const;

private:
  /**
   * Determine the number of basis functions needed for a given variable based on the information
   * on the singular values.
   * @param singular_values Vector of singular values
   * @param num_modes_compute Max number of modes to compute
   * @param energy Energy threshold to determine the minimum number of modes
   * @return dof_id_type Number of modes to extract
   */
  dof_id_type determineNumberOfModes(const std::vector<Real> & singular_values,
                                     const dof_id_type num_modes_compute,
                                     const Real energy) const;

#if !PETSC_VERSION_LESS_THAN(3, 14, 0)
  /// The container where the snapshots are stored
  const ParallelSolutionStorage * const _parallel_storage;
  /// Additional options for the singular value solver
  const std::string & _extra_slepc_options;
  /// The communicator for parallel routines
  const Parallel::Communicator & _communicator;
#endif
};
}
