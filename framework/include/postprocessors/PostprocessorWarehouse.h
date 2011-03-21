#ifndef POSTPROCESSORWAREHOUSE_H_
#define POSTPROCESSORWAREHOUSE_H_

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
  PostprocessorWarehouse();
  virtual ~PostprocessorWarehouse();

  PostprocessorIterator elementPostprocessorsBegin(unsigned int block_id);
  PostprocessorIterator elementPostprocessorsEnd(unsigned int block_id);

  PostprocessorIterator sidePostprocessorsBegin(unsigned int boundary_id);
  PostprocessorIterator sidePostprocessorsEnd(unsigned int boundary_id);

  PostprocessorIterator genericPostprocessorsBegin();
  PostprocessorIterator genericPostprocessorsEnd();

  PostprocessorIterator allPostprocessorsBegin();
  PostprocessorIterator allPostprocessorsEnd();


  void addPostprocessor(Postprocessor *postprocessor);

  /**
   * All of the block ids that have postprocessors specified to act on them.
   */
  std::set<unsigned int> _block_ids_with_postprocessors;

  /**
   * All of the boundary ids that have postprocessors specified to act on them.
   */
  std::set<unsigned int> _boundary_ids_with_postprocessors;

protected:
  std::map<unsigned int, std::vector<Postprocessor *> > _element_postprocessors;
  std::map<unsigned int, std::vector<Postprocessor *> > _side_postprocessors;

  std::vector<Postprocessor *> _generic_postprocessors;
  std::vector<Postprocessor *> _all_postprocessors;
};

#endif // POSTPROCESSORWAREHOUSE_H_
