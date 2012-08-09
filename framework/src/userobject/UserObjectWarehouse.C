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

  for(std::vector<GeneralUserObject *>::const_iterator i=_all_generic_user_objects.begin();
      i!=_all_generic_user_objects.end();
      ++i)
    (*i)->jacobianSetup();
}


void
UserObjectWarehouse::addUserObject(UserObject *user_object)
{
  _all_user_objects.push_back(user_object);
  _name_to_user_objects[user_object->name()] = user_object;

  if(dynamic_cast<ElementUserObject*>(user_object))
  {
    ElementUserObject * elem_pp = dynamic_cast<ElementUserObject*>(user_object);
    MooseMesh &mesh = elem_pp->getSubProblem().mesh();
    const std::vector<SubdomainName> & blocks = elem_pp->blocks();
    for (std::vector<SubdomainName>::const_iterator it = blocks.begin(); it != blocks.end(); ++it)
    {
      SubdomainID block_id;
      // Switch the Any Block Id string to a number type here
      if (*it == "ANY_BLOCK_ID")
        block_id = Moose::ANY_BLOCK_ID;
      else
        block_id = mesh.getSubdomainID(*it);
      _element_user_objects[block_id].push_back(elem_pp);
      _all_element_user_objects.push_back(elem_pp);
      _block_ids_with_user_objects.insert(block_id);
    }
  }
  else if(dynamic_cast<SideUserObject*>(user_object))
  {
    SideUserObject * side_pp = dynamic_cast<SideUserObject*>(user_object);
    MooseMesh &mesh = side_pp->getSubProblem().mesh();

    const std::vector<BoundaryName> & bnds = side_pp->boundaries();
    for (std::vector<BoundaryName>::const_iterator it = bnds.begin(); it != bnds.end(); ++it)
    {
      BoundaryID boundary_id = mesh.getBoundaryID(*it);

      _side_user_objects[boundary_id].push_back(side_pp);
      _all_side_user_objects.push_back(side_pp);
      _boundary_ids_with_user_objects.insert(boundary_id);
    }
  }
  else if(dynamic_cast<NodalUserObject*>(user_object))
  {
    NodalUserObject * nodal_pp = dynamic_cast<NodalUserObject*>(user_object);
    MooseMesh &mesh = nodal_pp->getSubProblem().mesh();

    const std::vector<BoundaryName> & bnds = nodal_pp->boundaries();
    for (std::vector<BoundaryName>::const_iterator it = bnds.begin(); it != bnds.end(); ++it)
    {
      BoundaryID boundary_id;

      if (*it == "ANY_BOUNDARY_ID")
        boundary_id = Moose::ANY_BOUNDARY_ID;
      else
        boundary_id = mesh.getBoundaryID(*it);
      _nodal_user_objects[boundary_id].push_back(nodal_pp);
      _all_nodal_user_objects.push_back(nodal_pp);
      _nodeset_ids_with_user_objects.insert(boundary_id);
    }
  }
  else
  {
    GeneralUserObject * general_pp = dynamic_cast<GeneralUserObject*>(user_object);

    // FIXME: generic pps multithreaded
    _generic_user_objects.push_back(general_pp);
    _all_generic_user_objects.push_back(general_pp);
  }
}
