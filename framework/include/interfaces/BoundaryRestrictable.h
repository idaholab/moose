//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#ifdef MOOSE_KOKKOS_ENABLED
#include "KokkosTypes.h"
#endif

// MOOSE includes
#include "InputParameters.h"
#include "MaterialData.h"

class MooseMesh;

/**
 * /class BoundaryRestrictable
 * /brief Provides functionality for limiting the object to certain boundary ids
 * The is class the inheriting class with methods useful for limiting an object
 * to certain boundaries. The parameters "_boundary_id" and "boundary", which are
 * created with BoundaryRestrictable::validParams() are used the framework.
 */
class BoundaryRestrictable
{
public:
  /// A flag changing the behavior of hasBoundary
  enum TEST_TYPE
  {
    ALL,
    ANY
  };

  /**
   * Class constructor
   * Populates the _bnd_ids for the given boundary names supplied
   * with the 'boundary' input parameter
   * @param parameters The input parameters
   * @param nodal True indicates that the object is operating on nodesets, false for sidesets
   */
  BoundaryRestrictable(const MooseObject * moose_object, bool nodal);

  static InputParameters validParams();

  /**
   * Class constructor
   * Populates the 'block' input parameters when an object is also block restricted,
   * see the general class documentation for details.
   * @param parameters The input parameters (see the detailed help for additional information)
   * @param block_ids The block ids that the object is restricted to
   * @param nodal True indicates that the object is operating on nodesets, false for sidesets
   */
  BoundaryRestrictable(const MooseObject * moose_object,
                       const std::set<SubdomainID> & block_ids,
                       bool nodal);

#ifdef MOOSE_KOKKOS_SCOPE
  /**
   * Class copy constructor
   * Used for dispatching Kokkos functor.
   * Only defined for Kokkos objects.
   */
  BoundaryRestrictable(const BoundaryRestrictable & object);
#endif

  /**
   * Helper for determining if the object is boundary restricted. This is needed for the
   * MaterialPropertyInterface.
   */
  static bool restricted(const std::set<BoundaryID> & ids);

  /**
   * Empty class destructor
   */
  virtual ~BoundaryRestrictable();

  /**
   * Return the boundary IDs for this object
   * @return A set of all boundary ids for which the object is restricted
   */
  const virtual std::set<BoundaryID> & boundaryIDs() const;

  /**
   * Return the boundary names for this object
   * @return A set of all boundary names for which the object is restricted
   */
  const std::vector<BoundaryName> & boundaryNames() const;

  /**
   * Return the number of boundaries for this object
   * @return The number of boundary ids
   */
  unsigned int numBoundaryIDs() const;

  /**
   * Test if the supplied boundary name is valid for this object
   * @param name A BoundaryName to check
   * @return True if the given id is valid for this object
   */
  bool hasBoundary(const BoundaryName & name) const;

  /**
   * Test if the supplied vector of boundary names are valid for this object
   * @param names A vector of BoundaryNames to check
   * @return True if the given ids are valid for this object
   */
  bool hasBoundary(const std::vector<BoundaryName> & names) const;

  /**
   * Test if the supplied boundary ids are valid for this object
   * @param id A BoundaryID to check
   * @return True if the given id is valid for this object
   */
  bool hasBoundary(const BoundaryID & id) const;

  /**
   * Test if the supplied vector boundary ids are valid for this object
   * @param ids A vector of BoundaryIDs ids to check
   * @param type A flag for the type of matching to perform: ALL requires that all supplied
   * ids must match those of the object; ANY requires that any one of the supplied ids must
   * match those of the object
   * @return True if the all of the given ids are found within the ids for this object
   */
  bool hasBoundary(const std::vector<BoundaryID> & ids, TEST_TYPE type = ALL) const;

  /**
   * Test if the supplied set of boundary ids are valid for this object
   * @param ids A std::set of BoundaryIDs to check
   * @param type A flag for the type of matching to perform: ALL requires that all supplied
   * ids must match those of the object; ANY requires that any one of the supplied ids must
   * match those of the object
   *
   * @return True if the all of the given ids are found within the ids for this object
   * \see isSubset
   */
  bool hasBoundary(const std::set<BoundaryID> & ids, TEST_TYPE type = ALL) const;

  /**
   * Test if the class boundary ids are a subset of the supplied objects
   * @param ids A std::set of boundaries to check
   * @return True if all of the boundary ids for this class are found within the given ids (opposite
   * of hasBoundary)
   * \see hasBoundary
   */
  bool isBoundarySubset(const std::set<BoundaryID> & ids) const;

  /*
   * Test if the class boundary ids are a subset of the supplied objects
   * @param ids A std::set of Boundary IDs to check
   * @return True if all of the boundary ids for this class are found within the given ids (opposite
   * of hasBoundary)
   * \see hasBoundary
   */
  bool isBoundarySubset(const std::vector<BoundaryID> & ids) const;

  /**
   * Check if a material property is valid for all boundaries of this object
   *
   * This method returns true if the supplied property name has been declared
   * in a Material object on the boundary ids for this object.
   *
   * @tparam T The type of material property
   * @param prop_name the name of the property to query
   * @return true if the property exists for all boundary ids of the object, otherwise false
   */
  template <typename T, bool is_ad = false>
  bool hasBoundaryMaterialProperty(const std::string & prop_name) const;

  /**
   * Returns true if this object has been restricted to a boundary
   * @see MooseObject
   */
  virtual bool boundaryRestricted() const;

  /**
   * Returns the set of all boundary ids for the entire mesh
   * @return A const reference the the boundary ids for the entire mesh
   */
  const std::set<BoundaryID> & meshBoundaryIDs() const;

  /**
   * Whether integrity/coverage checking should be conducted for moose variables used in this
   * object. This should return true if variables are only evaluated locally, e.g. on the current
   * node or element. This should return false if evaluation of this object entails non-local
   * variable evaluations
   */
  virtual bool checkVariableBoundaryIntegrity() const { return true; }

private:
  /// Pointer to FEProblemBase
  FEProblemBase * _bnd_feproblem;

  /// Point to mesh
  MooseMesh * _bnd_mesh;

  /// Set of the boundary ids
  std::set<BoundaryID> _bnd_ids;

  /// Vector of the boundary ids
  std::vector<BoundaryID> _vec_ids;

  /// Vector the the boundary names
  std::vector<BoundaryName> _boundary_names;

  /// Flag for allowing dual restriction with BlockRestrictable
  const bool _bnd_dual_restrictable;

  /// An empty set for referencing when block_ids is not included
  const std::set<SubdomainID> _empty_block_ids;

  /// Reference to the block_ids, defaults to an empty set if not provided
  const std::set<SubdomainID> & _block_ids;

  /// Thread id for this object
  THREAD_ID _bnd_tid;

  /// Pointer to MaterialData for boundary (@see hasBoundaryMaterialProperty)
  const MaterialData & _bnd_material_data;

  /// Whether or not this object is restricted to nodesets
  bool _bnd_nodal;

  /// The moose object that this is an interface for
  const MooseObject & _moose_object;

  /**
   * An initialization routine needed for dual constructors
   */
  void initializeBoundaryRestrictable();

#ifdef MOOSE_KOKKOS_ENABLED
  void initializeKokkosBoundaryRestrictable(MooseMesh * mesh);
#endif

protected:
  /**
   * A helper method to avoid circular #include problems.
   * @see hasBoundaryMaterialProperty
   */
  bool hasBoundaryMaterialPropertyHelper(const std::string & prop_name) const;

#ifdef MOOSE_KOKKOS_SCOPE
  /**
   * Get the number of nodes this Kokkos object is operating on
   * @returns The number of nodes local to this process
   */
  KOKKOS_FUNCTION dof_id_type numKokkosBoundaryNodes() const { return _kokkos_node_ids.size(); }
  /**
   * Get the number of sides this Kokkos object is operating on
   * @returns The number of sides local to this process
   */
  KOKKOS_FUNCTION dof_id_type numKokkosBoundarySides() const
  {
    return _kokkos_element_side_ids.size();
  }
  /**
   * Get the node ID this Kokkos thread is operating on
   * @param tid The thread ID
   * @returns The node ID
   */
  KOKKOS_FUNCTION dof_id_type kokkosBoundaryNodeID(dof_id_type tid) const
  {
    return _kokkos_node_ids[tid];
  }
  /**
   * Get the element ID - side index pair this Kokkos thread is operating on
   * @param tid The thread ID
   * @returns The element ID - side index pair
   */
  KOKKOS_FUNCTION auto kokkosBoundaryElementSideID(dof_id_type tid) const
  {
    return _kokkos_element_side_ids[tid];
  }
#endif

#ifdef MOOSE_KOKKOS_ENABLED
private:
  /**
   * List of node IDs this Kokkos object is operating on
   */
  Moose::Kokkos::Array<dof_id_type> _kokkos_node_ids;
  /**
   * List of element ID - side index pairs this Kokkos object is operating on
   */
  Moose::Kokkos::Array<Moose::Kokkos::Pair<dof_id_type, unsigned int>> _kokkos_element_side_ids;
#endif
};

template <typename T, bool is_ad>
bool
BoundaryRestrictable::hasBoundaryMaterialProperty(const std::string & prop_name) const
{
  // If you get here the supplied property is defined on all boundaries, but is still subject
  // existence in the MateialData class
  return hasBoundaryMaterialPropertyHelper(prop_name) &&
         _bnd_material_data.haveGenericProperty<T, is_ad>(prop_name);
}
