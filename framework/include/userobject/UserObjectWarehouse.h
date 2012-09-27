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

#ifndef USER_OBJECTWAREHOUSE_H
#define USER_OBJECTWAREHOUSE_H

#include "UserObject.h"

#include <vector>
#include <map>
#include <set>

class ElementUserObject;
class NodalUserObject;
class SideUserObject;
class GeneralUserObject;

/**
 * Holds user_objects and provides some services
 */
class UserObjectWarehouse
{
public:

  enum GROUP
  {
    ALL,
    PRE_AUX,
    POST_AUX
  };

  UserObjectWarehouse();
  virtual ~UserObjectWarehouse();

  void updateDependObjects(const std::set<std::string> & depend_uo);

  // Setup /////
  void initialSetup();
  void timestepSetup();
  void residualSetup();
  void jacobianSetup();

  /**
   * Find out if a user object of a given name exists.
   * @param name The name of the user object
   * @return true if user object exists
   */
  bool hasUserObject(std::string name) { return _name_to_user_objects.find(name) != _name_to_user_objects.end(); }

  /**
   * Get a user object by its name.
   * @param name The name of the user object you want to get back.
   * @return A pointer pointing to the user object you are trying to get or NULL if its not found.
   */
  UserObject * getUserObjectByName(std::string name) { return _name_to_user_objects[name]; }

  /**
   * Get the list of all  elemental user_objects
   * @param block_id Block ID
   * @return The list of all elemental user_objects
   */
  const std::vector<ElementUserObject *> & elementUserObjects(SubdomainID block_id, GROUP group = ALL);

  /**
   * Get the list of side user_objects
   * @param boundary_id Boundary ID
   * @return The list of side user_objects
   */
  const std::vector<SideUserObject *> & sideUserObjects(BoundaryID boundary_id, GROUP group = ALL);

  /**
   * Get the list of nodal user_objects
   * @param boundary_id Boundary ID
   * @return The list of all nodal user_objects
   */
  const std::vector<NodalUserObject *> & nodalUserObjects(BoundaryID boundary_id, GROUP group = ALL);

  /**
   * Get the list of nodal user_objects restricted on the specified subdomain
   * @param subdomain_id Subdomain ID
   * @return The list of all block nodal user_objects
   */
  const std::vector<NodalUserObject *> & blockNodalUserObjects(SubdomainID subdomain_id, GROUP group = ALL);

  /**
   * Get the list general user_objects
   * @return The list of general user_objects
   */
  const std::vector<GeneralUserObject *> & genericUserObjects(GROUP group = ALL);

  /**
   * Get the list of all user_objects
   * @return The list of all user_objects
   */
  const std::vector<UserObject *> & all() { return _all_user_objects; }

  /**
   * Add a user_object
   * @param user_object UserObject being added
   */
  void addUserObject(UserObject *user_object);

  /**
   * Get the list of blocks with user_objects
   * @return The list of block IDs with user_objects
   */
  const std::set<SubdomainID> & blocks() { return _block_ids_with_user_objects; }

  /**
   * Get the list of boundary IDs with user_objects
   * @return The list of boundary IDs with user_objects
   */
  const std::set<BoundaryID> & boundaryIds() { return _boundary_ids_with_user_objects; }

  /**
   * Get the list of nodeset IDs with user_objects
   * @return The list of nodeset IDs with user_objects
   */
  const std::set<BoundaryID> & nodesetIds() { return _nodeset_ids_with_user_objects; }

  /**
   * Get the list of subdomain IDs with *nodal* user_objects
   * @return The list of subdomain IDs with user_objects
   */
  const std::set<SubdomainID> & blockNodalIds() { return _block_ids_with_nodal_user_objects; }

protected:
  std::map<std::string, UserObject *> _name_to_user_objects;

  std::vector<ElementUserObject *> _all_element_user_objects;
  std::vector<NodalUserObject *> _all_nodal_user_objects;
  std::vector<SideUserObject *> _all_side_user_objects;
  std::vector<GeneralUserObject *> _all_generic_user_objects;
  std::vector<UserObject *> _all_user_objects;

  std::map<SubdomainID, std::vector<ElementUserObject *> > _element_user_objects;
  std::map<BoundaryID, std::vector<SideUserObject *> > _side_user_objects;
  std::map<BoundaryID, std::vector<NodalUserObject *> > _nodal_user_objects;
  // Block restricted nodal user objects
  std::map<SubdomainID, std::vector<NodalUserObject *> > _block_nodal_user_objects;
  std::vector<GeneralUserObject *> _generic_user_objects;

  // PreAux UO
  std::map<SubdomainID, std::vector<ElementUserObject *> > _pre_element_user_objects;
  std::map<BoundaryID, std::vector<SideUserObject *> > _pre_side_user_objects;
  std::map<BoundaryID, std::vector<NodalUserObject *> > _pre_nodal_user_objects;
  std::map<SubdomainID, std::vector<NodalUserObject *> > _pre_block_nodal_user_objects;
  std::vector<GeneralUserObject *> _pre_generic_user_objects;

  // PostAux UO
  std::map<SubdomainID, std::vector<ElementUserObject *> > _post_element_user_objects;
  std::map<BoundaryID, std::vector<SideUserObject *> > _post_side_user_objects;
  std::map<BoundaryID, std::vector<NodalUserObject *> > _post_nodal_user_objects;
  std::map<SubdomainID, std::vector<NodalUserObject *> > _post_block_nodal_user_objects;
  std::vector<GeneralUserObject *> _post_generic_user_objects;

  /// All of the block ids that have user_objects specified to act on them
  std::set<SubdomainID> _block_ids_with_user_objects;
  /// All of the boundary ids that have user_objects specified to act on them
  std::set<BoundaryID> _boundary_ids_with_user_objects;
  /// All of the nodeset ids that have user_objects specified to act on them
  std::set<BoundaryID> _nodeset_ids_with_user_objects;
  /// All of the block ids that have nodal user_objects specified to act on them
  std::set<SubdomainID> _block_ids_with_nodal_user_objects;
};

#endif // USER_OBJECTWAREHOUSE_H
