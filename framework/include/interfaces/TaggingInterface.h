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
#include "Assembly.h"

#include "libmesh/dense_vector.h"
#include "libmesh/dense_matrix.h"

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

  void useVectorTag(TagName & tag_name);

  void useMatrixTag(TagName & tag_name);

  void useVectorTag(TagID tag_id);

  void useMatrixTag(TagID tag_id);

  bool isVectorTagged() { return _vector_tags.size() > 0; }

  bool isMatrixTagged() { return _matrix_tags.size() > 0; }

  std::set<TagID> & getVectorTags() { return _vector_tags; }

  std::set<TagID> & getMatrixTags() { return _matrix_tags; }

  /**
   * Prepare data for computing element residual
   * according to ative tags.
   * Residual blocks for different tags will be extracted from Assembly.
   * A local residual variable will be zeroed
   */
  void prepareVectorTag(Assembly & assembly, unsigned int ivar);

  /**
   * Prepare data for computing element jacobian
   * according to ative tags.
   * Jacobian blocks for different tags will be extracted from Assembly.
   * A local Jacobian variable will be zeroed
   */
  void prepareMatrixTag(Assembly & assembly, unsigned int ivar, unsigned int jvar);

  /**
   * Local residual blocks  will be appended by adding
   * the current local kernel residual
   */
  void accumulateTaggedLocalResidual();

  /**
   * Local residual blocks will assigned as
   * the current local kernel residual
   */
  void assignTaggedLocalResidual();
  /**
   * Local Jacobian blocks  will be appended by adding
   * the current local kernel Jacobian
   */
  void accumulateTaggedLocalMatrix();
  /**
   * Local Jacobian blocks will assigned as
   * the current local kernel Jacobian
   */
  void assignTaggedLocalMatrix();

protected:
  /// The vectors this Kernel will contribute to
  std::set<TagID> _vector_tags;

  /// The matrices this Kernel will contribute to
  std::set<TagID> _matrix_tags;

  SubProblem & _subproblem;

  const MooseObject & _moose_object;

  /// Residual blocks Vectors For each Tag
  std::vector<DenseVector<Number> *> _re_blocks;

  /// Kernel blocks Vectors For each Tag
  std::vector<DenseMatrix<Number> *> _ke_blocks;

  /// Holds residual entries as they are accumulated by this Kernel
  DenseVector<Number> _local_re;

  /// Holds residual entries as they are accumulated by this Kernel
  DenseMatrix<Number> _local_ke;
};

#endif /* TAGGINGINTERFACE_H */
