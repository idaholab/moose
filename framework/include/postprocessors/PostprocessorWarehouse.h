/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Orieneted Simulation Environment */
/*                                                              */
/*            @ 2010 Battelle Energy Alliance, LLC              */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

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
  PostprocessorWarehouse();
  virtual ~PostprocessorWarehouse();

  PostprocessorIterator elementPostprocessorsBegin();
  PostprocessorIterator elementPostprocessorsEnd();

  PostprocessorIterator sidePostprocessorsBegin(unsigned int boundary_id);
  PostprocessorIterator sidePostprocessorsEnd(unsigned int boundary_id);

  PostprocessorIterator genericPostprocessorsBegin();
  PostprocessorIterator genericPostprocessorsEnd();

  void addPostprocessor(Postprocessor *postprocessor);

  /**
   * All of the boundary ids that have postprocessors specified to act on them.
   */
  std::set<unsigned int> _boundary_ids_with_postprocessors;

protected:
  std::vector<Postprocessor *> _element_postprocessors;

  std::map<unsigned int, std::vector<Postprocessor *> > _side_postprocessors;

  std::vector<Postprocessor *> _generic_postprocessors;
};

#endif // POSTPROCESSORWAREHOUSE_H
