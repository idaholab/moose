/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#ifndef BLOCKRESTRICTABLE_H
#define BLOCKRESTRICTABLE_H

// MOOSE includes
#include "InputParameters.h"
#include "ParallelUniqueId.h"
#include "MaterialData.h"
#include "MooseObject.h"

// Forward declarations
class BlockRestrictable;
class FEProblemBase;
class MooseMesh;

template <>
InputParameters validParams<BlockRestrictable>();

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
 * Also, the validParams<BlockRestricted>() must be added to any other parameters for the
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
   */
  BlockRestrictable(const InputParameters & parameters);

  /**
   * Class constructor
   * Populates the 'block' input parameters when an object is also boundary restricted,
   * see the general class documentation for details.
   * @param parameters The input parameters (see the detailed help for additional information)
   * @param boundary_ids The boundary ids that the object is restricted to
   */
  BlockRestrictable(const InputParameters & parameters, const std::set<BoundaryID> & boundary_ids);

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
  bool hasBlocks(SubdomainName name) const;

  /**
   * Test if the supplied vector of block names are valid for this object
   * @param names A vector of SubdomainNames to check
   * @return True if the given ids are valid for this object
   */
  bool hasBlocks(std::vector<SubdomainName> names) const;

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
  bool hasBlocks(std::vector<SubdomainID> ids) const;

  /**
   * Test if the supplied set of block ids are valid for this object
   * @param ids A std::set of SubdomainIDs to check
   * @return True if the all of the given ids are found within the ids for this object
   * \see isSubset
   */
  bool hasBlocks(std::set<SubdomainID> ids) const;

  /**
   * Test if the class block ids are a subset of the supplied objects
   * @param ids A std::set of Subdomains to check
   * @return True if all of the block ids for this class are found within the given ids (opposite of
   * hasBlocks)
   * \see hasBlocks
   */
  bool isBlockSubset(std::set<SubdomainID> ids) const;

  /**
   * Test if the class block ids are a subset of the supplied objects
   * @param ids A std::vector of Subdomains to check
   * @return True if all of the block ids for this class are found within the given ids (opposite of
   * hasBlocks)
   * \see hasBlocks
   */
  bool isBlockSubset(std::vector<SubdomainID> ids) const;

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
  template <typename T>
  bool hasBlockMaterialProperty(const std::string & prop_name);

  /**
   * Return all of the SubdomainIDs for the mesh
   * @return A set of all subdomians for the entire mesh
   */
  const std::set<SubdomainID> & meshBlockIDs() const;

  /**
   * Returns true if this object has been restricted to a boundary
   * @see MooseObject
   */
  virtual bool blockRestricted();

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
  void initializeBlockRestrictable(const InputParameters & parameters);

  /**
   * Check if the blocks this object operates on all have the same coordinate system,
   * and if so return it.
   */
  Moose::CoordinateSystemType getBlockCoordSystem();

private:
  /// Set of block ids supplied by the user via the input file
  std::set<SubdomainID> _blk_ids;

  /// Vector the block names supplied by the user via the input file
  std::vector<SubdomainName> _blocks;

  bool _initialized;

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

  /**
   * A helper function for extracting the subdomain IDs for a variable
   * @param parameters A reference to the input parameters supplied to the object
   */
  std::set<SubdomainID> variableSubdomainIDs(const InputParameters & parameters) const;
};

template <typename T>
bool
BlockRestrictable::hasBlockMaterialProperty(const std::string & prop_name)
{
  mooseAssert(_blk_material_data != NULL, "MaterialData pointer is not defined");
  return hasBlockMaterialPropertyHelper(prop_name) &&
         _blk_material_data->haveProperty<T>(prop_name);
}

#endif // BLOCKRESTRICTABLE_H
