//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosTypes.h"
#include "KokkosVector.h"
#include "KokkosMatrix.h"
#include "KokkosDofSpace.h"
#include "PerfGraphInterface.h"

class SystemBase;

namespace Moose::Kokkos
{

/**
 * The Kokkos base system class. Each system in MOOSE has a corresponding Kokkos
 * base system. On top of the system's DOF layout it carries the tagged vectors and matrices the
 * residual objects assemble into.
 */
class System : public PerfGraphInterface, public DofSpace
{
public:
  /**
   * Constructor
   * @param system The associated MOOSE system
   */
  System(SystemBase & system);

  /**
   * Defaulted copy constructor
   * Used by FESystem in mixed FE+FV simulations.
   */
  System(const System & src) = default;

#ifdef MOOSE_KOKKOS_SCOPE
  /**
   * Synchronize the active tagged vectors and matrices between host and device
   * @param dir Copy direction
   */
  void sync(const MemcpyType dir);
  /**
   * Synchronize the specified tagged vectors between host and device
   * @param tags The vector tags
   * @param dir Copy direction
   */
  ///@{
  void sync(const std::set<TagID> & tags, const MemcpyType dir);
  void sync(const std::vector<TagID> & tags, const MemcpyType dir);
  void sync(const TagID tag, const MemcpyType dir);
  ///@}

  /**
   * Set the active variables
   * @param vars The active MOOSE variables
   */
  void setActiveVariables(const std::set<MooseVariableFieldBase *> & vars);

  /**
   * Set the active tags whose vectors are read by device code and carry no off-process
   * contributions, which are synchronized to the device and released without being assembled. The
   * tags a MOOSE system holds a solution in are the usual case, but the category is the access
   * mode and not the kind of quantity: an operator application reads a residual this way too.
   * @param tags The active read tags
   * @param read_only Whether device code only reads the vectors, which takes their arrays through
   * PETSc's read-only accessor and so accepts a vector the caller has locked against writes
   */
  void setActiveReadTags(const std::set<TagID> & tags, bool read_only = false);

  /**
   * Set the active tags whose vectors device code contributes to, which are assembled on release
   * so that contributions to degrees of freedom owned by another process reach their owner
   * @param tags The active assembled tags
   */
  void setActiveAssembledTags(const std::set<TagID> & tags);

  /**
   * Set the active matrix tags
   * @param vars The active matrix tags
   */
  void setActiveMatrixTags(const std::set<TagID> & tags);

  /**
   * Clear the cached active variables
   */
  void clearActiveVariables() { _active_variables.destroy(); }

  /**
   * Clear the cached active read tags
   */
  void clearActiveReadTags()
  {
    _active_read_tags.destroy();
    _read_tags_read_only = false;
  }

  /**
   * Clear the cached active assembled tags
   */
  void clearActiveAssembledTags()
  {
    _active_assembled_tags.destroy();
    _assembled_tag_active = false;
  }

  /**
   * Clear the cached active matrix tags
   */
  void clearActiveMatrixTags()
  {
    _active_matrix_tags.destroy();
    _matrix_tag_active = false;
  }

  /**
   * Get the MOOSE system
   * @returns The MOOSE system
   */
  ///@{
  SystemBase & getSystem() { return _system; }
  const SystemBase & getSystem() const { return _system; }
  ///@}

  /**
   * Check whether a variable is active on a subdomain
   * @param var The variable number
   * @param subdomain The contiguous subdomain ID
   * @returns Whether the variable is active
   */
  KOKKOS_FUNCTION bool isVariableActive(unsigned int var, ContiguousSubdomainID subdomain) const
  {
    return _var_subdomain_active(var, subdomain);
  }

  /**
   * Check whether an assembled tag is active
   * @param tag The assembled tag
   * @returns Whether the assembled tag is active
   */
  KOKKOS_FUNCTION bool isAssembledTagActive(TagID tag) const { return _assembled_tag_active[tag]; }

  /**
   * Check whether a matrix tag is active
   * @param tag The matrix tag
   * @returns Whether the matrix tag is active
   */
  KOKKOS_FUNCTION bool isMatrixTagActive(TagID tag) const { return _matrix_tag_active[tag]; }

  /**
   * Get a tagged Kokkos vector
   * @param tag The vector tag
   * @returns The Kokkos vector
   */
  KOKKOS_FUNCTION Vector & getVector(TagID tag) const { return _vectors[tag]; }

  /**
   * Get a tagged Kokkos matrix
   * @param tag The matrix tag
   * @returns The Kokkos matrix
   */
  KOKKOS_FUNCTION Matrix & getMatrix(TagID tag) const { return _matrices[tag]; }

  /**
   * Get the DOF value of a tagged vector
   * @param dof The local DOF index
   * @param tag The vector tag
   * @returns The DOF value
   */
  KOKKOS_FUNCTION Real & getVectorDofValue(const dof_id_type dof, const TagID tag) const
  {
    return _vectors[tag][dof];
  }

  /**
   * Get an entry from a tagged matrix
   * @param row The local row index
   * @param col The global column index
   * @param tag The matrix tag
   * @returns The entry from the tagged matrix
   */
  KOKKOS_FUNCTION Real & getMatrixValue(dof_id_type row, dof_id_type col, TagID tag) const
  {
    return _matrices[tag](row, col);
  }

#endif

protected:
  /**
   * Reference of the MOOSE system
   */
  SystemBase & _system;

  /**
   * Kokkos vectors and matrices on device
   */
  ///@{
  Array<Vector> _vectors;
  Array<Matrix> _matrices;
  ///@}

  /**
   * Whether each variable is active on subdomains
   */
  Array2D<bool> _var_subdomain_active;

  /**
   * List of active variable numbers
   */
  Array<unsigned int> _active_variables;

  /**
   * List of active tags
   */
  ///@{
  Array<TagID> _active_read_tags;
  Array<TagID> _active_assembled_tags;
  Array<TagID> _active_matrix_tags;
  ///@}

  /**
   * Flag whether each tag is active
   */
  ///@{
  Array<bool> _assembled_tag_active;
  Array<bool> _matrix_tag_active;
  ///@}

  /**
   * Flag whether the active read tags are only read, in which case their arrays are taken through
   * PETSc's read-only accessor
   */
  bool _read_tags_read_only = false;

private:
  /**
   * Setup variable data
   */
  void setupVariables();

  /**
   * Allocate the tagged vector, matrix and tag activity data
   */
  void setupTags();
};

#ifdef MOOSE_KOKKOS_SCOPE
#define MakeSystemHolderMethods(SystemTypeName)                                                    \
  KOKKOS_FUNCTION const Array<SystemTypeName> & kokkosSystems() const                              \
  {                                                                                                \
    KOKKOS_IF_ON_HOST(return _systems_host;)                                                       \
    return _systems_device;                                                                        \
  }                                                                                                \
  Array<SystemTypeName> & kokkosSystems() { return _systems_host; }                                \
  KOKKOS_FUNCTION const SystemTypeName & kokkosSystem(unsigned int sys) const                      \
  {                                                                                                \
    KOKKOS_IF_ON_HOST(return _systems_host[sys];)                                                  \
    return _systems_device[sys];                                                                   \
  }                                                                                                \
  SystemTypeName & kokkosSystem(unsigned int sys) { return _systems_host[sys]; }
#else
#define MakeSystemHolderMethods(SystemTypeName)
#endif

/**
 * The Kokkos interface that holds the host reference of the Kokkos systems and copies it to device
 * during parallel dispatch.
 * Maintains synchronization between host and device Kokkos systems and provides access to the
 * appropriate Kokkos systems depending on the architecture.
 */
// clang-format off
#define MakeSystemHolder(SystemTypeName)                                                           \
  class SystemTypeName##Holder                                                                     \
  {                                                                                                \
  public:                                                                                          \
    SystemTypeName##Holder(Array<SystemTypeName> & systems)                                        \
      : _systems_host(systems), _systems_device(systems)                                           \
    {                                                                                              \
    }                                                                                              \
    SystemTypeName##Holder(const SystemTypeName##Holder & holder)                                  \
      : _systems_host(holder._systems_host), _systems_device(holder._systems_host)                 \
    {                                                                                              \
    }                                                                                              \
    MakeSystemHolderMethods(SystemTypeName)                                                        \
  private:                                                                                         \
    Array<SystemTypeName> & _systems_host;                                                         \
    const Array<SystemTypeName> _systems_device;                                                   \
  }
// clang-format on

MakeSystemHolder(System);
} // namespace Moose::Kokkos
