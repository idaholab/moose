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

#include "InputParameters.h"
#include "MooseTypes.h"
#include "FEProblem.h"

class BlockRestrictable;

template<>
InputParameters validParams<BlockRestrictable>();

/**
 * \class BlockRestrictable BlockRestrictable.h
 * \brief An interface that restricts an object to subdomains via the 'blocks' input parameter
 *
 * This class adds the 'blocks' input parameter and checks that it is populated, if it is not populated
 * it will be populated with all valid block ids via two methods:
 * -# If a 'variable' parameter is present in the input parameters then by default the 'block'
 * input is set to all blocks associated with the variable.
 * -# If no 'variable' parameter is present then it utilizes all available blocks for the
 * associated mesh.
 *
 * When using with an object with a 'variable' parameter (e.g., Kernel), the following must also exist within
 * the input parameters for the class to operate correctly
 * - '_fe_problem' = pointer to FEProblem
 * - '_tid' = THREAD_ID for the class
 * - '_sys' = pointer to the SystemBase
 *
 * In the general case (i.e., no 'variable') \b either \b one of the following must also exist within the
 * input parameters for proper operation of the class:
 * - '_fe_problem' = a pointer to FEProblem
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
   * @param name The name of the class
   * @param parameters The input parameters (see the detailed help for additional information)
   */
  BlockRestrictable(const std::string name, InputParameters & params);

  /**
   * Return the block names for this object
   *
   * Note, if the 'blocks' input parameter was not utilized this will return an
   * empty vector.
   *
   * @return vector of SubdomainNames that are valid for this object
   */
  const std::vector<SubdomainName> & blocks();

  /**
   * Return the number of blocks for this object
   * @return The number of subdomains
   */
  unsigned int numBlocks();

  /**
   * Return the block subdomain ids for this object
   * @return a set of SudomainIDs that are valid for this object
   */
  const std::set<SubdomainID> & blockIDs() const;

  /**
   * Test if the supplied block name is valid for this object
   * @param name A SubdomainName to check
   * @return True if the given id is valid for this object
   */
  bool hasBlocks(SubdomainName name);

  /**
   * Test if the supplied vector of block names are valid for this object
   * @param names A vector of SubdomainNames to check
   * @return True if the given ids are valid for this object
   */
  bool hasBlocks(std::vector<SubdomainName> names);

  /**
   * Test if the supplied block ids are valid for this object
   * @param id A SubdomainID to check
   * @return True if the given id is valid for this object
   */
  bool hasBlocks(SubdomainID id);

  /**
   * Test if the supplied vector block ids are valid for this object
   * @param ids A vector of SubdomainIDs ids to check
   * @return True if the all of the given ids are found within the ids for this object
   */
  bool hasBlocks(std::vector<SubdomainID> ids);

  /**
   * Test if the supplied set of block ids are valid for this object
   * @param ids A std::set of SubdomainIDs to check
   * @return True if the all of the given ids are found within the ids for this object
   * \see isSubset
   */
  bool hasBlocks(std::set<SubdomainID> ids);

  /**
   * Test if the class block ids are a subset of the supplied objects
   * @param ids A std::set of Subdomains to check
   * @return True if all of the block ids for this class are found within the given ids (opposite of hasBlocks)
   * \see hasBlocks
   */
  bool isBlockSubset(std::set<SubdomainID> ids);

  /**
   * Test if the class block ids are a subset of the supplied objects
   * @param ids A std::vector of Subdomains to check
   * @return True if all of the block ids for this class are found within the given ids (opposite of hasBlocks)
   * \see hasBlocks
   */
  bool isBlockSubset(std::vector<SubdomainID> ids);

  /**
   * Check if a material property is valid for all blocks of this object
   *
   * This method returns true if the block ids for this object are a subset of the blocks
   * associated with the material property for the supplied property name. This function
   * will return false it the getMaterialPropertyBlocks for the same name is empty.
   *
   * @param name the name of the property to query
   * @return true if the property exists for all block ids of the object, otherwise false
   * \see MaterialPropertyInterface::getMaterialPropertyBlocks
   * \see isBlockSubet
   */
  bool hasBlockMaterialProperty(const std::string & name);

private:

  /// Set of block ids
  std::set<SubdomainID> _blk_ids;

  /// Vector the block names
  std::vector<SubdomainName> _blocks;

  /// Flag for allowing dual restriction
  const bool _blk_dual_restrictable;

  /// Pointer to FEProblem
  FEProblem * _blk_feproblem;

  /// Pointer to Mesh
  MooseMesh * _blk_mesh;

  /**
   * A helper function for extracting the subdomain IDs for a variable
   * @param paramters A reference to the input parameters supplied to the object
   */
  std::set<SubdomainID> variableSubdomianIDs(InputParameters & parameters);

};


#endif // BLOCKRESTRICTABLE_H
