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

  for(std::vector<GeneralPostprocessor *>::const_iterator i=_all_generic_postprocessors.begin();
      i!=_all_generic_postprocessors.end();
      ++i)
    (*i)->jacobianSetup();
}


void
PostprocessorWarehouse::addPostprocessor(Postprocessor *postprocessor)
{
  _all_postprocessors.push_back(postprocessor);

  if(dynamic_cast<ElementPostprocessor*>(postprocessor))
  {
    ElementPostprocessor * elem_pp = dynamic_cast<ElementPostprocessor*>(postprocessor);
    MooseMesh &mesh = elem_pp->getSubProblem().mesh();
    const std::vector<SubdomainName> & blocks = dynamic_cast<ElementPostprocessor*>(elem_pp)->blocks();
    for (std::vector<SubdomainName>::const_iterator it = blocks.begin(); it != blocks.end(); ++it)
    {
      SubdomainID block_id;
      // Switch the Any Block Id string to a number type here
      if (*it == "ANY_BLOCK_ID")
        block_id = Moose::ANY_BLOCK_ID;
      else
        block_id = mesh.getSubdomainID(*it);
      _element_postprocessors[block_id].push_back(elem_pp);
      _all_element_postprocessors.push_back(elem_pp);
      _block_ids_with_postprocessors.insert(block_id);
    }
  }
  else if(dynamic_cast<SidePostprocessor*>(postprocessor))
  {
    SidePostprocessor * side_pp = dynamic_cast<SidePostprocessor*>(postprocessor);
    MooseMesh &mesh = side_pp->getSubProblem().mesh();

    const std::vector<BoundaryName> & bnds = dynamic_cast<SidePostprocessor*>(side_pp)->boundaries();
    for (std::vector<BoundaryName>::const_iterator it = bnds.begin(); it != bnds.end(); ++it)
    {
      BoundaryID boundary_id = mesh.getBoundaryID(*it);

      _side_postprocessors[boundary_id].push_back(side_pp);
      _all_side_postprocessors.push_back(side_pp);
      _boundary_ids_with_postprocessors.insert(boundary_id);
    }
  }
  else if(dynamic_cast<NodalPostprocessor*>(postprocessor))
  {
    NodalPostprocessor * nodal_pp = dynamic_cast<NodalPostprocessor*>(postprocessor);
    MooseMesh &mesh = nodal_pp->getSubProblem().mesh();

    // NodalPostprocessors can be "block" restricted or "boundary" restricted
    const std::vector<BoundaryName> & bnds = nodal_pp->boundaries();
    const std::vector<SubdomainName> & blocks = nodal_pp->blocks();

    if (blocks.size() == 0 || blocks[0] == "ANY_BLOCK_ID")
      for (std::vector<BoundaryName>::const_iterator it = bnds.begin(); it != bnds.end(); ++it)
      {
        BoundaryID boundary_id;

        if (*it == "ANY_BOUNDARY_ID")
          boundary_id = Moose::ANY_BOUNDARY_ID;
        else
          boundary_id = mesh.getBoundaryID(*it);
        _nodal_postprocessors[boundary_id].push_back(nodal_pp);
        _all_nodal_postprocessors.push_back(nodal_pp);
        _nodeset_ids_with_postprocessors.insert(boundary_id);
      }
    else
      for (std::vector<SubdomainName>::const_iterator it = blocks.begin(); it != blocks.end(); ++it)
      {
        SubdomainID block_id;

        if (*it == "ANY_BLOCK_ID")
          block_id = Moose::ANY_BLOCK_ID;
        else
          block_id = mesh.getSubdomainID(*it);
        _block_nodal_postprocessors[block_id].push_back(nodal_pp);
        _all_nodal_postprocessors.push_back(nodal_pp);
        _block_ids_with_nodal_postprocessors.insert(block_id);
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
