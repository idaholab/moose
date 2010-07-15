#ifndef POSTPROCESSORWAREHOUSE_H
#define POSTPROCESSORWAREHOUSE_H

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
class PostprocessorWarehouse
{
public:
  PostprocessorWarehouse(MooseSystem &sys);
  virtual ~PostprocessorWarehouse();

  PostprocessorIterator elementPostprocessorsBegin(THREAD_ID tid);
  PostprocessorIterator elementPostprocessorsEnd(THREAD_ID tid);

  PostprocessorIterator sidePostprocessorsBegin(THREAD_ID tid, unsigned int boundary_id);
  PostprocessorIterator sidePostprocessorsEnd(THREAD_ID tid, unsigned int boundary_id);

  void addPostprocessor(THREAD_ID tid, Postprocessor *postprocessor);

  /**
   * All of the boundary ids that have postprocessors specified to act on them.
   */
  std::set<unsigned int> _boundary_ids_with_postprocessors;

protected:
  std::vector<std::vector<Postprocessor *> > _element_postprocessors;

  std::vector<std::map<unsigned int, std::vector<Postprocessor *> > > _side_postprocessors;

  MooseSystem &_moose_system;
};

#endif // POSTPROCESSORWAREHOUSE_H
