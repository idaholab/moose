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

#include "MooseSystem.h"
#include "ElementPostprocessor.h"
#include "SidePostprocessor.h"

PostprocessorWarehouse::PostprocessorWarehouse()
{
}

PostprocessorWarehouse::~PostprocessorWarehouse()
{
  {
    std::map<unsigned int, std::vector<Postprocessor *> >::iterator j;
    for (j=_element_postprocessors.begin(); j!=_element_postprocessors.end(); ++j)
    {
      PostprocessorIterator k;
      for (k=j->second.begin(); k!=j->second.end(); ++k)
        delete *k;
    }
  }

  // delete side postprocessors
  {
    std::map<unsigned int, std::vector<Postprocessor *> >::iterator j;
    for (j=_side_postprocessors.begin(); j!=_side_postprocessors.end(); ++j)
    {
      PostprocessorIterator k;
      for (k=j->second.begin(); k!=j->second.end(); ++k)
        delete *k;
    }
  }

  // delete generic postprocessors
  for (std::vector<Postprocessor *>::iterator i=_generic_postprocessors.begin(); i!=_generic_postprocessors.end(); ++i)
    delete *i;
}

void
PostprocessorWarehouse::addPostprocessor(Postprocessor *postprocessor)
{
  if(dynamic_cast<ElementPostprocessor*>(postprocessor))
  {
    unsigned int block_id = dynamic_cast<ElementPostprocessor*>(postprocessor)->blockID();
    _element_postprocessors[block_id].push_back(postprocessor);
    _block_ids_with_postprocessors.insert(block_id);
  }
  else if(dynamic_cast<SidePostprocessor*>(postprocessor))
  {
    unsigned int boundary_id = dynamic_cast<SidePostprocessor*>(postprocessor)->boundaryID();
    _side_postprocessors[boundary_id].push_back(postprocessor);
    _boundary_ids_with_postprocessors.insert(boundary_id);
  }
  else
  {
    // FIXME: generic pps multithreaded
    _generic_postprocessors.push_back(postprocessor);
  }
}

PostprocessorIterator
PostprocessorWarehouse::elementPostprocessorsBegin(unsigned int block_id)
{
  return _element_postprocessors[block_id].begin();
}

PostprocessorIterator
PostprocessorWarehouse::elementPostprocessorsEnd(unsigned int block_id)
{
  return _element_postprocessors[block_id].end();
}

PostprocessorIterator
PostprocessorWarehouse::sidePostprocessorsBegin(unsigned int boundary_id)
{
  return _side_postprocessors[boundary_id].begin();
}

PostprocessorIterator
PostprocessorWarehouse::sidePostprocessorsEnd(unsigned int boundary_id)
{
  return _side_postprocessors[boundary_id].end();
}

PostprocessorIterator
PostprocessorWarehouse::genericPostprocessorsBegin()
{
  return _generic_postprocessors.begin();
}

PostprocessorIterator
PostprocessorWarehouse::genericPostprocessorsEnd()
{
  return _generic_postprocessors.end();
}
