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

  if(dynamic_cast<ElementPostprocessor*>(postprocessor))
  {
    const std::vector<unsigned int> & block_ids = dynamic_cast<ElementPostprocessor*>(postprocessor)->blockIDs();
    for (std::vector<unsigned int>::const_iterator it = block_ids.begin(); it != block_ids.end(); ++it)
    {
      unsigned int block_id = *it;
      _element_postprocessors[block_id].push_back(postprocessor);
      _block_ids_with_postprocessors.insert(block_id);
    }
  }
  else if(dynamic_cast<SidePostprocessor*>(postprocessor))
  {
    const std::vector<unsigned int> & bnd_ids = dynamic_cast<SidePostprocessor*>(postprocessor)->boundaryIDs();
    for (std::vector<unsigned int>::const_iterator it = bnd_ids.begin(); it != bnd_ids.end(); ++it)
    {
      unsigned int boundary_id = *it;
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
