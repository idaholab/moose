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

#include "PostprocessorWarehouse.h"
#include "Postprocessor.h"
#include "ElementPostprocessor.h"
#include "InternalSidePostprocessor.h"
#include "SidePostprocessor.h"
#include "NodalPostprocessor.h"
#include "GeneralPostprocessor.h"
#include "MooseMesh.h"
#include "SubProblem.h"
#include "Parser.h"

PostprocessorWarehouse::PostprocessorWarehouse()
{
}

PostprocessorWarehouse::~PostprocessorWarehouse()
{
  // We don't need to free because that's taken care of by the UserObjectWarehouse
//  for (std::vector<Postprocessor *>::iterator i=_all_postprocessors.begin(); i!=_all_postprocessors.end(); ++i)
//    delete *i;
}

void
PostprocessorWarehouse::initialSetup()
{
  for(std::vector<ElementPostprocessor *>::const_iterator i=_all_element_postprocessors.begin();
      i!=_all_element_postprocessors.end();
      ++i)
    (*i)->initialSetup();

  for(std::vector<NodalPostprocessor *>::const_iterator i=_all_nodal_postprocessors.begin();
      i!=_all_nodal_postprocessors.end();
      ++i)
    (*i)->initialSetup();

  for(std::vector<SidePostprocessor *>::const_iterator i=_all_side_postprocessors.begin();
      i!=_all_side_postprocessors.end();
      ++i)
    (*i)->initialSetup();

  for(std::vector<InternalSidePostprocessor *>::const_iterator i=_all_internal_side_postprocessors.begin();
      i!=_all_internal_side_postprocessors.end();
      ++i)
    (*i)->initialSetup();


  for(std::vector<GeneralPostprocessor *>::const_iterator i=_all_generic_postprocessors.begin();
      i!=_all_generic_postprocessors.end();
      ++i)
    (*i)->initialSetup();
}

void
PostprocessorWarehouse::timestepSetup()
{
  for(std::vector<ElementPostprocessor *>::const_iterator i=_all_element_postprocessors.begin();
      i!=_all_element_postprocessors.end();
      ++i)
    (*i)->timestepSetup();

  for(std::vector<NodalPostprocessor *>::const_iterator i=_all_nodal_postprocessors.begin();
      i!=_all_nodal_postprocessors.end();
      ++i)
    (*i)->timestepSetup();

  for(std::vector<SidePostprocessor *>::const_iterator i=_all_side_postprocessors.begin();
      i!=_all_side_postprocessors.end();
      ++i)
    (*i)->timestepSetup();

  for(std::vector<InternalSidePostprocessor *>::const_iterator i=_all_internal_side_postprocessors.begin();
      i!=_all_internal_side_postprocessors.end();
      ++i)
    (*i)->timestepSetup();


  for(std::vector<GeneralPostprocessor *>::const_iterator i=_all_generic_postprocessors.begin();
      i!=_all_generic_postprocessors.end();
      ++i)
    (*i)->timestepSetup();
}

void
PostprocessorWarehouse::residualSetup()
{
  for(std::vector<ElementPostprocessor *>::const_iterator i=_all_element_postprocessors.begin();
      i!=_all_element_postprocessors.end();
      ++i)
    (*i)->residualSetup();

  for(std::vector<NodalPostprocessor *>::const_iterator i=_all_nodal_postprocessors.begin();
      i!=_all_nodal_postprocessors.end();
      ++i)
    (*i)->residualSetup();

  for(std::vector<SidePostprocessor *>::const_iterator i=_all_side_postprocessors.begin();
      i!=_all_side_postprocessors.end();
      ++i)
    (*i)->residualSetup();

  for(std::vector<InternalSidePostprocessor *>::const_iterator i=_all_internal_side_postprocessors.begin();
      i!=_all_internal_side_postprocessors.end();
      ++i)
    (*i)->residualSetup();

  for(std::vector<GeneralPostprocessor *>::const_iterator i=_all_generic_postprocessors.begin();
      i!=_all_generic_postprocessors.end();
      ++i)
    (*i)->residualSetup();
}

void
PostprocessorWarehouse::jacobianSetup()
{
  for(std::vector<ElementPostprocessor *>::const_iterator i=_all_element_postprocessors.begin();
      i!=_all_element_postprocessors.end();
      ++i)
    (*i)->jacobianSetup();

  for(std::vector<NodalPostprocessor *>::const_iterator i=_all_nodal_postprocessors.begin();
      i!=_all_nodal_postprocessors.end();
      ++i)
    (*i)->jacobianSetup();

  for(std::vector<SidePostprocessor *>::const_iterator i=_all_side_postprocessors.begin();
      i!=_all_side_postprocessors.end();
      ++i)
    (*i)->jacobianSetup();

  for(std::vector<InternalSidePostprocessor *>::const_iterator i=_all_internal_side_postprocessors.begin();
      i!=_all_internal_side_postprocessors.end();
      ++i)
    (*i)->jacobianSetup();


  for(std::vector<GeneralPostprocessor *>::const_iterator i=_all_generic_postprocessors.begin();
      i!=_all_generic_postprocessors.end();
      ++i)
    (*i)->jacobianSetup();
}


void
PostprocessorWarehouse::addPostprocessor(Postprocessor *postprocessor)
{
  _all_postprocessors.push_back(postprocessor);

  if (dynamic_cast<ElementPostprocessor*>(postprocessor))
  {
    ElementPostprocessor * elem_pp = dynamic_cast<ElementPostprocessor*>(postprocessor);
    const std::set<SubdomainID> & block_ids = dynamic_cast<ElementPostprocessor*>(elem_pp)->blockIDs();
    _all_element_postprocessors.push_back(elem_pp);
    for (std::set<SubdomainID>::const_iterator it = block_ids.begin(); it != block_ids.end(); ++it)
    {
      _element_postprocessors[*it].push_back(elem_pp);
      _block_ids_with_postprocessors.insert(*it);
    }
  }

  else if (dynamic_cast<SidePostprocessor*>(postprocessor))
  {
    SidePostprocessor * side_pp = dynamic_cast<SidePostprocessor*>(postprocessor);
    _all_side_postprocessors.push_back(side_pp);

    const std::set<BoundaryID> & bnds = dynamic_cast<SidePostprocessor*>(side_pp)->boundaryIDs();
    for (std::set<BoundaryID>::const_iterator it = bnds.begin(); it != bnds.end(); ++it)
    {
      _side_postprocessors[*it].push_back(side_pp);
      _boundary_ids_with_postprocessors.insert(*it);
    }
  }

  else if (dynamic_cast<InternalSidePostprocessor*>(postprocessor))
  {
    InternalSidePostprocessor * internal_side_pp = dynamic_cast<InternalSidePostprocessor*>(postprocessor);
    _all_internal_side_postprocessors.push_back(internal_side_pp);

    const std::set<SubdomainID> & blks = dynamic_cast<InternalSidePostprocessor*>(internal_side_pp)->blockIDs();
    for (std::set<SubdomainID>::const_iterator it = blks.begin(); it != blks.end(); ++it)
    {
      _internal_side_postprocessors[*it].push_back(internal_side_pp);
      _block_ids_with_postprocessors.insert(*it);
    }
  }

  else if (dynamic_cast<NodalPostprocessor*>(postprocessor))
  {
    NodalPostprocessor * nodal_pp = dynamic_cast<NodalPostprocessor*>(postprocessor);

    // NodalPostprocessors can be "block" restricted and/or "boundary" restricted
    const std::set<BoundaryID> & bnds = nodal_pp->boundaryIDs();
    const std::set<SubdomainID> & blks = nodal_pp->blockIDs();

    // Add to the storage of all postprocessors
    _all_nodal_postprocessors.push_back(nodal_pp);

    // If the Block IDs are not empty, then the postprocessor is block restricted
    if (blks.find(Moose::ANY_BLOCK_ID) == blks.end())
      for (std::set<SubdomainID>::const_iterator it = blks.begin(); it != blks.end(); ++it)
      {
        _block_nodal_postprocessors[*it].push_back(nodal_pp);
        _block_ids_with_nodal_postprocessors.insert(*it);
      }

    // Otherwise the postprocessor is a boundary postprocessor
    else
      for (std::set<BoundaryID>::const_iterator it = bnds.begin(); it != bnds.end(); ++it)
      {
        _nodal_postprocessors[*it].push_back(nodal_pp);
        _nodeset_ids_with_postprocessors.insert(*it);
      }

  }

  else
  {
    GeneralPostprocessor * general_pp = dynamic_cast<GeneralPostprocessor*>(postprocessor);

    // FIXME: generic pps multithreaded
    _generic_postprocessors.push_back(general_pp);
    _all_generic_postprocessors.push_back(general_pp);
  }
}

Postprocessor *
PostprocessorWarehouse::getPostprocessor(std::string name)
{
  // Loop through all the postprocessors, return the pointer if the names match
  for (std::vector<Postprocessor *>::iterator it=_all_postprocessors.begin(); it != _all_postprocessors.end(); ++it)
  {
    if (name.compare((*it)->PPName()) == 0)
      return *it;
  }

  // Return a null if nothing was found
  return NULL;
}
