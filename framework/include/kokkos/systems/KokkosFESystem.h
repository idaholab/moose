//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosSystem.h"
#include "KokkosAssembly.h"

class MooseMesh;
class SystemBase;

namespace Moose::Kokkos
{

class NodalBCBase;

/**
 * The Kokkos FE system class. Each system in MOOSE with FE variables has a corresponding Kokkos FE
 * system.
 */
class FESystem : public System, public AssemblyHolder
{
public:
  /**
   * Constructor for standalone use (pure FE simulations)
   * @param system The associated MOOSE system
   */
  FESystem(SystemBase & system);

  /**
   * Constructor for mixed FE+FV simulations, sharing device memory with an existing System
   * @param base The existing Kokkos System whose device memory to share
   * @param system The associated MOOSE system
   */
  FESystem(System & base, SystemBase & system);

#ifdef MOOSE_KOKKOS_SCOPE
  ///@}
  /**
   * Allocate the quadrature point vectors for active variable and tags and cache
   * quadrature point values
   */
  void reinit();

  /**
   * Get the list of off-diagonal coupled variable numbers of a variable
   * @param var The variable number
   * @returns The list of off-diagonal coupled variable numbers
   */
  KOKKOS_FUNCTION const auto & getCoupling(unsigned int var) const { return _coupling[var]; }

  /**
   * Check whether a local DOF index is associated with a nodal BC for an extra matrix tag
   * @param dof The local DOF index
   * @param tag The extra matrix tag
   * @returns Whether the local DOF index is covered by a nodal BC
   */
  KOKKOS_FUNCTION bool hasNodalBCMatrixTag(dof_id_type dof, TagID tag) const
  {
    return _nbc_matrix_tag_dof[tag].isAlloc() && _nbc_matrix_tag_dof[tag][dof];
  }

  /**
   * Get the FE type ID of a variable
   * @param var The variable number
   * @returns The FE type ID
   */
  KOKKOS_FUNCTION unsigned int getFETypeID(unsigned int var) const { return _var_fe_types[var]; }

  /**
   * Get the local DOF index of a variable for a node
   * @param node The contiguous node ID
   * @param i The node-local DOF index
   * @param var The variable number
   * @returns The local DOF index
   */
  KOKKOS_FUNCTION dof_id_type getNodeLocalDofIndex(ContiguousNodeID node,
                                                   unsigned int i,
                                                   unsigned int var) const
  {
    return _local_node_dof_index[var][node] + i;
  }

  /**
   * Get the global DOF index of a variable for a node
   * @param node The contiguous node ID
   * @param i The node-local DOF index
   * @param var The variable number
   * @returns The global DOF index
   */
  KOKKOS_FUNCTION dof_id_type getNodeGlobalDofIndex(ContiguousNodeID node,
                                                    unsigned int i,
                                                    unsigned int var) const
  {
    return _local_to_global_dof_index[getNodeLocalDofIndex(node, i, var)];
  }

  /**
   * Get whether a variable is defined on a node
   * @param node The contiguous node ID
   * @param var The variable number
   * @returns Whether the variable is defined on the node
   */
  KOKKOS_FUNCTION bool isNodalDefined(ContiguousNodeID node, unsigned int var) const
  {
    return _local_node_dof_index[var][node] != libMesh::DofObject::invalid_id;
  }

  /**
   * Get the DOF value of a tagged vector for automatic differentiation (AD)
   * @param dof The local DOF index
   * @param tag The vector tag
   * @param seed The derivative seed
   * @returns The DOF AD value with optional seed derivative
   */
  KOKKOS_FUNCTION ADReal getVectorDofADValue(const dof_id_type dof,
                                             const TagID tag,
                                             const Real seed) const;
  /**
   * Get the quadrature point value of a variable from a tagged vector
   * @param info The element information object
   * @param qp The subdomain-local flattened quadrature point index
   * @param var The variable number
   * @param tag The vector tag
   * @returns The quadrature value
   */
  KOKKOS_FUNCTION Real & getVectorQpValue(const ElementInfo info,
                                          const dof_id_type qp,
                                          const unsigned int var,
                                          const TagID tag) const
  {
    return _qp_solutions[tag](info.subdomain, var)[qp];
  }
  /**
   * Get the quadrature point value of a variable from a tagged vector for automatic differentiation
   * (AD)
   * @param info The element information object
   * @param offset The element's offset into the subdomain-local flattened quadrature point index
   * @param qp The local quadrature point index
   * @param var The variable number
   * @param tag The vector tag
   * @param seed The derivative seed
   * @returns The quadrature AD value
   */
  KOKKOS_FUNCTION ADReal getVectorQpADValue(const ElementInfo info,
                                            const dof_id_type offset,
                                            const dof_id_type qp,
                                            const unsigned int var,
                                            const TagID tag,
                                            const Real seed) const;
  /**
   * Get the quadrature point gradient of a variable from a tagged vector
   * @param info The element information object
   * @param qp The subdomain-local flattened quadrature point index
   * @param var The variable number
   * @param tag The vector tag
   * @returns The quadrature gradient
   */
  KOKKOS_FUNCTION Real3 & getVectorQpGrad(const ElementInfo info,
                                          const dof_id_type qp,
                                          const unsigned int var,
                                          const TagID tag) const
  {
    return _qp_solutions_grad[tag](info.subdomain, var)[qp];
  }
  /**
   * Get the quadrature point value of a vector variable from a tagged vector
   * @param info The element information object
   * @param qp The subdomain-local flattened quadrature point index
   * @param var The variable number
   * @param tag The vector tag
   * @returns The quadrature vector value
   */
  KOKKOS_FUNCTION Real3 & getVectorQpVectorValue(const ElementInfo info,
                                                 const dof_id_type qp,
                                                 const unsigned int var,
                                                 const TagID tag) const
  {
    return _qp_vector_solutions[tag](info.subdomain, var)[qp];
  }
  /**
   * Get the quadrature point gradient of a vector variable from a tagged vector
   * @param info The element information object
   * @param qp The subdomain-local flattened quadrature point index
   * @param var The variable number
   * @param tag The vector tag
   * @returns The quadrature vector gradient
   */
  KOKKOS_FUNCTION Real33 & getVectorQpVectorGrad(const ElementInfo info,
                                                 const dof_id_type qp,
                                                 const unsigned int var,
                                                 const TagID tag) const
  {
    return _qp_vector_solutions_grad[tag](info.subdomain, var)[qp];
  }
  /**
   * Get the quadrature point curl of a vector variable from a tagged vector
   * @param info The element information object
   * @param qp The subdomain-local flattened quadrature point index
   * @param var The variable number
   * @param tag The vector tag
   * @returns The quadrature vector curl
   */
  KOKKOS_FUNCTION Real3 & getVectorQpVectorCurl(const ElementInfo info,
                                                const dof_id_type qp,
                                                const unsigned int var,
                                                const TagID tag) const
  {
    return _qp_vector_solutions_curl[tag](info.subdomain, var)[qp];
  }
  /**
   * Get the quadrature point gradient of a variable from a tagged vector for automatic
   * differentiation (AD)
   * @param info The element information object
   * @param jacobian The inverse Jacobian matrix
   * @param offset The element's offset into the subdomain-local flattened quadrature point index
   * @param qp The local quadrature point index
   * @param var The variable number
   * @param tag The vector tag
   * @param seed The derivative seed
   * @returns The quadrature AD gradient
   */
  KOKKOS_FUNCTION ADReal3 getVectorQpADGrad(const ElementInfo info,
                                            const Real33 jacobian,
                                            const dof_id_type offset,
                                            const dof_id_type qp,
                                            const unsigned int var,
                                            const TagID tag,
                                            const Real seed) const;
  /**
   * Get the face quadrature point value of a variable from a tagged vector
   * @param info The element information object
   * @param side The side index
   * @param qp The local quadrature point index
   * @param var The variable number
   * @param tag The vector tag
   * @returns The face quadrature value
   */
  KOKKOS_FUNCTION Real getVectorQpValueFace(const ElementInfo info,
                                            const unsigned int side,
                                            const unsigned int qp,
                                            const unsigned int var,
                                            const TagID tag) const;
  /**
   * Get the face quadrature point value of a vector variable from a tagged vector
   * @param info The element information object
   * @param side The side index
   * @param qp The local quadrature point index
   * @param var The variable number
   * @param tag The vector tag
   * @returns The face quadrature vector value
   */
  KOKKOS_FUNCTION Real3 getVectorQpVectorValueFace(const ElementInfo info,
                                                   const unsigned int side,
                                                   const unsigned int qp,
                                                   const unsigned int var,
                                                   const TagID tag) const;
  /**
   * Get the face quadrature point value of a variable from a tagged vector for automatic
   * differentiation (AD)
   * @param info The element information object
   * @param side The side index
   * @param qp The local quadrature point index
   * @param var The variable number
   * @param tag The vector tag
   * @param seed The derivative seed
   * @returns The face quadrature AD value
   */
  KOKKOS_FUNCTION ADReal getVectorQpADValueFace(const ElementInfo info,
                                                const unsigned int side,
                                                const unsigned int qp,
                                                const unsigned int var,
                                                const TagID tag,
                                                const Real seed) const;
  /**
   * Get the face quadrature point gradient of a variable from a tagged vector
   * @param info The element information object
   * @param side The side index
   * @param jacobian The inverse Jacobian matrix
   * @param qp The local quadrature point index
   * @param var The variable number
   * @param tag The vector tag
   * @returns The face quadrature gradient
   */
  KOKKOS_FUNCTION Real3 getVectorQpGradFace(const ElementInfo info,
                                            const unsigned int side,
                                            const Real33 jacobian,
                                            const unsigned int qp,
                                            const unsigned int var,
                                            const TagID tag) const;
  /**
   * Get the face quadrature point gradient of a vector variable from a tagged vector
   * @param info The element information object
   * @param side The side index
   * @param jacobian The inverse Jacobian matrix
   * @param qp The local quadrature point index
   * @param var The variable number
   * @param tag The vector tag
   * @returns The face quadrature vector gradient
   */
  KOKKOS_FUNCTION Real33 getVectorQpVectorGradFace(const ElementInfo info,
                                                   const unsigned int side,
                                                   const Real33 jacobian,
                                                   const unsigned int qp,
                                                   const unsigned int var,
                                                   const TagID tag) const;

  /**
   * Get the face quadrature point gradient of a variable from a tagged vector for automatic
   * differentiation (AD)
   * @param info The element information object
   * @param side The side index
   * @param jacobian The inverse Jacobian matrix
   * @param qp The local quadrature point index
   * @param var The variable number
   * @param tag The vector tag
   * @param seed The derivative seed
   * @returns The face quadrature AD gradient
   */
  KOKKOS_FUNCTION ADReal3 getVectorQpADGradFace(const ElementInfo info,
                                                const unsigned int side,
                                                const Real33 jacobian,
                                                const unsigned int qp,
                                                const unsigned int var,
                                                const TagID tag,
                                                const Real seed) const;

  /**
   * Kokkos function for caching variable values on element quadrature points
   */
  KOKKOS_FUNCTION void operator()(const ThreadID tid) const;
#endif

private:
  /**
   * Setup variable data
   */
  void setupVariables();

  /**
   * Setup DOF data
   */
  void setupDofs();

  /**
   * Setup coupling data between variables
   */
  void setupCoupling();

  /**
   * Mark the DOFs covered by nodal BCs
   */
  void setupNodalBCDofs();

  /**
   * Get the list of DOFs covered by a nodal BC
   * @param nbc The Kokkos nodal BC object
   * @param dofs Local-plus-ghost DOF mask; entries are set true for DOFs covered by the nodal BC
   */
  void getNodalBCDofs(const NodalBCBase * nbc, Array<bool> & dofs);

  /**
   * Kokkos thread object
   */
  Thread<> _thread;

  /**
   * Cached elemental quadrature values and gradients
   */
  ///@{
  Array<Array2D<Array<Real>>> _qp_solutions;
  Array<Array2D<Array<Real3>>> _qp_solutions_grad;
  Array<Array2D<Array<Real3>>> _qp_vector_solutions;
  Array<Array2D<Array<Real33>>> _qp_vector_solutions_grad;
  Array<Array2D<Array<Real3>>> _qp_vector_solutions_curl;
  ///@}

  /**
   * Local nodal DOF indices of each variable
   */
  Array<Array<dof_id_type>> _local_node_dof_index;

  /**
   * FE type ID of each variable
   */
  Array<unsigned int> _var_fe_types;

  /**
   * Whether each variable is vector-valued
   */
  Array<bool> _var_is_vector;

  /**
   * Off-diagonal coupled variable numbers of each variable
   */
  Array<Array<unsigned int>> _coupling;

  /**
   * Per-matrix-tag local-plus-ghost DOF masks for nodal BC coverage
   */
  Array<Array<bool>> _nbc_matrix_tag_dof;
};

#ifdef MOOSE_KOKKOS_SCOPE

KOKKOS_FUNCTION inline ADReal
FESystem::getVectorDofADValue(const dof_id_type dof, TagID tag, const Real seed) const
{
  ADReal value = _vectors[tag][dof];

  if (seed != 0)
    value.derivatives().insert(_local_to_global_dof_index[dof]) = seed;

  return value;
}

KOKKOS_FUNCTION inline ADReal
FESystem::getVectorQpADValue(const ElementInfo info,
                             const dof_id_type offset,
                             const dof_id_type qp,
                             const unsigned int var,
                             const TagID tag,
                             const Real seed) const
{
  ADReal value = 0;

  if (seed == 0)
    value = getVectorQpValue(info, offset + qp, var, tag);
  else
  {
    auto fe = _var_fe_types[var];
    auto n_dofs = kokkosAssembly().getNumDofs(info.type, fe);
    auto & phi = kokkosAssembly().getPhi(info.subdomain, info.type, fe);

    for (unsigned int i = 0; i < n_dofs; ++i)
      value += getVectorDofADValue(getElemLocalDofIndex(info.id, i, var), tag, seed) * phi(i, qp);
  }

  return value;
}

KOKKOS_FUNCTION inline ADReal3
FESystem::getVectorQpADGrad(const ElementInfo info,
                            const Real33 jacobian,
                            const dof_id_type offset,
                            const dof_id_type qp,
                            const unsigned int var,
                            const TagID tag,
                            const Real seed) const
{
  ADReal3 grad;

  if (seed == 0)
    grad = getVectorQpGrad(info, offset + qp, var, tag);
  else
  {
    auto fe = _var_fe_types[var];
    auto n_dofs = kokkosAssembly().getNumDofs(info.type, fe);
    auto & grad_phi = kokkosAssembly().getGradPhi(info.subdomain, info.type, fe);

    for (unsigned int i = 0; i < n_dofs; ++i)
      grad += getVectorDofADValue(getElemLocalDofIndex(info.id, i, var), tag, seed) *
              (jacobian * grad_phi(i, qp));
  }

  return grad;
}

KOKKOS_FUNCTION inline Real
FESystem::getVectorQpValueFace(const ElementInfo info,
                               const unsigned int side,
                               const unsigned int qp,
                               const unsigned int var,
                               const TagID tag) const
{
  auto fe = _var_fe_types[var];
  auto n_dofs = kokkosAssembly().getNumDofs(info.type, fe);
  auto & phi = kokkosAssembly().getPhiFace(info.subdomain, info.type, fe)(side);

  Real value = 0;

  for (unsigned int i = 0; i < n_dofs; ++i)
    value += getVectorDofValue(getElemLocalDofIndex(info.id, i, var), tag) * phi(i, qp);

  return value;
}

KOKKOS_FUNCTION inline Real3
FESystem::getVectorQpVectorValueFace(const ElementInfo info,
                                     const unsigned int side,
                                     const unsigned int qp,
                                     const unsigned int var,
                                     const TagID tag) const
{
  auto fe = _var_fe_types[var];
  auto n_dofs = kokkosAssembly().getNumDofs(info.type, fe);
  auto & phi = kokkosAssembly().getVectorPhiFace(info.subdomain, info.type, fe)(side);

  Real3 value = 0;

  for (unsigned int i = 0; i < n_dofs; ++i)
    value += getVectorDofValue(getElemLocalDofIndex(info.id, i, var), tag) * phi(i, qp);

  return value;
}

KOKKOS_FUNCTION inline ADReal
FESystem::getVectorQpADValueFace(const ElementInfo info,
                                 const unsigned int side,
                                 const unsigned int qp,
                                 const unsigned int var,
                                 const TagID tag,
                                 const Real seed) const
{
  auto fe = _var_fe_types[var];
  auto n_dofs = kokkosAssembly().getNumDofs(info.type, fe);
  auto & phi = kokkosAssembly().getPhiFace(info.subdomain, info.type, fe)(side);

  ADReal value = 0;

  for (unsigned int i = 0; i < n_dofs; ++i)
    value += getVectorDofADValue(getElemLocalDofIndex(info.id, i, var), tag, seed) * phi(i, qp);

  return value;
}

KOKKOS_FUNCTION inline Real3
FESystem::getVectorQpGradFace(const ElementInfo info,
                              const unsigned int side,
                              const Real33 jacobian,
                              const unsigned int qp,
                              const unsigned int var,
                              const TagID tag) const
{
  auto fe = _var_fe_types[var];
  auto n_dofs = kokkosAssembly().getNumDofs(info.type, fe);
  auto & grad_phi = kokkosAssembly().getGradPhiFace(info.subdomain, info.type, fe)(side);

  Real3 grad = 0;

  for (unsigned int i = 0; i < n_dofs; ++i)
    grad += getVectorDofValue(getElemLocalDofIndex(info.id, i, var), tag) *
            (jacobian * grad_phi(i, qp));

  return grad;
}

KOKKOS_FUNCTION inline Real33
FESystem::getVectorQpVectorGradFace(const ElementInfo info,
                                    const unsigned int side,
                                    const Real33 jacobian,
                                    const unsigned int qp,
                                    const unsigned int var,
                                    const TagID tag) const
{
  auto fe = _var_fe_types[var];
  auto n_dofs = kokkosAssembly().getNumDofs(info.type, fe);
  auto & grad_phi = kokkosAssembly().getVectorGradPhiFace(info.subdomain, info.type, fe)(side);

  Real33 grad = 0;

  for (unsigned int i = 0; i < n_dofs; ++i)
    grad += getVectorDofValue(getElemLocalDofIndex(info.id, i, var), tag) *
            (grad_phi(i, qp) * jacobian.transpose());

  return grad;
}

KOKKOS_FUNCTION inline ADReal3
FESystem::getVectorQpADGradFace(const ElementInfo info,
                                const unsigned int side,
                                const Real33 jacobian,
                                const unsigned int qp,
                                const unsigned int var,
                                const TagID tag,
                                const Real seed) const
{
  auto fe = _var_fe_types[var];
  auto n_dofs = kokkosAssembly().getNumDofs(info.type, fe);
  auto & grad_phi = kokkosAssembly().getGradPhiFace(info.subdomain, info.type, fe)(side);

  ADReal3 grad = ADReal(0);

  for (unsigned int i = 0; i < n_dofs; ++i)
    grad += getVectorDofADValue(getElemLocalDofIndex(info.id, i, var), tag, seed) *
            (jacobian * grad_phi(i, qp));

  return grad;
}
#endif

MakeSystemHolder(FESystem);
} // namespace Moose::Kokkos
