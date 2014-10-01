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

#include "VectorPostprocessorWarehouse.h"
#include "VectorPostprocessor.h"
#include "ElementVectorPostprocessor.h"
#include "InternalSideVectorPostprocessor.h"
#include "SideVectorPostprocessor.h"
#include "NodalVectorPostprocessor.h"
#include "GeneralVectorPostprocessor.h"
#include "MooseMesh.h"
#include "SubProblem.h"
#include "Parser.h"

VectorPostprocessorWarehouse::VectorPostprocessorWarehouse() :
    Warehouse<VectorPostprocessor>()
{
}

VectorPostprocessorWarehouse::~VectorPostprocessorWarehouse()
{
}

void
VectorPostprocessorWarehouse::initialSetup()
{
  for (std::vector<ElementVectorPostprocessor *>::const_iterator i=_all_element_VectorPostprocessors.begin();
      i!=_all_element_VectorPostprocessors.end();
      ++i)
    (*i)->initialSetup();

  for (std::vector<NodalVectorPostprocessor *>::const_iterator i=_all_nodal_VectorPostprocessors.begin();
      i!=_all_nodal_VectorPostprocessors.end();
      ++i)
    (*i)->initialSetup();

  for (std::vector<SideVectorPostprocessor *>::const_iterator i=_all_side_VectorPostprocessors.begin();
      i!=_all_side_VectorPostprocessors.end();
      ++i)
    (*i)->initialSetup();

  for (std::vector<InternalSideVectorPostprocessor *>::const_iterator i=_all_internal_side_VectorPostprocessors.begin();
      i!=_all_internal_side_VectorPostprocessors.end();
      ++i)
    (*i)->initialSetup();


  for (std::vector<GeneralVectorPostprocessor *>::const_iterator i=_all_generic_VectorPostprocessors.begin();
      i!=_all_generic_VectorPostprocessors.end();
      ++i)
    (*i)->initialSetup();
}

void
VectorPostprocessorWarehouse::timestepSetup()
{
  for (std::vector<ElementVectorPostprocessor *>::const_iterator i=_all_element_VectorPostprocessors.begin();
      i!=_all_element_VectorPostprocessors.end();
      ++i)
    (*i)->timestepSetup();

  for (std::vector<NodalVectorPostprocessor *>::const_iterator i=_all_nodal_VectorPostprocessors.begin();
      i!=_all_nodal_VectorPostprocessors.end();
      ++i)
    (*i)->timestepSetup();

  for (std::vector<SideVectorPostprocessor *>::const_iterator i=_all_side_VectorPostprocessors.begin();
      i!=_all_side_VectorPostprocessors.end();
      ++i)
    (*i)->timestepSetup();

  for (std::vector<InternalSideVectorPostprocessor *>::const_iterator i=_all_internal_side_VectorPostprocessors.begin();
      i!=_all_internal_side_VectorPostprocessors.end();
      ++i)
    (*i)->timestepSetup();


  for (std::vector<GeneralVectorPostprocessor *>::const_iterator i=_all_generic_VectorPostprocessors.begin();
      i!=_all_generic_VectorPostprocessors.end();
      ++i)
    (*i)->timestepSetup();
}

void
VectorPostprocessorWarehouse::residualSetup()
{
  for (std::vector<ElementVectorPostprocessor *>::const_iterator i=_all_element_VectorPostprocessors.begin();
      i!=_all_element_VectorPostprocessors.end();
      ++i)
    (*i)->residualSetup();

  for (std::vector<NodalVectorPostprocessor *>::const_iterator i=_all_nodal_VectorPostprocessors.begin();
      i!=_all_nodal_VectorPostprocessors.end();
      ++i)
    (*i)->residualSetup();

  for (std::vector<SideVectorPostprocessor *>::const_iterator i=_all_side_VectorPostprocessors.begin();
      i!=_all_side_VectorPostprocessors.end();
      ++i)
    (*i)->residualSetup();

  for (std::vector<InternalSideVectorPostprocessor *>::const_iterator i=_all_internal_side_VectorPostprocessors.begin();
      i!=_all_internal_side_VectorPostprocessors.end();
      ++i)
    (*i)->residualSetup();

  for (std::vector<GeneralVectorPostprocessor *>::const_iterator i=_all_generic_VectorPostprocessors.begin();
      i!=_all_generic_VectorPostprocessors.end();
      ++i)
    (*i)->residualSetup();
}

void
VectorPostprocessorWarehouse::jacobianSetup()
{
  for (std::vector<ElementVectorPostprocessor *>::const_iterator i=_all_element_VectorPostprocessors.begin();
      i!=_all_element_VectorPostprocessors.end();
      ++i)
    (*i)->jacobianSetup();

  for (std::vector<NodalVectorPostprocessor *>::const_iterator i=_all_nodal_VectorPostprocessors.begin();
      i!=_all_nodal_VectorPostprocessors.end();
      ++i)
    (*i)->jacobianSetup();

  for (std::vector<SideVectorPostprocessor *>::const_iterator i=_all_side_VectorPostprocessors.begin();
      i!=_all_side_VectorPostprocessors.end();
      ++i)
    (*i)->jacobianSetup();

  for (std::vector<InternalSideVectorPostprocessor *>::const_iterator i=_all_internal_side_VectorPostprocessors.begin();
      i!=_all_internal_side_VectorPostprocessors.end();
      ++i)
    (*i)->jacobianSetup();


  for (std::vector<GeneralVectorPostprocessor *>::const_iterator i=_all_generic_VectorPostprocessors.begin();
      i!=_all_generic_VectorPostprocessors.end();
      ++i)
    (*i)->jacobianSetup();
}

const std::vector<ElementVectorPostprocessor *> &
VectorPostprocessorWarehouse::elementVectorPostprocessors(SubdomainID block_id) const
{
  std::map<SubdomainID, std::vector<ElementVectorPostprocessor *> >::const_iterator it = _element_VectorPostprocessors.find(block_id);

  if (it == _element_VectorPostprocessors.end())
    mooseError("No element vector postprocessors on block: " << block_id);

  return it->second;
}

const std::vector<SideVectorPostprocessor *> &
VectorPostprocessorWarehouse::sideVectorPostprocessors(BoundaryID boundary_id) const
{
  std::map<BoundaryID, std::vector<SideVectorPostprocessor *> >::const_iterator it = _side_VectorPostprocessors.find(boundary_id);

  if (it == _side_VectorPostprocessors.end())
    mooseError("No side vector postprocessors on boundary: " << boundary_id);

  return it->second;
}

const std::vector<NodalVectorPostprocessor *> &
VectorPostprocessorWarehouse::nodalVectorPostprocessors(BoundaryID boundary_id) const
{
  std::map<BoundaryID, std::vector<NodalVectorPostprocessor *> >::const_iterator it = _nodal_VectorPostprocessors.find(boundary_id);

  if (it == _nodal_VectorPostprocessors.end())
    mooseError("No nodal vector postprocessors on boundary: " << boundary_id);

  return it->second;
}

const std::vector<NodalVectorPostprocessor *> &
VectorPostprocessorWarehouse::blockNodalVectorPostprocessors(SubdomainID block_id) const
{
  std::map<SubdomainID, std::vector<NodalVectorPostprocessor *> >::const_iterator it = _block_nodal_VectorPostprocessors.find(block_id);

  if (it == _block_nodal_VectorPostprocessors.end())
    mooseError("No block nodal vector postprocessors on block: " << block_id);

  return it->second;
}

void
VectorPostprocessorWarehouse::addVectorPostprocessor(MooseSharedPointer<VectorPostprocessor> vector_postprocessor)
{
  _all_ptrs.push_back(vector_postprocessor);

  VectorPostprocessor * raw_ptr = vector_postprocessor.get();

  _all_objects.push_back(raw_ptr);

  if (dynamic_cast<ElementVectorPostprocessor*>(raw_ptr))
  {
    ElementVectorPostprocessor * elem_pp = dynamic_cast<ElementVectorPostprocessor *>(raw_ptr);
    const std::set<SubdomainID> & block_ids = dynamic_cast<ElementVectorPostprocessor*>(elem_pp)->blockIDs();
    _all_element_VectorPostprocessors.push_back(elem_pp);
    for (std::set<SubdomainID>::const_iterator it = block_ids.begin(); it != block_ids.end(); ++it)
    {
      _element_VectorPostprocessors[*it].push_back(elem_pp);
      _block_ids_with_VectorPostprocessors.insert(*it);
    }
  }
  else if (dynamic_cast<SideVectorPostprocessor*>(raw_ptr))
  {
    SideVectorPostprocessor * side_pp = dynamic_cast<SideVectorPostprocessor *>(raw_ptr);
    _all_side_VectorPostprocessors.push_back(side_pp);

    const std::set<BoundaryID> & bnds = dynamic_cast<SideVectorPostprocessor *>(side_pp)->boundaryIDs();
    for (std::set<BoundaryID>::const_iterator it = bnds.begin(); it != bnds.end(); ++it)
    {
      _side_VectorPostprocessors[*it].push_back(side_pp);
      _boundary_ids_with_VectorPostprocessors.insert(*it);
    }
  }
  else if (dynamic_cast<InternalSideVectorPostprocessor*>(raw_ptr))
  {
    InternalSideVectorPostprocessor * internal_side_pp = dynamic_cast<InternalSideVectorPostprocessor *>(raw_ptr);
    _all_internal_side_VectorPostprocessors.push_back(internal_side_pp);

    const std::set<SubdomainID> & blks = dynamic_cast<InternalSideVectorPostprocessor *>(internal_side_pp)->blockIDs();
    for (std::set<SubdomainID>::const_iterator it = blks.begin(); it != blks.end(); ++it)
    {
      _internal_side_VectorPostprocessors[*it].push_back(internal_side_pp);
      _block_ids_with_VectorPostprocessors.insert(*it);
    }
  }
  else if (dynamic_cast<NodalVectorPostprocessor*>(raw_ptr))
  {
    NodalVectorPostprocessor * nodal_pp = dynamic_cast<NodalVectorPostprocessor *>(raw_ptr);

    // NodalVectorPostprocessors can be "block" restricted and/or "boundary" restricted
    const std::set<BoundaryID> & bnds = nodal_pp->boundaryIDs();
    const std::set<SubdomainID> & blks = nodal_pp->blockIDs();

    // Add to the storage of all VectorPostprocessors
    _all_nodal_VectorPostprocessors.push_back(nodal_pp);

    // If the Block IDs are not empty, then the VectorPostprocessor is block restricted
    if (blks.find(Moose::ANY_BLOCK_ID) == blks.end())
      for (std::set<SubdomainID>::const_iterator it = blks.begin(); it != blks.end(); ++it)
      {
        _block_nodal_VectorPostprocessors[*it].push_back(nodal_pp);
        _block_ids_with_nodal_VectorPostprocessors.insert(*it);
      }

    // Otherwise the VectorPostprocessor is a boundary VectorPostprocessor
    else
      for (std::set<BoundaryID>::const_iterator it = bnds.begin(); it != bnds.end(); ++it)
      {
        _nodal_VectorPostprocessors[*it].push_back(nodal_pp);
        _nodeset_ids_with_VectorPostprocessors.insert(*it);
      }

  }
  else if (dynamic_cast<GeneralVectorPostprocessor*>(raw_ptr))
  {
    GeneralVectorPostprocessor * general_pp = dynamic_cast<GeneralVectorPostprocessor *>(raw_ptr);

    // FIXME: generic pps multithreaded
    _generic_VectorPostprocessors.push_back(general_pp);
    _all_generic_VectorPostprocessors.push_back(general_pp);
  }
  else
    mooseError("Unknown type of VectorPostprocessor!");
}

VectorPostprocessor *
VectorPostprocessorWarehouse::getVectorPostprocessor(std::string name)
{
  // Loop through all the VectorPostprocessors, return the pointer if the names match
  for (std::vector<VectorPostprocessor *>::iterator it=_all_objects.begin(); it != _all_objects.end(); ++it)
  {
    if (name.compare((*it)->PPName()) == 0)
      return *it;
  }

  // Return a null if nothing was found
  return NULL;
}
