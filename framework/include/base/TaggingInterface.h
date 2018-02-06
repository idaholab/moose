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

#ifndef TAGGINGINTERFACE_H
#define TAGGINGINTERFACE_H

#include "MooseTypes.h"
#include "MultiMooseEnum.h"

// Forward declarations
class InputParameters;
class MooseObject;
class TaggingInterface;
class SubProblem;

template <typename T>
InputParameters validParams();

template <>
InputParameters validParams<TaggingInterface>();

class TaggingInterface
{
public:
  TaggingInterface(SubProblem & subproblem, const MooseObject & moose_object);
  virtual ~TaggingInterface();

  void addVectorTag(TagName & tag_name);

  void addMatrixTag(TagName & tag_name);

  void addVectorTag(TagID tag_id);

  void addMatrixTag(TagID tag_id);

  bool isVectorTagged() { return _vector_tags.size() > 0; }

  bool isMatrixTagged() { return _matrix_tags.size() > 0; }

  std::set<TagID> & getVectorTags() { return _vector_tags; }

  std::set<TagID> & getMatrixTags() { return _matrix_tags; }

protected:
  /// The vectors this Kernel will contribute to
  std::set<TagID> _vector_tags;

  /// The matrices this Kernel will contribute to
  std::set<TagID> _matrix_tags;

  SubProblem & _subproblem;

  const MooseObject & _moose_object;
};

#endif /* TAGGINGINTERFACE_H */
