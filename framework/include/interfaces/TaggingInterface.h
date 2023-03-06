//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "DenseMatrix.h"
#include "MooseTypes.h"
#include "MultiMooseEnum.h"

#include "libmesh/dense_vector.h"

// Forward declarations
class InputParameters;
class MooseObject;
class SubProblem;
class Assembly;

template <typename T>
InputParameters validParams();

class TaggingInterface
{
public:
  TaggingInterface(const MooseObject * moose_object);
  virtual ~TaggingInterface();

  static InputParameters validParams();

  void useVectorTag(const TagName & tag_name);

  void useMatrixTag(const TagName & tag_name);

  void useVectorTag(TagID tag_id);

  void useMatrixTag(TagID tag_id);

  bool isVectorTagged() { return _vector_tags.size() > 0; }

  bool isMatrixTagged() { return _matrix_tags.size() > 0; }

  const std::set<TagID> & getVectorTags() const { return _vector_tags; }

  const std::set<TagID> & getMatrixTags() const { return _matrix_tags; }

  /**
   * Prepare data for computing element residual the according to active tags.
   * Residual blocks for different tags will be extracted from Assembly.
   * A local residual will be zeroed. It should be called
   * right before the local element vector is computed.
   */
  void prepareVectorTag(Assembly & assembly, unsigned int ivar);

  /**
   * Prepare data for computing element residual the according to active tags
   * for DG and interface kernels.
   * Residual blocks for different tags will be extracted from Assembly.
   * A local residual will be zeroed. It should be called
   * right before the local element vector is computed.
   */
  void prepareVectorTagNeighbor(Assembly & assembly, unsigned int ivar);

  /**
   * Prepare data for computing the residual according to active tags for mortar constraints.
   * Residual blocks for different tags will be extracted from Assembly.  A local residual will be
   * zeroed. It should be called right before the local element vector is computed.
   */
  void prepareVectorTagLower(Assembly & assembly, unsigned int ivar);

  /**
   * Prepare data for computing element jacobian according to the ative tags.
   * Jacobian blocks for different tags will be extracted from Assembly.
   * A local Jacobian will be zeroed. It should be called
   * right before the local element matrix is computed.
   */
  void prepareMatrixTag(Assembly & assembly, unsigned int ivar, unsigned int jvar);

  /**
   * Prepare data for computing element jacobian according to the ative tags
   * for DG and interface kernels.
   * Jacobian blocks for different tags will be extracted from Assembly.
   * A local Jacobian will be zeroed. It should be called
   * right before the local element matrix is computed.
   */
  void prepareMatrixTagNeighbor(Assembly & assembly,
                                unsigned int ivar,
                                unsigned int jvar,
                                Moose::DGJacobianType type);

  /**
   * Prepare data for computing the jacobian according to the ative tags for mortar.  Jacobian
   * blocks for different tags will be extracted from Assembly.  A local Jacobian will be zeroed. It
   * should be called right before the local element matrix is computed.
   */
  void prepareMatrixTagLower(Assembly & assembly,
                             unsigned int ivar,
                             unsigned int jvar,
                             Moose::ConstraintJacobianType type);

  /**
   * Local residual blocks  will be appended by adding the current local kernel residual.
   * It should be called after the local element vector has been computed.
   */
  void accumulateTaggedLocalResidual();

  /**
   * Local residual blocks will assigned as the current local kernel residual.
   * It should be called after the local element vector has been computed.
   */
  void assignTaggedLocalResidual();

  /**
   * Local Jacobian blocks  will be appended by adding the current local kernel Jacobian.
   * It should be called after the local element matrix has been computed.
   */
  void accumulateTaggedLocalMatrix();

  /**
   * Local Jacobian blocks will assigned as the current local kernel Jacobian.
   * It should be called after the local element matrix has been computed.
   */
  void assignTaggedLocalMatrix();

protected:
  /// The residual tag ids this Kernel will contribute to
  std::set<TagID> _vector_tags;

  /// The absolute value residual tag ids
  std::set<TagID> _abs_vector_tags;

  /// The matrices this Kernel will contribute to
  std::set<TagID> _matrix_tags;

  /// Moose objct this tag works on
  const MooseObject & _moose_object;

  /// Parameters from moose object
  const InputParameters & _tag_params;

  /// SubProblem that contains tag info
  SubProblem & _subproblem;

  /// Residual blocks Vectors For each Tag
  std::vector<DenseVector<Number> *> _re_blocks;

  /// Residual blocks for absolute value residual tags
  std::vector<DenseVector<Number> *> _absre_blocks;

  /// Kernel blocks Vectors For each Tag
  std::vector<DenseMatrix<Number> *> _ke_blocks;

  /// Holds residual entries as they are accumulated by this Kernel
  DenseVector<Number> _local_re;

  /// Holds residual entries as they are accumulated by this Kernel
  DenseMatrix<Number> _local_ke;
};

#define usingTaggingInterfaceMembers                                                               \
  using TaggingInterface::_subproblem;                                                             \
  using TaggingInterface::accumulateTaggedLocalResidual;                                           \
  using TaggingInterface::accumulateTaggedLocalMatrix;                                             \
  using TaggingInterface::prepareVectorTag;                                                        \
  using TaggingInterface::prepareMatrixTag;                                                        \
  using TaggingInterface::prepareVectorTagNeighbor;                                                \
  using TaggingInterface::_local_re;                                                               \
  using TaggingInterface::prepareVectorTagLower;                                                   \
  using TaggingInterface::prepareMatrixTagNeighbor;                                                \
  using TaggingInterface::prepareMatrixTagLower;                                                   \
  using TaggingInterface::_local_ke
