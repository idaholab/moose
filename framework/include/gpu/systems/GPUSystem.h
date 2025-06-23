//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GPUTypes.h"
#include "GPUVector.h"
#include "GPUMatrix.h"
#include "GPUAssembly.h"

#include "SystemBase.h"

#include "libmesh/communicator.h"

class MooseMesh;
class SystemBase;
class GPUNodalBCBase;

class GPUSystem : public GPUMeshHolder, public GPUAssemblyHolder
{
private:
  void setupVariables();
  void setupDofs();
  void setupSparsity();
  void checkNodalBCs();

private:
  void getNodalBCDofs(const GPUNodalBCBase * nbc, GPUArray<bool> & dofs);

public:
  /**
   * Constructor
   * @param system Associated MOOSE system
   */
  GPUSystem(SystemBase & system);
  /**
   * Initialize the GPU system
   */
  void init();
  /**
   * Check whether the GPU system was initialized
   */
  auto initialized() { return _initialized; }

#ifdef MOOSE_GPU_SCOPE
  /**
   * Prepare and copy in or out vectors and matrices
   * @param dir Copy direction
   */
  void sync(GPUMemcpyKind dir);
  /**
   * Copy in or out vectors corresponding to the given tags
   * @param tags A set of vector tags to be copied in or out
   * @param dir Copy direction
   */
  void sync(std::set<TagID> tags, GPUMemcpyKind dir);
  /**
   * Preallocate quadrature point solution vectors
   */
  void preallocate();
  /**
   * Cache active variables
   * @param vars A set of MOOSE variables to be cached
   */
  void setActiveVariables(std::set<MooseVariableFieldBase *> vars);
  /**
   * Cache active variable tags
   * @param vars A set of variable tags to be cached
   */
  void setActiveVariableTags(std::set<TagID> tags);
  /**
   * Cache active residual tags
   * @param vars A set of residual tags to be cached
   */
  void setActiveResidualTags(std::set<TagID> tags);
  /**
   * Cache active matrix tags
   * @param vars A set of matrix tags to be cached
   */
  void setActiveMatrixTags(std::set<TagID> tags);
  /**
   * Set quadrature projection status flags of active variables and tags for given subdomains
   * @param subdomains List of subdomains
   */
  void setProjectionFlags(std::set<SubdomainID> subdomains);
  /**
   * Clear cached active variables
   */
  void clearActiveVariables() { _active_variables.destroy(); }
  /**
   * Clear cached active variable tags
   */
  void clearActiveVariableTags() { _active_variable_tags.destroy(); }
  /**
   * Clear cached active residual tags
   */
  void clearActiveResidualTags()
  {
    _active_residual_tags.destroy();
    _residual_tag_active = false;
  }
  /**
   * Clear cached active matrix tags
   */
  void clearActiveMatrixTags()
  {
    _active_matrix_tags.destroy();
    _matrix_tag_active = false;
  }
  /**
   * Clear quadrature projection status flags
   */
  void clearProjectionFlags()
  {
    for (auto & flags : _variable_on_qp)
      if (flags.isAlloc())
        flags = false;
  }
  /**
   * Get the MOOSE system
   */
  const auto & getSystem() const { return _system; }
  /**
   * Get the libMesh DOF map
   */
  const auto & getDofMap() const { return _system.dofMap(); }
  /**
   * Get the libMesh communicator
   */
  const auto & getComm() const { return *_comm; }
  /**
   * Get the list of local DOF indices to send/receive
   */
  const auto & getLocalCommList() const { return _local_comm_list; }
  /**
   * Get the list of ghost DOF indices to send/receive
   */
  const auto & getGhostCommList() const { return _ghost_comm_list; }
  /**
   * Get the sparisty pattern data
   */
  const auto & getSparsity() const { return _sparsity; }
  /**
   * Get the off-diagonal coupled variables of a variable
   * @param var Variable number
   */
  KOKKOS_FUNCTION const auto & getCoupling(unsigned int var) const { return _coupling[var]; }
  /**
   * Check whether a variable is active on a subdomain
   * @param var Variable number
   * @param subdomain Subdomain ID
   */
  KOKKOS_FUNCTION auto isVariableActive(unsigned int var, SubdomainID subdomain) const
  {
    return _var_subdomain_active(var, subdomain);
  }
  /**
   * Check whether a residual tag is active
   * @param tag Residual tag
   */
  KOKKOS_FUNCTION auto isResidualTagActive(TagID tag) const { return _residual_tag_active[tag]; }
  /**
   * Check whether a matrix tag is active
   * @param tag Matrix tag
   */
  KOKKOS_FUNCTION auto isMatrixTagActive(TagID tag) const { return _matrix_tag_active[tag]; }
  /**
   * Check whether a local DOF index is associated with a nodal BC
   * @param dof Local DOF index
   */
  KOKKOS_FUNCTION auto hasNodalBC(dof_id_type dof) const
  {
    return _nbc_dof.isAlloc() && _nbc_dof[dof];
  }
  /**
   * Check whether a local DOF index is associated with a nodal BC for an extra residual tag
   * @param dof Local DOF index
   * @param tag Residual tag
   */
  KOKKOS_FUNCTION auto hasNodalBCResidualTag(dof_id_type dof, TagID tag) const
  {
    return _nbc_residual_tag_dof[tag].isAlloc() && _nbc_residual_tag_dof[tag][dof];
  }
  /**
   * Check whether a local DOF index is associated with a nodal BC for an extra matrix tag
   * @param dof Local DOF index
   * @param tag Matrix tag
   */
  KOKKOS_FUNCTION auto hasNodalBCMatrixTag(dof_id_type dof, TagID tag) const
  {
    return _nbc_matrix_tag_dof[tag].isAlloc() && _nbc_matrix_tag_dof[tag][dof];
  }
  /**
   * Get the FE type number of a variable
   * @param var Variable number
   */
  KOKKOS_FUNCTION auto getFETypeNum(unsigned int var) const { return _var_fe_types[var]; }
  /**
   * Get the number of local DOFs
   */
  KOKKOS_FUNCTION auto getNumLocalDofs() const { return _n_local_dofs; }
  /**
   * Get the number of ghost DOFs
   */
  KOKKOS_FUNCTION auto getNumGhostDofs() const { return _n_ghost_dofs; }
  /**
   * Get the local maximum number of DOFs per element of a variable
   * @param var Variable number
   */
  KOKKOS_FUNCTION auto getMaxDofsPerElem(unsigned int var) const { return _max_dofs_per_elem[var]; }
  /**
   * Get the local DOF index of a variable given a local element ID and element-local DOF index
   * @param elem Local element ID
   * @param i Element-local DOF index
   * @param var Variable number
   */
  KOKKOS_FUNCTION auto
  getElemLocalDofIndex(dof_id_type elem, unsigned int i, unsigned int var) const
  {
    return _local_elem_dof_index[var](elem, i);
  }
  /**
   * Get the local DOF index of a variable given a local node ID
   * @param node Local node ID
   * @param var Variable number
   */
  KOKKOS_FUNCTION auto getNodeLocalDofIndex(dof_id_type node, unsigned int var) const
  {
    return _local_node_dof_index[var][node];
  }
  /**
   * Get the global DOF index of a variable given a local element ID and element-local DOF index
   * @param elem Local element ID
   * @param i Element-local DOF index
   * @param var Variable number
   */
  KOKKOS_FUNCTION auto
  getElemGlobalDofIndex(dof_id_type elem, unsigned int i, unsigned int var) const
  {
    return _local_to_global_dof_index[_local_elem_dof_index[var](elem, i)];
  }
  /**
   * Get the global DOF index of a variable given a local node ID
   * @param node Local node ID
   * @param var Variable number
   */
  KOKKOS_FUNCTION auto getNodeGlobalDofIndex(dof_id_type node, unsigned int var) const
  {
    return _local_to_global_dof_index[_local_node_dof_index[var][node]];
  }
  /**
   * Convert the local DOF index to the global DOF index
   * @param dof Local DOF index
   */
  KOKKOS_FUNCTION auto localToGlobalDofIndex(dof_id_type dof) const
  {
    return _local_to_global_dof_index[dof];
  }
  /**
   * Get the vector associated with a tag
   * @param tag Vector tag
   */
  KOKKOS_FUNCTION auto & getVector(TagID tag) const { return _vectors[tag]; }
  /**
   * Get the matrix associated with a tag
   * @param tag Matrix tag
   */
  KOKKOS_FUNCTION auto & getMatrix(TagID tag) const { return _matrices[tag]; }
  /**
   * Get the DOF value of a vector associated with a tag
   * @param dof Local DOF index
   * @param tag Vector tag
   */
  KOKKOS_FUNCTION auto & getVectorDofValue(dof_id_type dof, TagID tag) const
  {
    return _vectors[tag][dof];
  }
  /**
   * Get the DOF value of a matrix associated with a tag
   * @param row Local DOF row index
   * @param col Global DOF column index
   * @param tag Matrix tag
   */
  KOKKOS_FUNCTION auto & getMatrixDofValue(dof_id_type row, dof_id_type col, TagID tag) const
  {
    return _matrices[tag](row, col);
  }
  /**
   * Get the element quadrature point value of a variable associated with a tag
   * @param info Element information object
   * @param qp Serialized elemental quadrature point index
   * @param var Variable number
   * @param tag Vector tag
   */
  KOKKOS_FUNCTION auto &
  getVectorQpValue(GPUElementInfo info, dof_id_type qp, unsigned int var, TagID tag) const
  {
    return _qp_solutions[tag](info.subdomain, var)[qp];
  }
  /**
   * Get the element quadrature point gradient of a variable associated with a tag
   * @param info Element information object
   * @param qp Serialized elemental quadrature point index
   * @param var Variable number
   * @param tag Vector tag
   */
  KOKKOS_FUNCTION auto &
  getVectorQpGrad(GPUElementInfo info, dof_id_type qp, unsigned int var, TagID tag) const
  {
    return _qp_solutions_grad[tag](info.subdomain, var)[qp];
  }
  /**
   * Get the face quadrature point value of a variable associated with a tag
   * @param info Element information object
   * @param side Side index
   * @param qp_local Face-local quadrature point index
   * @param var Variable number
   * @param tag Vector tag
   */
  KOKKOS_FUNCTION auto getVectorQpValueFace(GPUElementInfo info,
                                            unsigned int side,
                                            unsigned int qp_local,
                                            unsigned int var,
                                            TagID tag) const
  {
    auto fe = _var_fe_types[var];
    auto n_dofs = assembly().getNumDofs(info.type, fe);
    auto & phi = assembly().getPhiFace(info.subdomain, info.type, fe);

    Real value = 0;

    for (unsigned int i = 0; i < n_dofs; ++i)
      value +=
          getVectorDofValue(getElemLocalDofIndex(info.id, i, var), tag) * phi(i, qp_local, side);

    return value;
  }
  /**
   * Get the face quadrature point gradient of a variable associated with a tag
   * @param info Element information object
   * @param side Side index of the face
   * @param jacobian Jacobian matrix
   * @param qp_local Face-local quadrature point index
   * @param var Variable number
   * @param tag Vector tag
   */
  KOKKOS_FUNCTION auto getVectorQpGradFace(GPUElementInfo info,
                                           unsigned int side,
                                           Real33 jacobian,
                                           unsigned int qp_local,
                                           unsigned int var,
                                           TagID tag) const
  {
    auto fe = _var_fe_types[var];
    auto n_dofs = assembly().getNumDofs(info.type, fe);
    auto & grad_phi = assembly().getGradPhiFace(info.subdomain, info.type, fe);

    Real3 grad = 0;

    for (unsigned int i = 0; i < n_dofs; ++i)
      grad += getVectorDofValue(getElemLocalDofIndex(info.id, i, var), tag) *
              (jacobian * grad_phi(i, qp_local, side));

    return grad;
  }

  /**
   * Check whether reinit should be called to compute quadrature point values
   * @param info Element information object
   */
  KOKKOS_FUNCTION bool needReinit(GPUElementInfo info)
  {
    auto subdomain = info.subdomain;

    for (unsigned int t = 0; t < _active_variable_tags.size(); ++t)
      for (unsigned int v = 0; v < _active_variables.size(); ++v)
      {
        auto tag = _active_variable_tags[t];
        auto var = _active_variables[v];

        if (!_variable_on_qp[tag](subdomain, var))
          return true;
      }

    return false;
  }
  /**
   * Cache quadrature point values and gradients of active variables and tags
   * @param info Element information object
   * @param jacobian Jacobian matrix
   * @param qp Serialized elemental quadrature point index
   * @param qp_local Element-local quadrature point index
   */
  KOKKOS_FUNCTION void
  reinit(GPUElementInfo info, Real33 jacobian, unsigned int qp, unsigned int qp_local)
  {
    auto elem = info.id;
    auto elem_type = info.type;
    auto subdomain = info.subdomain;

    for (unsigned int t = 0; t < _active_variable_tags.size(); ++t)
      for (unsigned int v = 0; v < _active_variables.size(); ++v)
      {
        auto tag = _active_variable_tags[t];
        auto var = _active_variables[v];

        if (_variable_on_qp[tag](subdomain, var))
          continue;

        auto fe = _var_fe_types[var];
        auto n_dofs = assembly().getNumDofs(elem_type, fe);

        auto & phi = assembly().getPhi(subdomain, elem_type, fe);
        auto & grad_phi = assembly().getGradPhi(subdomain, elem_type, fe);

        Real value = 0;
        Real3 grad = 0;

        for (unsigned int i = 0; i < n_dofs; ++i)
        {
          auto vector = getVectorDofValue(getElemLocalDofIndex(elem, i, var), tag);

          value += vector * phi(i, qp_local);
          grad += vector * (jacobian * grad_phi(i, qp_local));
        }

        getVectorQpValue(info, qp, var, tag) = value;
        getVectorQpGrad(info, qp, var, tag) = grad;
      }
  }
#endif

private:
  // Whether the GPU system was initialized
  bool _initialized = false;
  // GPU copy of vectors
  GPUArray<GPUVector> _vectors;
  // GPU copy of matrices
  GPUArray<GPUMatrix> _matrices;
  // Element quadrature point values of variables
  GPUArray<GPUArray2D<GPUArray<Real>>> _qp_solutions;
  // Element quadrature point gradients of variables
  GPUArray<GPUArray2D<GPUArray<Real3>>> _qp_solutions_grad;
  // Whether variables were projected to quadrature points
  GPUArray<GPUArray2D<bool>> _variable_on_qp;
  // Local DOF index of each variable at elements
  GPUArray<GPUArray2D<dof_id_type>> _local_elem_dof_index;
  // Local DOF index of each variable at nodes
  GPUArray<GPUArray<dof_id_type>> _local_node_dof_index;
  // Local DOF index to global DOF index
  GPUArray<dof_id_type> _local_to_global_dof_index;
  // Maximum number of DOFs per element of each variable
  GPUArray<dof_id_type> _max_dofs_per_elem;
  // FE type number of variables
  GPUArray<unsigned int> _var_fe_types;
  // Whether variables are active on subdomains
  GPUArray2D<bool> _var_subdomain_active;
  // Coupled variables
  GPUArray<GPUArray<unsigned int>> _coupling;

private:
  // Reference to the MOOSE system
  SystemBase & _system;
  // Reference to the MOOSE mesh
  const MooseMesh & _mesh;
  // Pointer to the libMesh communicator
  const Parallel::Communicator * _comm = nullptr;
  // Number of variables
  unsigned int _n_vars = 0;
  // Number of processes
  processor_id_type _n_procs = 0;
  // Number of local elements
  dof_id_type _n_elems = 0;
  // Number of local nodes
  dof_id_type _n_nodes = 0;
  // Number of local DOFs
  dof_id_type _n_local_dofs = 0;
  // Number of ghost DOFs
  dof_id_type _n_ghost_dofs = 0;

private:
  // List of active variable numbers
  GPUArray<unsigned int> _active_variables;
  // List of active variable tags
  GPUArray<TagID> _active_variable_tags;
  // List of active residual tags
  GPUArray<TagID> _active_residual_tags;
  // List of active matrix tags
  GPUArray<TagID> _active_matrix_tags;
  // Whether a residual tag is active
  GPUArray<bool> _residual_tag_active;
  // Whether a matrix tag is active
  GPUArray<bool> _matrix_tag_active;
  // Whether a DOF is associated with a nodal BC
  GPUArray<bool> _nbc_dof;
  // Whether a DOF is associated with a nodal BC for extra tags
  GPUArray<GPUArray<bool>> _nbc_residual_tag_dof;
  GPUArray<GPUArray<bool>> _nbc_matrix_tag_dof;
  // List of DOFs to send and receive
  GPUArray<GPUArray<dof_id_type>> _local_comm_list;
  GPUArray<GPUArray<dof_id_type>> _ghost_comm_list;

public:
  struct Sparsity
  {
    GPUArray<PetscInt> col_idx;
    GPUArray<PetscInt> row_idx;
    GPUArray<PetscInt> row_ptr;
  };

private:
  // Sparsity pattern data
  Sparsity _sparsity;
};

class GPUSystemHolder
{
private:
  // Copy of GPU systems
  GPUArray<GPUSystem> _systems_device;
  // Reference to GPU systems
  GPUArray<GPUSystem> & _systems_host;

#ifdef MOOSE_GPU_SCOPE
public:
  KOKKOS_FUNCTION const GPUArray<GPUSystem> & systems() const
  {
    KOKKOS_IF_ON_HOST(return _systems_host;)
    KOKKOS_IF_ON_DEVICE(return _systems_device;)
  }
  KOKKOS_FUNCTION GPUArray<GPUSystem> & systems()
  {
    KOKKOS_IF_ON_HOST(return _systems_host;)
    KOKKOS_IF_ON_DEVICE(return _systems_device;)
  }
  KOKKOS_FUNCTION const GPUSystem & system(unsigned int sys) const
  {
    KOKKOS_IF_ON_HOST(return _systems_host[sys];)
    KOKKOS_IF_ON_DEVICE(return _systems_device[sys];)
  }
  KOKKOS_FUNCTION GPUSystem & system(unsigned int sys)
  {
    KOKKOS_IF_ON_HOST(return _systems_host[sys];)
    KOKKOS_IF_ON_DEVICE(return _systems_device[sys];)
  }
#endif

public:
  GPUSystemHolder(GPUArray<GPUSystem> & systems) : _systems_device(systems), _systems_host(systems)
  {
  }
  GPUSystemHolder(const GPUSystemHolder & holder)
    : _systems_device(holder._systems_host), _systems_host(holder._systems_host)
  {
  }
};
