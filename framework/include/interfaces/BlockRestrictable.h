//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "InputParameters.h"
#include "ParallelUniqueId.h"
#include "MaterialData.h"
#include "MooseObject.h"

#define usingBlockRestrictableMembers using BlockRestrictable::getBlockCoordSystem

class FEProblemBase;
class MooseMesh;

class MooseVariableFieldBase;

/**
 * \class BlockRestrictable BlockRestrictable.h
 * \brief An interface that restricts an object to subdomains via the 'blocks' input parameter
 *
 * This class adds the 'blocks' input parameter and checks that it is populated, if it is not
 * populated
 * it will be populated with all valid block ids via two methods:
 * -# If a 'variable' parameter is present in the input parameters then by default the 'block'
 * input is set to all blocks associated with the variable.
 * -# If no 'variable' parameter is present then it utilizes all available blocks for the
 * associated mesh.
 *
 * When using with an object with a 'variable' parameter (e.g., Kernel), the following must also
 * exist within
 * the input parameters for the class to operate correctly
 * - '_fe_problem' = pointer to FEProblemBase
 * - '_tid' = THREAD_ID for the class
 * - '_sys' = pointer to the SystemBase
 *
 * In the general case (i.e., no 'variable') \b either \b one of the following must also exist
 * within the
 * input parameters for proper operation of the class:
 * - '_fe_problem' = a pointer to FEProblemBase
 * - '_mesh' = a pointer to MooseMesh
 *
 * When creating a new object, generally, this class should be inherited following MooseObject.
 * Also, the BlockRestricted::validParams() must be added to any other parameters for the
 * the class being created, since this is where the 'blocks' input parameter is created.
 *
 * \see Kernel
 * \see SideSetsAroundSubdomain
 */
class BlockRestrictable
{
public:
  /**
   * Class constructor
   * Populates the 'block' input parameters, see the general class documentation for details.
   * @param parameters The input parameters (see the detailed help for additional information)
   * @param initialize Disable initialization, MooseVariableBase was converted to a MooseObject,
   * this flag allows it to be constructed as if it wasn't to maintain backward compatibility, this
   * will be removed in the fugure.
   *                   TODO:MooseVariableToMooseObject (see #10601)
   */
  BlockRestrictable(const MooseObject * moose_object, bool initialize = true);

  static InputParameters validParams();

  /**
   * Class constructor
   * Populates the 'block' input parameters when an object is also boundary restricted,
   * see the general class documentation for details.
   * @param parameters The input parameters (see the detailed help for additional information)
   * @param boundary_ids The boundary ids that the object is restricted to
   */
  BlockRestrictable(const MooseObject * moose_object, const std::set<BoundaryID> & boundary_ids);

  /**
   * Destructor: does nothing but needs to be marked as virtual since
   * this class defines virtual functions.
   */
  virtual ~BlockRestrictable() {}

  /**
   * Return the block names for this object
   *
   * Note, if the 'blocks' input parameter was not utilized this will return an
   * empty vector.
   *
   * @return vector of SubdomainNames that are valid for this object
   */
  const std::vector<SubdomainName> & blocks() const;

  /**
   * Return the number of blocks for this object
   * @return The number of subdomains
   */
  unsigned int numBlocks() const;

  /**
   * Return the block subdomain ids for this object
   * @return a set of SudomainIDs that are valid for this object
   */
  const virtual std::set<SubdomainID> & blockIDs() const;

  /**
   * Test if the supplied block name is valid for this object
   * @param name A SubdomainName to check
   * @return True if the given id is valid for this object
   */
  bool hasBlocks(const SubdomainName & name) const;

  /**
   * Test if the supplied vector of block names are valid for this object
   * @param names A vector of SubdomainNames to check
   * @return True if the given ids are valid for this object
   */
  bool hasBlocks(const std::vector<SubdomainName> & names) const;

  /**
   * Test if the supplied block ids are valid for this object
   * @param id A SubdomainID to check
   * @return True if the given id is valid for this object
   */
  bool hasBlocks(SubdomainID id) const;

  /**
   * Test if the supplied vector block ids are valid for this object
   * @param ids A vector of SubdomainIDs ids to check
   * @return True if the all of the given ids are found within the ids for this object
   */
  bool hasBlocks(const std::vector<SubdomainID> & ids) const;

  /**
   * Test if the supplied set of block ids are valid for this object
   * @param ids A std::set of SubdomainIDs to check
   * @return True if the all of the given ids are found within the ids for this object
   * \see isSubset
   */
  bool hasBlocks(const std::set<SubdomainID> & ids) const;

  /**
   * Test if the class block ids are a subset of the supplied objects
   * @param ids A std::set of Subdomains to check
   * @return True if all of the block ids for this class are found within the given ids (opposite of
   * hasBlocks)
   * \see hasBlocks
   */
  bool isBlockSubset(const std::set<SubdomainID> & ids) const;

  /**
   * Test if the class block ids are a subset of the supplied objects
   * @param ids A std::vector of Subdomains to check
   * @return True if all of the block ids for this class are found within the given ids (opposite of
   * hasBlocks)
   * \see hasBlocks
   */
  bool isBlockSubset(const std::vector<SubdomainID> & ids) const;

  /**
   * Check if a material property is valid for all blocks of this object
   *
   * This method returns true if the supplied property name has been declared
   * in a Material object on the block ids for this object.
   *
   * @tparam T The type of material property
   * @param prop_name the name of the property to query
   * @return true if the property exists for all block ids of the object, otherwise false
   *
   * @see Material::hasBlockMaterialProperty
   */
  template <typename T, bool is_ad = false>
  bool hasBlockMaterialProperty(const std::string & prop_name);

  /**
   * Return all of the SubdomainIDs for the mesh
   * @return A set of all subdomians for the entire mesh
   */
  const std::set<SubdomainID> & meshBlockIDs() const;

  /**
   * Returns true if this object has been restricted to a block
   * @see MooseObject
   */
  virtual bool blockRestricted() const;

  /**
   * Helper for checking that the ids for this object are in agreement with the variables
   * on the supplied variable.
   *
   * @param variable The variable to check against.
   */
  virtual void checkVariable(const MooseVariableFieldBase & variable) const;

protected:
  /// Pointer to the MaterialData class for this object
  std::shared_ptr<MaterialData> _blk_material_data;

  /**
   * A helper method to allow the Material object to specialize the behavior
   * of hasBlockMaterialProperty. It also avoid circular \#include problems.
   * @see hasBlockMaterialProperty
   */
  virtual bool hasBlockMaterialPropertyHelper(const std::string & prop_name);

  /**
   * An initialization routine needed for dual constructors
   */
  void initializeBlockRestrictable(const MooseObject * moose_object);

  /**
   * Check if the blocks this object operates on all have the same coordinate system,
   * and if so return it.
   */
  Moose::CoordinateSystemType getBlockCoordSystem();

private:
  /// Set of block ids supplied by the user via the input file (for error checking)
  std::set<SubdomainID> _blk_ids;

  /// Vector of block ids supplied by the user via the input file (for error reporting)
  std::vector<SubdomainID> _vec_ids;

  /// Vector the block names supplied by the user via the input file
  std::vector<SubdomainName> _blocks;

  /// Flag for allowing dual restriction
  const bool _blk_dual_restrictable;

  /// Pointer to FEProblemBase
  FEProblemBase * _blk_feproblem;

  /// Pointer to Mesh
  MooseMesh * _blk_mesh;

  /// An empty set for referencing when boundary_ids is not included
  const std::set<BoundaryID> _empty_boundary_ids;

  /// Reference to the boundary_ids, defaults to an empty set if not provided
  const std::set<BoundaryID> & _boundary_ids;

  /// Thread id for this object
  THREAD_ID _blk_tid;

  /// Name of the object
  const std::string & _blk_name;
};

template <typename T, bool is_ad>
bool
BlockRestrictable::hasBlockMaterialProperty(const std::string & prop_name)
{
  mooseAssert(_blk_material_data != NULL, "MaterialData pointer is not defined");
  return hasBlockMaterialPropertyHelper(prop_name) &&
         _blk_material_data->haveGenericProperty<T, is_ad>(prop_name);
}
