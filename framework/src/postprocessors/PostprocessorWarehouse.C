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
#include "ElementPostprocessor.h"
#include "SidePostprocessor.h"
#include "NodalPostprocessor.h"
#include "MooseMesh.h"
#include "SubProblem.h"
#include "Parser.h"

PostprocessorWarehouse::PostprocessorWarehouse()
{
}

PostprocessorWarehouse::~PostprocessorWarehouse()
{
  for (std::vector<Postprocessor *>::iterator i=_all_postprocessors.begin(); i!=_all_postprocessors.end(); ++i)
    delete *i;
}

void
PostprocessorWarehouse::initialSetup()
{
  for(std::vector<Postprocessor *>::const_iterator i=all().begin(); i!=all().end(); ++i)
    (*i)->initialSetup();
}

void
PostprocessorWarehouse::timestepSetup()
{
  for(std::vector<Postprocessor *>::const_iterator i=all().begin(); i!=all().end(); ++i)
    (*i)->timestepSetup();
}

void
PostprocessorWarehouse::residualSetup()
{
  for(std::vector<Postprocessor *>::const_iterator i=all().begin(); i!=all().end(); ++i)
    (*i)->residualSetup();
}

void
PostprocessorWarehouse::jacobianSetup()
{
  for(std::vector<Postprocessor *>::const_iterator i=all().begin(); i!=all().end(); ++i)
    (*i)->jacobianSetup();
}


void
PostprocessorWarehouse::addPostprocessor(Postprocessor *postprocessor)
{
  _all_postprocessors.push_back(postprocessor);
  MooseMesh &mesh = postprocessor->getSubProblem().mesh();

  if(dynamic_cast<ElementPostprocessor*>(postprocessor))
  {
    const std::vector<SubdomainName> & blocks = dynamic_cast<ElementPostprocessor*>(postprocessor)->blocks();
    for (std::vector<std::string>::const_iterator it = blocks.begin(); it != blocks.end(); ++it)
    {
      SubdomainID block_id;
      // Switch the Any Block Id string to a number type here
      if (*it == "ANY_BLOCK_ID")
        block_id = Moose::ANY_BLOCK_ID;
      else
        block_id = mesh.getSubdomainID(*it);
      _element_postprocessors[block_id].push_back(postprocessor);
      _block_ids_with_postprocessors.insert(block_id);
    }
  }
  else if(dynamic_cast<SidePostprocessor*>(postprocessor))
  {
    const std::vector<BoundaryName> & bnds = dynamic_cast<SidePostprocessor*>(postprocessor)->boundaries();
    for (std::vector<BoundaryName>::const_iterator it = bnds.begin(); it != bnds.end(); ++it)
    {
      BoundaryID boundary_id = mesh.getBoundaryID(*it);

      _side_postprocessors[boundary_id].push_back(postprocessor);
      _boundary_ids_with_postprocessors.insert(boundary_id);
    }
  }
  else if(dynamic_cast<NodalPostprocessor*>(postprocessor))
  {
    // FIXME: nodal pps multithreaded
    _nodal_postprocessors.push_back(postprocessor);
  }
  else
  {
    // FIXME: generic pps multithreaded
    _generic_postprocessors.push_back(postprocessor);
  }
}
