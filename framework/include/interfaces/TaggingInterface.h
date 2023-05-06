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
#include "Assembly.h"
#include "SystemBase.h"
#include "MooseVariableFE.h"

#include "libmesh/dense_vector.h"
#include "metaphysicl/raw_type.h"

#include <vector>

// Forward declarations
class InputParameters;
class MooseObject;
class SubProblem;
class Assembly;
class ReferenceResidualProblem;

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

  /**
   * Class that is used as a parameter to getVectorTags() that allows only
   * AttribVectorTags methods to call it
   */
  class GetVectorTagsKey
  {
    friend class AttribVectorTags;
    friend class NonlinearEigenSystem;
    template <typename>
    friend class MooseObjectTagWarehouse;

    GetVectorTagsKey() {}
    GetVectorTagsKey(const GetVectorTagsKey &) {}
  };

  const std::set<TagID> & getVectorTags(GetVectorTagsKey) const { return _vector_tags; }

  /**
   * Class that is used as a parameter to getMatrixTags() that allows only
   * AttribMatrixTags methods to call it
   */
  class GetMatrixTagsKey
  {
    friend class AttribMatrixTags;
    friend class NonlinearEigenSystem;
    template <typename>
    friend class MooseObjectTagWarehouse;

    GetMatrixTagsKey() {}
    GetMatrixTagsKey(const GetMatrixTagsKey &) {}
  };

  const std::set<TagID> & getMatrixTags(GetMatrixTagsKey) const { return _matrix_tags; }

protected:
  /**
   * Prepare data for computing element residual according to active tags.
   * Residual blocks for different tags will be extracted from Assembly.
   * A local residual will be zeroed. It should be called
   * right before the local element vector is computed.
   */
  void prepareVectorTag(Assembly & assembly, unsigned int ivar);

  void prepareVectorTag(Assembly & assembly,
                        unsigned int ivar,
                        const ReferenceResidualProblem * ref_problem,
                        bool prepare_non_ref_tags);

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
   * Local residual blocks for the specified tags will be appended by adding the current local
   * kernel residual. It should be called after the local element vector has been computed.
   */
  void accumulateTaggedLocalResidual(const std::set<TagID> & vector_tags);

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

  /**
   * Process the provided incoming residuals corresponding to the provided dof indices
   */
  template <typename Residuals, typename Indices>
  void processResiduals(Assembly & assembly,
                        const Residuals & residuals,
                        const Indices & dof_indices,
                        Real scaling_factor);

  /**
   * Process the provided incoming residuals and derivatives for the Jacobian, corresponding to the
   * provided dof indices
   */
  template <typename Residuals, typename Indices>
  void processResidualsAndJacobian(Assembly & assembly,
                                   const Residuals & residuals,
                                   const Indices & dof_indices,
                                   Real scaling_factor);

  /**
   * Process the provided incoming residualsderivatives for the Jacobian, corresponding to the
   * provided dof indices
   */
  template <typename Residuals, typename Indices>
  void processJacobian(Assembly & assembly,
                       const Residuals & residuals,
                       const Indices & dof_indices,
                       Real scaling_factor);

  /**
   * Process the provided incoming residuals corresponding to the provided dof indices without
   * constraints
   */
  template <typename Residuals, typename Indices>
  void processResidualsWithoutConstraints(Assembly & assembly,
                                          const Residuals & residuals,
                                          const Indices & dof_indices,
                                          Real scaling_factor);

  /**
   * Process the provided incoming residuals and derivatives for the Jacobian, corresponding to the
   * provided dof indices without constraints
   */
  template <typename Residuals, typename Indices>
  void processResidualsAndJacobianWithoutConstraints(Assembly & assembly,
                                                     const Residuals & residuals,
                                                     const Indices & dof_indices,
                                                     Real scaling_factor);

  /**
   * Process the provided incoming residualsderivatives for the Jacobian, corresponding to the
   * provided dof indices without constraints
   */
  template <typename Residuals, typename Indices>
  void processJacobianWithoutConstraints(Assembly & assembly,
                                         const Residuals & residuals,
                                         const Indices & dof_indices,
                                         Real scaling_factor);

  /**
   * Process a single Jacobian element
   */
  void processJacobianElement(Assembly & assembly,
                              dof_id_type row_index,
                              dof_id_type column_index,
                              Real value,
                              Real scaling_factor) = delete;

  /**
   * Process a local Jacobian matrix
   */
  void processJacobian(Assembly & assembly,
                       const std::vector<dof_id_type> & row_indices,
                       const std::vector<dof_id_type> & column_indices,
                       DenseMatrix<Real> & local_k,
                       Real scaling_factor) = delete;

  /**
   * Process a single Jacobian element
   */
  void processJacobianElement(Assembly & assembly,
                              Real value,
                              dof_id_type row_index,
                              dof_id_type column_index,
                              Real scaling_factor);

  /**
   * Process a local Jacobian matrix
   */
  void processJacobian(Assembly & assembly,
                       DenseMatrix<Real> & local_k,
                       const std::vector<dof_id_type> & row_indices,
                       const std::vector<dof_id_type> & column_indices,
                       Real scaling_factor);

  /**
   * Set residual using the variables' insertion API
   */
  template <typename T>
  void setResidual(SystemBase & sys, const T & residual, MooseVariableFE<T> & var);

  /**
   * Set residual at a specified degree of freedom index
   */
  void setResidual(SystemBase & sys, Real residual, dof_id_type dof_index);

  /// SubProblem that contains tag info
  SubProblem & _subproblem;

  /// Holds residual entries as they are accumulated by this Kernel
  DenseVector<Number> _local_re;

  /// Holds residual entries as they are accumulated by this Kernel
  DenseMatrix<Number> _local_ke;

private:
  /**
   * Prepare data for computing element residual according to the specified tags
   * Residual blocks for different tags will be extracted from Assembly.
   * A local residual will be zeroed. It should be called
   * right before the local element vector is computed.
   */
  void prepareVectorTagInternal(Assembly & assembly,
                                unsigned int ivar,
                                const std::set<TagID> & vector_tags,
                                const std::set<TagID> & absolute_value_vector_tags,
                                const std::set<TagID> & vector_tags_to_skip,
                                const std::set<TagID> & absolute_value_vector_tags_to_skip);

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

  /// Residual blocks Vectors For each Tag
  std::vector<DenseVector<Number> *> _re_blocks;

  /// Residual blocks for absolute value residual tags
  std::vector<DenseVector<Number> *> _absre_blocks;

  /// Kernel blocks Vectors For each Tag
  std::vector<DenseMatrix<Number> *> _ke_blocks;

  /// A container to hold absolute values of residuals passed into \p processResiduals. We maintain
  /// this data member to avoid constant dynamic heap allocations
  std::vector<Real> _absolute_residuals;
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

template <typename Residuals, typename Indices>
void
TaggingInterface::processResiduals(Assembly & assembly,
                                   const Residuals & residuals,
                                   const Indices & dof_indices,
                                   const Real scaling_factor)
{
  assembly.processResiduals(residuals, dof_indices, _vector_tags, scaling_factor);
  if (!_abs_vector_tags.empty())
  {
    _absolute_residuals.resize(residuals.size());
    for (const auto i : index_range(residuals))
      _absolute_residuals[i] = std::abs(MetaPhysicL::raw_value(residuals[i]));

    assembly.processResiduals(_absolute_residuals, dof_indices, _abs_vector_tags, scaling_factor);
  }
}

template <typename Residuals, typename Indices>
void
TaggingInterface::processResidualsWithoutConstraints(Assembly & assembly,
                                                     const Residuals & residuals,
                                                     const Indices & dof_indices,
                                                     const Real scaling_factor)
{
  assembly.processResidualsWithoutConstraints(residuals, dof_indices, _vector_tags, scaling_factor);
  if (!_abs_vector_tags.empty())
  {
    _absolute_residuals.resize(residuals.size());
    for (const auto i : index_range(residuals))
      _absolute_residuals[i] = std::abs(MetaPhysicL::raw_value(residuals[i]));

    assembly.processResidualsWithoutConstraints(
        _absolute_residuals, dof_indices, _abs_vector_tags, scaling_factor);
  }
}

template <typename Residuals, typename Indices>
void
TaggingInterface::processResidualsAndJacobian(Assembly & assembly,
                                              const Residuals & residuals,
                                              const Indices & dof_indices,
                                              Real scaling_factor)
{
  processResiduals(assembly, residuals, dof_indices, scaling_factor);
  processJacobian(assembly, residuals, dof_indices, scaling_factor);
}

template <typename Residuals, typename Indices>
void
TaggingInterface::processJacobian(Assembly & assembly,
                                  const Residuals & residuals,
                                  const Indices & dof_indices,
                                  Real scaling_factor)
{
  assembly.processJacobian(residuals, dof_indices, _matrix_tags, scaling_factor);
}

template <typename Residuals, typename Indices>
void
TaggingInterface::processResidualsAndJacobianWithoutConstraints(Assembly & assembly,
                                                                const Residuals & residuals,
                                                                const Indices & dof_indices,
                                                                Real scaling_factor)
{
  processResidualsWithoutConstraints(assembly, residuals, dof_indices, scaling_factor);
  processJacobianWithoutConstraints(assembly, residuals, dof_indices, scaling_factor);
}

template <typename Residuals, typename Indices>
void
TaggingInterface::processJacobianWithoutConstraints(Assembly & assembly,
                                                    const Residuals & residuals,
                                                    const Indices & dof_indices,
                                                    Real scaling_factor)
{
  assembly.processJacobianWithoutConstraints(residuals, dof_indices, _matrix_tags, scaling_factor);
}

inline void
TaggingInterface::processJacobianElement(Assembly & assembly,
                                         const Real value,
                                         const dof_id_type row_index,
                                         const dof_id_type column_index,
                                         const Real scaling_factor)
{
  assembly.cacheJacobian(row_index, column_index, value * scaling_factor, _matrix_tags);
}

inline void
TaggingInterface::processJacobian(Assembly & assembly,
                                  DenseMatrix<Real> & local_k,
                                  const std::vector<dof_id_type> & row_indices,
                                  const std::vector<dof_id_type> & column_indices,
                                  const Real scaling_factor)
{
  for (const auto matrix_tag : _matrix_tags)
    assembly.cacheJacobianBlock(local_k, row_indices, column_indices, scaling_factor, matrix_tag);
}

template <typename T>
void
TaggingInterface::setResidual(SystemBase & sys, const T & residual, MooseVariableFE<T> & var)
{
  for (const auto tag_id : _vector_tags)
    if (sys.hasVector(tag_id))
      var.insertNodalValue(sys.getVector(tag_id), residual);
}

inline void
TaggingInterface::setResidual(SystemBase & sys, const Real residual, const dof_id_type dof_index)
{
  for (const auto tag_id : _vector_tags)
    if (sys.hasVector(tag_id))
      sys.getVector(tag_id).set(dof_index, residual);
}
