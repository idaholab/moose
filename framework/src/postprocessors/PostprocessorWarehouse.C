#include "PostprocessorWarehouse.h"

#include "MooseSystem.h"
#include "ElementPostprocessor.h"
#include "SidePostprocessor.h"

PostprocessorWarehouse::PostprocessorWarehouse(MooseSystem &sys) :
  _moose_system(sys)
{
  _element_postprocessors.resize(libMesh::n_threads());
  _side_postprocessors.resize(libMesh::n_threads());
}

PostprocessorWarehouse::~PostprocessorWarehouse()
{
  std::vector<std::map<unsigned int, std::vector<Postprocessor *> > > _side_postprocessors;

  std::vector<Postprocessor *> _generic_postprocessors;
  // delete element postprocessors
  {
    std::vector<std::vector<Postprocessor *> >::iterator i;
    for (i=_element_postprocessors.begin(); i!=_element_postprocessors.end(); ++i)
    {
      PostprocessorIterator j;
      for (j=i->begin(); j!=i->end(); ++j)
      {
        delete *j;
      }
    }
  }

  // delete side postprocessors
  {
    std::vector<std::map<unsigned int, std::vector<Postprocessor *> > >::iterator i;
    for (i=_side_postprocessors.begin(); i!=_side_postprocessors.end(); ++i)
    {
      std::map<unsigned int, std::vector<Postprocessor *> >::iterator j;
      for (j=i->begin(); j!=i->end(); ++j)
      {
        PostprocessorIterator k;
        for (k=j->second.begin(); k!=j->second.end(); ++k)
          delete *k;
      }
    }
  }

  // delete generic postprocessors
  {
    std::vector<Postprocessor *>::iterator i;
    for (i=_generic_postprocessors.begin(); i!=_generic_postprocessors.end(); ++i)
      delete *i;
  }
}

void
PostprocessorWarehouse::addPostprocessor(THREAD_ID tid, Postprocessor *postprocessor)
{
  if(dynamic_cast<ElementPostprocessor*>(postprocessor))
    _element_postprocessors[tid].push_back(postprocessor);
  else if(dynamic_cast<SidePostprocessor*>(postprocessor))
  {
    unsigned int boundary_id = dynamic_cast<SidePostprocessor*>(postprocessor)->boundaryID();
    _side_postprocessors[tid][boundary_id].push_back(postprocessor);
    _boundary_ids_with_postprocessors.insert(boundary_id);
  }
  else
  {
    //Generic postprocessors aren't multithreaded, so only add one copy of each one
    if (tid == 0)
      _generic_postprocessors.push_back(postprocessor);
  }
}

PostprocessorIterator
PostprocessorWarehouse::elementPostprocessorsBegin(THREAD_ID tid)
{
  return _element_postprocessors[tid].begin();
}

PostprocessorIterator
PostprocessorWarehouse::elementPostprocessorsEnd(THREAD_ID tid)
{
  return _element_postprocessors[tid].end();
}

PostprocessorIterator
PostprocessorWarehouse::sidePostprocessorsBegin(THREAD_ID tid, unsigned int boundary_id)
{
  return _side_postprocessors[tid][boundary_id].begin();
}

PostprocessorIterator
PostprocessorWarehouse::sidePostprocessorsEnd(THREAD_ID tid, unsigned int boundary_id)
{
  return _side_postprocessors[tid][boundary_id].end();
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
