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

#include "UserObjectWarehouse.h"
#include "ElementUserObject.h"
#include "SideUserObject.h"
#include "InternalSideUserObject.h"
#include "NodalUserObject.h"
#include "GeneralUserObject.h"
#include "MooseMesh.h"
#include "SubProblem.h"
#include "Parser.h"

UserObjectWarehouse::UserObjectWarehouse()
{
}

UserObjectWarehouse::~UserObjectWarehouse()
{
  for (std::vector<UserObject *>::iterator i=_all_user_objects.begin(); i!=_all_user_objects.end(); ++i)
    delete *i;
}

void
UserObjectWarehouse::updateDependObjects(const std::set<std::string> & depend_uo)
{
  // Bin the user objects into either Pre or Post AuxKernel bins
  for (std::map<SubdomainID, std::vector<ElementUserObject *> >::iterator it1 = _block_element_user_objects.begin(); it1 != _block_element_user_objects.end(); ++it1)
  {
    _pre_element_user_objects[it1->first].clear();
    _post_element_user_objects[it1->first].clear();
    for (std::vector<ElementUserObject *>::iterator it2 = it1->second.begin(); it2 != it1->second.end(); ++it2)
    {
      if (depend_uo.find((*it2)->name()) != depend_uo.end())
        _pre_element_user_objects[it1->first].push_back(*it2);
      else
        _post_element_user_objects[it1->first].push_back(*it2);
    }
  }

  for (std::map<BoundaryID, std::vector<SideUserObject *> >::iterator it1 = _boundary_side_user_objects.begin(); it1 != _boundary_side_user_objects.end(); ++it1)
  {
    _pre_side_user_objects[it1->first].clear();
    _post_side_user_objects[it1->first].clear();
    for (std::vector<SideUserObject *>::iterator it2 = it1->second.begin(); it2 != it1->second.end(); ++it2)
    {
      if (depend_uo.find((*it2)->name()) != depend_uo.end())
        _pre_side_user_objects[it1->first].push_back(*it2);
      else
        _post_side_user_objects[it1->first].push_back(*it2);
    }
  }

  _pre_internal_side_user_objects.clear();
  _post_internal_side_user_objects.clear();
  for (std::map<SubdomainID, std::vector<InternalSideUserObject *> >::iterator it1 = _block_internal_side_user_objects.begin(); it1 != _block_internal_side_user_objects.end(); ++it1)
  {
    _pre_internal_side_user_objects[it1->first].clear();
    _post_internal_side_user_objects[it1->first].clear();
    for (std::vector<InternalSideUserObject *>::iterator it2 = it1->second.begin(); it2 != it1->second.end(); ++it2)
    {
      if (depend_uo.find((*it2)->name()) != depend_uo.end())
        _pre_internal_side_user_objects[it1->first].push_back(*it2);
      else
        _post_internal_side_user_objects[it1->first].push_back(*it2);
    }
  }


  for (std::map<BoundaryID, std::vector<NodalUserObject *> >::iterator it1 = _boundary_nodal_user_objects.begin(); it1 != _boundary_nodal_user_objects.end(); ++it1)
  {
    _pre_nodal_user_objects[it1->first].clear();
    _post_nodal_user_objects[it1->first].clear();
    for (std::vector<NodalUserObject *>::iterator it2 = it1->second.begin(); it2 != it1->second.end(); ++it2)
    {
      if (depend_uo.find((*it2)->name()) != depend_uo.end())
        _pre_nodal_user_objects[it1->first].push_back(*it2);
      else
        _post_nodal_user_objects[it1->first].push_back(*it2);
    }
  }

  for (std::map<SubdomainID, std::vector<NodalUserObject *> >::iterator it1 = _block_nodal_user_objects.begin(); it1 != _block_nodal_user_objects.end(); ++it1)
  {
    _pre_block_nodal_user_objects[it1->first].clear();
    _post_block_nodal_user_objects[it1->first].clear();
    for (std::vector<NodalUserObject *>::iterator it2 = it1->second.begin(); it2 != it1->second.end(); ++it2)
    {
      if (depend_uo.find((*it2)->name()) != depend_uo.end())
        _pre_block_nodal_user_objects[it1->first].push_back(*it2);
      else
        _post_block_nodal_user_objects[it1->first].push_back(*it2);
    }
  }

  _pre_generic_user_objects.clear();
  _post_generic_user_objects.clear();
  for (std::vector<GeneralUserObject *>::iterator it2 = _all_generic_user_objects.begin(); it2 != _all_generic_user_objects.end(); ++it2)
  {
    if (depend_uo.find((*it2)->name()) != depend_uo.end())
      _pre_generic_user_objects.push_back(*it2);
    else
      _post_generic_user_objects.push_back(*it2);
  }
}

void
UserObjectWarehouse::initialSetup()
{
  for(std::vector<ElementUserObject *>::const_iterator i=_all_element_user_objects.begin();
      i!=_all_element_user_objects.end();
      ++i)
    (*i)->initialSetup();

  for(std::vector<NodalUserObject *>::const_iterator i=_all_nodal_user_objects.begin();
      i!=_all_nodal_user_objects.end();
      ++i)
    (*i)->initialSetup();

  for(std::vector<SideUserObject *>::const_iterator i=_all_side_user_objects.begin();
      i!=_all_side_user_objects.end();
      ++i)
    (*i)->initialSetup();

  for(std::vector<InternalSideUserObject *>::const_iterator i=_all_internal_side_user_objects.begin();
      i!=_all_internal_side_user_objects.end();
      ++i)
    (*i)->initialSetup();

  for(std::vector<GeneralUserObject *>::const_iterator i=_all_generic_user_objects.begin();
      i!=_all_generic_user_objects.end();
      ++i)
    (*i)->initialSetup();
}

void
UserObjectWarehouse::timestepSetup()
{
  for(std::vector<ElementUserObject *>::const_iterator i=_all_element_user_objects.begin();
      i!=_all_element_user_objects.end();
      ++i)
    (*i)->timestepSetup();

  for(std::vector<NodalUserObject *>::const_iterator i=_all_nodal_user_objects.begin();
      i!=_all_nodal_user_objects.end();
      ++i)
    (*i)->timestepSetup();

  for(std::vector<SideUserObject *>::const_iterator i=_all_side_user_objects.begin();
      i!=_all_side_user_objects.end();
      ++i)
    (*i)->timestepSetup();

  for(std::vector<InternalSideUserObject *>::const_iterator i=_all_internal_side_user_objects.begin();
      i!=_all_internal_side_user_objects.end();
      ++i)
    (*i)->timestepSetup();

  for(std::vector<GeneralUserObject *>::const_iterator i=_all_generic_user_objects.begin();
      i!=_all_generic_user_objects.end();
      ++i)
    (*i)->timestepSetup();
}

void
UserObjectWarehouse::residualSetup()
{
  for(std::vector<ElementUserObject *>::const_iterator i=_all_element_user_objects.begin();
      i!=_all_element_user_objects.end();
      ++i)
    (*i)->residualSetup();

  for(std::vector<NodalUserObject *>::const_iterator i=_all_nodal_user_objects.begin();
      i!=_all_nodal_user_objects.end();
      ++i)
    (*i)->residualSetup();

  for(std::vector<SideUserObject *>::const_iterator i=_all_side_user_objects.begin();
      i!=_all_side_user_objects.end();
      ++i)
    (*i)->residualSetup();

  for(std::vector<InternalSideUserObject *>::const_iterator i=_all_internal_side_user_objects.begin();
      i!=_all_internal_side_user_objects.end();
      ++i)
    (*i)->residualSetup();

  for(std::vector<GeneralUserObject *>::const_iterator i=_all_generic_user_objects.begin();
      i!=_all_generic_user_objects.end();
      ++i)
    (*i)->residualSetup();
}

void
UserObjectWarehouse::jacobianSetup()
{
  for(std::vector<ElementUserObject *>::const_iterator i=_all_element_user_objects.begin();
      i!=_all_element_user_objects.end();
      ++i)
    (*i)->jacobianSetup();

  for(std::vector<NodalUserObject *>::const_iterator i=_all_nodal_user_objects.begin();
      i!=_all_nodal_user_objects.end();
      ++i)
    (*i)->jacobianSetup();

  for(std::vector<SideUserObject *>::const_iterator i=_all_side_user_objects.begin();
      i!=_all_side_user_objects.end();
      ++i)
    (*i)->jacobianSetup();

  for(std::vector<InternalSideUserObject *>::const_iterator i=_all_internal_side_user_objects.begin();
      i!=_all_internal_side_user_objects.end();
      ++i)
    (*i)->jacobianSetup();

  for(std::vector<GeneralUserObject *>::const_iterator i=_all_generic_user_objects.begin();
      i!=_all_generic_user_objects.end();
      ++i)
    (*i)->jacobianSetup();
}


void
UserObjectWarehouse::addUserObject(UserObject *user_object)
{
  // Add the object and its name to the lists of all objects
  _all_user_objects.push_back(user_object);
  _name_to_user_objects[user_object->name()] = user_object;

  // Add an ElementUserObject
  if (dynamic_cast<ElementUserObject*>(user_object))
  {
    // Extract the BlockIDs (see BlockRestrictable)
    ElementUserObject * element_uo = dynamic_cast<ElementUserObject*>(user_object);
    const std::set<SubdomainID> & blks = element_uo->blockIDs();

    // Add to the list of all SideUserObjects
    _all_element_user_objects.push_back(element_uo);

    // Loop through each of the block ids and update the various storage lists
    for (std::set<SubdomainID>::const_iterator it = blks.begin(); it != blks.end(); ++it)
    {
      _block_element_user_objects[*it].push_back(element_uo);
      _block_ids_with_user_objects.insert(*it);
    }
  }

  // Add a SideUserObject
  else if (dynamic_cast<SideUserObject*>(user_object))
  {
    // Extract the BoundaryIDs (see BoundaryRestrictable)
    SideUserObject * side_uo = dynamic_cast<SideUserObject*>(user_object);
    const std::set<BoundaryID> & bnds = side_uo->boundaryIDs();

    // Add to the list of all SideUserObjects
    _all_side_user_objects.push_back(side_uo);

    // Loop through each of the block ids and update the various storage lists
    for (std::set<BoundaryID>::const_iterator it = bnds.begin(); it != bnds.end(); ++it)
    {
      _boundary_side_user_objects[*it].push_back(side_uo);
      _boundary_ids_with_user_objects.insert(*it);
    }
  }

  // Add an InternalSideUserObject
  else if (dynamic_cast<InternalSideUserObject*>(user_object))
  {
    // Extract the BlockIDs (see BlockRestrictable)
    InternalSideUserObject * element_uo = dynamic_cast<InternalSideUserObject*>(user_object);
    const std::set<SubdomainID> & blks = element_uo->blockIDs();

    // Add to the list of all SideUserObjects
    _all_internal_side_user_objects.push_back(element_uo);

    // Loop through each of the block ids and update the various storage lists
    for (std::set<SubdomainID>::const_iterator it = blks.begin(); it != blks.end(); ++it)
    {
      _block_internal_side_user_objects[*it].push_back(element_uo);
      _block_ids_with_user_objects.insert(*it);
    }
  }

  // Add a NodalUserObject
  else if (dynamic_cast<NodalUserObject*>(user_object))
  {
    // Extract the Boundary and Block Ids (see BoundaryRestrictable and BlockRestrictable)
    NodalUserObject * nodal_uo = dynamic_cast<NodalUserObject*>(user_object);
    const std::set<BoundaryID> & bnds = nodal_uo->boundaryIDs();
    const std::set<SubdomainID> & blks = nodal_uo->blockIDs();

    // Update the list of all NodalUserObjects
    _all_nodal_user_objects.push_back(nodal_uo);

    // If the Block IDs does not contain ANY_BLOCK_ID, then the object is block restricted
    if (blks.find(Moose::ANY_BLOCK_ID) == blks.end())
      for (std::set<SubdomainID>::const_iterator it = blks.begin(); it != blks.end(); ++it)
      {
        _block_nodal_user_objects[*it].push_back(nodal_uo);
        _block_ids_with_nodal_user_objects.insert(*it);
      }

    // Otherwise the objects is boundary restricted
    else
      for (std::set<BoundaryID>::const_iterator it = bnds.begin(); it != bnds.end(); ++it)
      {
        _boundary_nodal_user_objects[*it].push_back(nodal_uo);
        _nodeset_ids_with_user_objects.insert(*it);
      }
  }

  // Add a GeneralUserObject
  else
  {
    GeneralUserObject * general_uo = dynamic_cast<GeneralUserObject*>(user_object);

    // FIXME: generic pps multithreaded
    _all_generic_user_objects.push_back(general_uo);
  }
}

const std::vector<ElementUserObject *> &
UserObjectWarehouse::elementUserObjects(SubdomainID block_id, GROUP group)
{
  switch(group)
  {
  case ALL:
    return _block_element_user_objects[block_id];
  case PRE_AUX:
    return _pre_element_user_objects[block_id];
  case POST_AUX:
    return _post_element_user_objects[block_id];
  default:
    mooseError("Bad Enum");
  }
}

const std::vector<SideUserObject *> &
UserObjectWarehouse::sideUserObjects(BoundaryID boundary_id, GROUP group)
{
  switch(group)
  {
  case ALL:
    return _boundary_side_user_objects[boundary_id];
  case PRE_AUX:
    return _pre_side_user_objects[boundary_id];
  case POST_AUX:
    return _post_side_user_objects[boundary_id];
  default:
    mooseError("Bad value for Enum GROUP, must be ALL, PRE_AUX, or POST_AUX");
  }
}

const std::vector<InternalSideUserObject *> &
UserObjectWarehouse::internalSideUserObjects(SubdomainID block_id, GROUP group)
{
  switch (group)
  {
  case ALL:
    return _block_internal_side_user_objects[block_id];
  case PRE_AUX:
    return _pre_internal_side_user_objects[block_id];
  case POST_AUX:
    return _post_internal_side_user_objects[block_id];
  default:
    mooseError("Bad value for Enum GROUP, must be ALL, PRE_AUX, or POST_AUX");

  }
}

const std::vector<NodalUserObject *> &
UserObjectWarehouse::nodalUserObjects(BoundaryID boundary_id, GROUP group)
{
  switch(group)
  {
  case ALL:
    return _boundary_nodal_user_objects[boundary_id];
  case PRE_AUX:
    return _pre_nodal_user_objects[boundary_id];
  case POST_AUX:
    return _post_nodal_user_objects[boundary_id];
  default:
    mooseError("Bad value for Enum GROUP, must be ALL, PRE_AUX, or POST_AUX");
  }
}

const std::vector<NodalUserObject *> &
UserObjectWarehouse::blockNodalUserObjects(SubdomainID subdomain_id, GROUP group)
{
  switch(group)
  {
  case ALL:
    return _block_nodal_user_objects[subdomain_id];
  case PRE_AUX:
    return _pre_block_nodal_user_objects[subdomain_id];
  case POST_AUX:
    return _post_block_nodal_user_objects[subdomain_id];
  default:
    mooseError("Bad value for Enum GROUP, must be ALL, PRE_AUX, or POST_AUX");
  }
}

const std::vector<GeneralUserObject *> &
UserObjectWarehouse::genericUserObjects(GROUP group)
{
  switch(group)
  {
  case ALL:
    return _all_generic_user_objects;
  case PRE_AUX:
    return _pre_generic_user_objects;
  case POST_AUX:
    return _post_generic_user_objects;
  default:
    mooseError("Bad value for Enum GROUP, must be ALL, PRE_AUX, or POST_AUX");
  }
}
