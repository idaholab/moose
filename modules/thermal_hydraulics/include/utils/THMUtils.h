//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MooseTypes.h"

#include "libmesh/parallel.h"

class MooseApp;

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

/**
 * Parses the component name out of a flow channel boundary connection string of the form
 * 'component_name:in' or 'component_name:out', used by ActionComponents connecting to a
 * FlowChannel1PhaseAC boundary (e.g. FlowBoundary1PhaseAC's 'input' parameter, or a junction's
 * 'connections' parameter). Errors if the string does not have a recognized 'component_name:end'
 * format.
 */
std::string parseConnectedComponentName(const BoundaryName & connection);

/**
 * Parses the outward normal direction implied by a flow channel boundary connection string of the
 * form 'component_name:in' or 'component_name:out' (see parseConnectedComponentName): -1 at the
 * ':in' end, +1 at the ':out' end. Errors if the end type is not 'in' or 'out'.
 */
Real parseConnectionNormal(const BoundaryName & connection);

/**
 * Whether an implicit time integration scheme is being used by the given app's executioner (see
 * e.g. ADRDG3EqnMaterial/ADNumericalFlux3EqnDGKernel/ADBoundaryFlux3EqnBC's 'implicit' parameter).
 * True if there is no transient executioner (e.g. Steady) or its time scheme is not one of the
 * legacy explicit integrators that need this flag set.
 */
bool implicitTimeIntegrationFlag(MooseApp & app);
}
