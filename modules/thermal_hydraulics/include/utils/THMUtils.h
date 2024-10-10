//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MooseTypes.h"

#include "libmesh/parallel.h"

namespace THM
{

/**
 * Computes two unit vectors orthogonal to the given vector
 *
 * The input vector need not be normalized; it will be normalized within this function.
 *
 * @param[in] n_unnormalized   Vector for which to find orthogonal directions
 * @param[out] t1   First orthogonal unit vector
 * @param[out] t2   Second orthogonal unit vector
 */
void computeOrthogonalDirections(const RealVectorValue & n_unnormalized,
                                 RealVectorValue & t1,
                                 RealVectorValue & t2);

/**
 * Parallel gather of a map of DoF ID to AD vector
 *
 * @param[in] comm  Parallel communicator
 * @param[inout] this_map  Data map
 */
void allGatherADVectorMap(const Parallel::Communicator & comm,
                          std::map<dof_id_type, std::vector<ADReal>> & this_map);

/**
 * Parallel gather of a map of DoF ID to AD vector
 *
 * In contrast to \c allGatherADVectorMap, this function does not assume that
 * each of the maps from the different processors have unique keys; it applies
 * a sum if the key exists on multiple processors.
 *
 * @param[in] comm  Parallel communicator
 * @param[inout] this_map  Data map
 */
void allGatherADVectorMapSum(const Parallel::Communicator & comm,
                             std::map<dof_id_type, std::vector<ADReal>> & this_map);
}
