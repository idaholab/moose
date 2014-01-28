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

#ifndef BOUNDARYRESTRICTABLE_H
#define BOUNDARYRESTRICTABLE_H

#include "InputParameters.h"
#include "MooseTypes.h"
#include "FEProblem.h"
#include "MooseMesh.h"

class BoundaryRestrictable;

template<>
InputParameters validParams<BoundaryRestrictable>();

/**
 * /class BoundaryRestrictable
 * /brief Provides functionality for limiting the object to certain boundary ids
 * The is class the inheriting class with methods useful for limiting an object
 * to certain boundaries. The parameters "_boundary_id" and "boundary", which are
 * created with validParams<BoundaryRestrictable> are used the framework.
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
   */
  BoundaryRestrictable(const std::string name, InputParameters & parameters);

  /**
   * Empty class destructor
   */
  virtual ~BoundaryRestrictable();

  /**
   * Return the boundary IDs for this object
   * @return A set of all boundary ids for which the object is restricted
   */
  const std::set<BoundaryID> & boundaryIDs() const;

  /**
   * Return the boundary names for this object
   * @return A set of all boundary names for which the object is restricted
   */
  const std::vector<BoundaryName> & boundaryNames();

  /**
   * Return the number of boundaries for this object
   * @return The number of subdomains
   */
  unsigned int numBoundaryIDs();

  /**
   * Test if the supplied boundary name is valid for this object
c   * @param name A BoundaryName to check
   * @return True if the given id is valid for this object
   */
  bool hasBoundary(BoundaryName name);

  /**
   * Test if the supplied vector of boundary names are valid for this object
   * @param names A vector of BoundaryNames to check
   * @return True if the given ids are valid for this object
   */
  bool hasBoundary(std::vector<BoundaryName> names);

  /**
   * Test if the supplied boundary ids are valid for this object
   * @param id A BoundaryID to check
   * @return True if the given id is valid for this object
   */
  bool hasBoundary(BoundaryID id);

  /**
   * Test if the supplied vector boundary ids are valid for this object
   * @param ids A vector of BoundaryIDs ids to check
   * @param type A flag for the type of matching to perform: ALL requires that all supplied
   * ids must match those of the object; ANY requires that any one of the supplied ids must
   * match those of the object
   * @return True if the all of the given ids are found within the ids for this object
   */
   bool hasBoundary(std::vector<BoundaryID> ids, TEST_TYPE type=ALL);

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
  bool hasBoundary(std::set<BoundaryID> ids, TEST_TYPE type=ALL);

  /**
   * Test if the class boundary ids are a subset of the supplied objects
   * @param ids A std::set of boundaries to check
   * @return True if all of the boundary ids for this class are found within the given ids (opposite of hasBoundary)
   * \see hasBoundary
   */
  bool isBoundarySubset(std::set<BoundaryID> ids);

  /*
   * Test if the class boundary ids are a subset of the supplied objects
   * @param ids A std::set of Boundary IDs to check
   * @return True if all of the boundary ids for this class are found within the given ids (opposite of hasBoundary)
   * \see hasBoundary
   */
  bool isBoundarySubset(std::vector<BoundaryID> ids);

  /**
   * Check if a material property is valid for all boundaries of this object
   *
   * This method returns true if the boundary ids for this object are a subset of the boundaries
   * associated with the material property for the supplied property name
   *
   * @param name the name of the property to query
   * @return true if the property exists for all block ids of the object, otherwise false
   * \see MaterialPropertyInterface::getMaterialPropertyBoundaryIDs
   * \see isBoundarySubet
   */
  template<typename T>
  bool hasBoundaryMaterialProperty(const std::string & name);

private:

  /// Pointer to FEProblem
  FEProblem * _bnd_feproblem;

  /// Point to mesh
  MooseMesh * _bnd_mesh;

  /// Set of the boundary ids
  std::set<BoundaryID> _bnd_ids;

  /// Vector the the boundary names
  std::vector<BoundaryName> _boundary_names;

  /// Flag for allowing dual restriction with BlockRestrictable
  const bool _bnd_dual_restrictable;

  /// Invalid BoundaryID for case when FEProblem
  const BoundaryID _invalid_boundary_id;

protected:

  /// Reference to active boundary id
  const BoundaryID & _current_boundary_id;

};

template<typename T>
bool
BoundaryRestrictable::hasBoundaryMaterialProperty(const std::string & name)
{
  // Return true if the boundaries for this object are a subset of the boundaries for the material
  return isBoundarySubset(_bnd_feproblem->getMaterialPropertyBoundaryIDs(name));
}

#endif // BOUNDARYRESTRICTABLE_H
