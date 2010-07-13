#include "PostprocessorHolder.h"

#include "MooseSystem.h"
#include "ElementPostprocessor.h"
#include "SidePostprocessor.h"

PostprocessorHolder::PostprocessorHolder(MooseSystem &sys) :
  _moose_system(sys)
{
  _element_postprocessors.resize(libMesh::n_threads());
  _side_postprocessors.resize(libMesh::n_threads());
}

PostprocessorHolder::~PostprocessorHolder()
{
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
}

void
PostprocessorHolder::addPostprocessor(THREAD_ID tid, Postprocessor *postprocessor)
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
    // TODO: Allow generic PPs
    mooseError("Unknown PP Type!");
  }
}

PostprocessorIterator
PostprocessorHolder::elementPostprocessorsBegin(THREAD_ID tid)
{
  return _element_postprocessors[tid].begin();
}

PostprocessorIterator
PostprocessorHolder::elementPostprocessorsEnd(THREAD_ID tid)
{
  return _element_postprocessors[tid].end();
}

PostprocessorIterator
PostprocessorHolder::sidePostprocessorsBegin(THREAD_ID tid, unsigned int boundary_id)
{
  return _side_postprocessors[tid][boundary_id].begin();
}

PostprocessorIterator
PostprocessorHolder::sidePostprocessorsEnd(THREAD_ID tid, unsigned int boundary_id)
{
  return _side_postprocessors[tid][boundary_id].end();
}
