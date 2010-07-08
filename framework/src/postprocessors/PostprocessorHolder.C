#include "PostprocessorHolder.h"
#include "MooseSystem.h"

PostprocessorHolder::PostprocessorHolder(MooseSystem &sys) :
  _moose_system(sys)
{
  _element_postprocessors.resize(libMesh::n_threads());
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
  _element_postprocessors[tid].push_back(postprocessor);
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
