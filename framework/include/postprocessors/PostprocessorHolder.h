#ifndef POSTPROCESSORHOLDER_H
#define POSTPROCESSORHOLDER_H

#include "Postprocessor.h"

#include <vector>
#include <map>
#include <set>

/**
 * Typedef to hide implementation details
 */
typedef std::vector<Postprocessor *>::iterator PostprocessorIterator;


/**
 * Holds postprocessors and provides some services
 */
class PostprocessorHolder
{
public:
  PostprocessorHolder(MooseSystem &sys);
  virtual ~PostprocessorHolder();

  PostprocessorIterator elementPostprocessorsBegin(THREAD_ID tid);
  PostprocessorIterator elementPostprocessorsEnd(THREAD_ID tid);

  PostprocessorIterator sidePostprocessorsBegin(THREAD_ID tid, unsigned int boundary_id);
  PostprocessorIterator sidePostprocessorsEnd(THREAD_ID tid, unsigned int boundary_id);

  void addPostprocessor(THREAD_ID tid, Postprocessor *postprocessor);

protected:
  std::vector<std::vector<Postprocessor *> > _element_postprocessors;

  std::vector<std::map<unsigned int, std::vector<Postprocessor *> > > _side_postprocessors;

  MooseSystem &_moose_system;
};

#endif // POSTPROCESSORHOLDER_H
