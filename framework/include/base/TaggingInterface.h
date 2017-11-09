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

protected:
  /// The vectors this Kernel will contribute to
  std::vector<TagID> _vector_tags;

  /// The matrices this Kernel will contribute to
  std::vector<TagID> _matrix_tags;

  SubProblem & _subproblem;

  const MooseObject & _moose_object;
};

#endif /* TAGGINGINTERFACE_H */
