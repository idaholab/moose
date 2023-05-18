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
#include "NonlinearSystemBase.h"
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

  /**
   * Class that is used as a parameter to some of our vector tag APIs that allows only
   * blessed framework classes to call them
   */
  class VectorTagsKey
  {
    friend class AttribVectorTags;
    friend class NonlinearEigenSystem;
    template <typename>
    friend class MooseObjectTagWarehouse;

    VectorTagsKey() {}
    VectorTagsKey(const VectorTagsKey &) {}
  };

  /**
   * Class that is used as a parameter to some of our matrix tag APIs that allows only
   * blessed framework classes to call them
   */
  class MatrixTagsKey
  {
    friend class AttribMatrixTags;
    friend class NonlinearEigenSystem;
    template <typename>
    friend class MooseObjectTagWarehouse;

    MatrixTagsKey() {}
    MatrixTagsKey(const MatrixTagsKey &) {}
  };

  /**
   * Enumerate whether a (residual) vector tag is to be of a non-reference or reference tag type
   */
  enum class ResidualTagType
  {
    NonReference,
    Reference
  };

  void useVectorTag(const TagName & tag_name, VectorTagsKey);

  void useMatrixTag(const TagName & tag_name, MatrixTagsKey);

  void useVectorTag(TagID tag_id, VectorTagsKey);

  void useMatrixTag(TagID tag_id, MatrixTagsKey);

  bool isVectorTagged() { return _vector_tags.size() > 0; }

  bool isMatrixTagged() { return _matrix_tags.size() > 0; }

  const std::set<TagID> & getVectorTags(VectorTagsKey) const { return _vector_tags; }

  const std::set<TagID> & getMatrixTags(MatrixTagsKey) const { return _matrix_tags; }

protected:
  /**
   * Prepare data for computing element residual according to active tags.
   * Residual blocks for different tags will be extracted from Assembly.
   * A local residual will be zeroed. It should be called
   * right before the local element vector is computed.
   */
  void prepareVectorTag(Assembly & assembly, unsigned int ivar);

  /**
   * Prepare vector tags in a reference residual problem context
   * @param Assembly The assembly object that we obtain the local residual blocks from
   * @param ivar The variable which we are retrieving the local residual blocks for
   * @param ref_problem A pointer to a reference residual problem. This can be a nullptr
   * @param tag_type What type of tags to prepare
   */
  void prepareVectorTag(Assembly & assembly, unsigned int ivar, ResidualTagType tag_type);

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
   * Prepare data for computing element jacobian according to the active tags.
   * Jacobian blocks for different tags will be extracted from Assembly.
   * A local Jacobian will be zeroed. It should be called
   * right before the local element matrix is computed.
   */
  void prepareMatrixTag(Assembly & assembly, unsigned int ivar, unsigned int jvar);

  void prepareMatrixTag(Assembly & assembly,
                        unsigned int ivar,
                        unsigned int jvar,
                        DenseMatrix<Number> & k) const;

  /**
   * Prepare data for computing nonlocal element jacobian according to the active tags.
   * Jacobian blocks for different tags will be extracted from Assembly.
   * A nonlocal Jacobian will be zeroed. It should be called
   * right before the nonlocal element matrix is computed.
   */
  void prepareMatrixTagNonlocal(Assembly & assembly, unsigned int ivar, unsigned int jvar);

  /**
   * Prepare data for computing element jacobian according to the active tags
   * for DG and interface kernels.
   * Jacobian blocks for different tags will be extracted from Assembly.
   * A local Jacobian will be zeroed. It should be called
   * right before the local element matrix is computed.
   */
  void prepareMatrixTagNeighbor(Assembly & assembly,
                                unsigned int ivar,
                                unsigned int jvar,
                                Moose::DGJacobianType type);

  void prepareMatrixTagNeighbor(Assembly & assembly,
                                unsigned int ivar,
                                unsigned int jvar,
                                Moose::DGJacobianType type,
                                DenseMatrix<Number> & k) const;

  /**
   * Prepare data for computing the jacobian according to the active tags for mortar.  Jacobian
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

  void accumulateTaggedLocalMatrix(Assembly & assembly,
                                   unsigned int ivar,
                                   unsigned int jvar,
                                   const DenseMatrix<Number> & k);

  /**
   * Nonlocal Jacobian blocks  will be appended by adding the current nonlocal kernel Jacobian.
   * It should be called after the nonlocal element matrix has been computed.
   */
  void accumulateTaggedNonlocalMatrix();

  void accumulateTaggedLocalMatrix(Assembly & assembly,
                                   unsigned int ivar,
                                   unsigned int jvar,
                                   Moose::DGJacobianType type,
                                   const DenseMatrix<Number> & k);

  /**
   * Local Jacobian blocks will assigned as the current local kernel Jacobian.
   * It should be called after the local element matrix has been computed.
   */
  void assignTaggedLocalMatrix();

  /**
   * Add the provided incoming residuals corresponding to the provided dof indices
   */
  template <typename Residuals, typename Indices>
  void addResiduals(Assembly & assembly,
                    const Residuals & residuals,
                    const Indices & dof_indices,
                    Real scaling_factor);

  /**
   * Add the provided incoming residuals corresponding to the provided dof indices
   */
  template <typename T, typename Indices>
  void addResiduals(Assembly & assembly,
                    const DenseVector<T> & residuals,
                    const Indices & dof_indices,
                    Real scaling_factor);

  /**
   * Add the provided incoming residuals and derivatives for the Jacobian, corresponding to the
   * provided dof indices
   */
  template <typename Residuals, typename Indices>
  void addResidualsAndJacobian(Assembly & assembly,
                               const Residuals & residuals,
                               const Indices & dof_indices,
                               Real scaling_factor);

  /**
   * Add the provided residual derivatives into the Jacobian for the provided dof indices
   */
  template <typename Residuals, typename Indices>
  void addJacobian(Assembly & assembly,
                   const Residuals & residuals,
                   const Indices & dof_indices,
                   Real scaling_factor);

  /**
   * Add the provided incoming residuals corresponding to the provided dof indices. This API should
   * only be used if the caller knows that no libMesh-level constraints (hanging nodes or periodic
   * boundary conditions) apply to the provided dof indices
   */
  template <typename Residuals, typename Indices>
  void addResidualsWithoutConstraints(Assembly & assembly,
                                      const Residuals & residuals,
                                      const Indices & dof_indices,
                                      Real scaling_factor);

  /**
   * Add the provided incoming residuals and derivatives for the Jacobian, corresponding to the
   * provided dof indices.  This API should only be used if the caller knows that no libMesh-level
   * constraints (hanging nodes or periodic boundary conditions) apply to the provided dof indices
   */
  template <typename Residuals, typename Indices>
  void addResidualsAndJacobianWithoutConstraints(Assembly & assembly,
                                                 const Residuals & residuals,
                                                 const Indices & dof_indices,
                                                 Real scaling_factor);

  /**
   * Add the provided residual derivatives into the Jacobian for the provided dof indices. This API
   * should only be used if the caller knows that no libMesh-level constraints (hanging nodes or
   * periodic boundary conditions) apply to the provided dof indices
   */
  template <typename Residuals, typename Indices>
  void addJacobianWithoutConstraints(Assembly & assembly,
                                     const Residuals & residuals,
                                     const Indices & dof_indices,
                                     Real scaling_factor);

  /**
   * Add into a single Jacobian element
   */
  void addJacobianElement(Assembly & assembly,
                          Real value,
                          dof_id_type row_index,
                          dof_id_type column_index,
                          Real scaling_factor);

  /**
   * Add a local Jacobian matrix
   */
  void addJacobian(Assembly & assembly,
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

  /**
   * Set residuals using the provided functor
   */
  template <typename SetResidualFunctor>
  void setResidual(SystemBase & sys, SetResidualFunctor set_residual_functor);

  /// SubProblem that contains tag info
  SubProblem & _subproblem;

  /// Holds local residual entries as they are accumulated by this Kernel
  DenseVector<Number> _local_re;

  /// Holds local Jacobian entries as they are accumulated by this Kernel
  DenseMatrix<Number> _local_ke;

  /// Holds nonlocal Jacobian entries as they are accumulated by this Kernel
  DenseMatrix<Number> _nonlocal_ke;

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
                                const std::set<TagID> & absolute_value_vector_tags);

  /// The residual tag ids this Kernel will contribute to
  std::set<TagID> _vector_tags;

  /// The absolute value residual tag ids
  std::set<TagID> _abs_vector_tags;

  /// The matrices this Kernel will contribute to
  std::set<TagID> _matrix_tags;

  /// A set to hold vector tags excluding the reference residual tag. If there is no reference
  /// residual problem, this container is the same as \p _vector_tags;
  std::set<TagID> _non_ref_vector_tags;

  /// A set to hold absolute value vector tags excluding the reference residual tag. If there is no
  /// reference residual problem, this container is the same as \p _abs_vector_tags;
  std::set<TagID> _non_ref_abs_vector_tags;

  /// A set of either size 1 or 0. If we have a reference residual problem and \p _vector_tags holds
  /// the reference vector tag, then this set holds the reference vector tags, otherwise it holds
  /// nothing
  std::set<TagID> _ref_vector_tags;

  /// A set of either size 1 or 0. If we have a reference residual problem and \p _abs_vector_tags
  /// holds the reference vector tag, then this set holds the reference vector tags, otherwise it
  /// holds nothing
  std::set<TagID> _ref_abs_vector_tags;

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

  /// A container to hold absolute values of residuals passed into \p addResiduals. We maintain
  /// this data member to avoid constant dynamic heap allocations
  std::vector<Real> _absolute_residuals;

  friend void NonlinearSystemBase::constraintJacobians(bool);
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
TaggingInterface::addResiduals(Assembly & assembly,
                               const Residuals & residuals,
                               const Indices & dof_indices,
                               const Real scaling_factor)
{
  assembly.cacheResiduals(
      residuals, dof_indices, scaling_factor, Assembly::LocalDataKey{}, _vector_tags);
  if (!_abs_vector_tags.empty())
  {
    _absolute_residuals.resize(residuals.size());
    for (const auto i : index_range(residuals))
      _absolute_residuals[i] = std::abs(MetaPhysicL::raw_value(residuals[i]));

    assembly.cacheResiduals(_absolute_residuals,
                            dof_indices,
                            scaling_factor,
                            Assembly::LocalDataKey{},
                            _abs_vector_tags);
  }
}

template <typename T, typename Indices>
void
TaggingInterface::addResiduals(Assembly & assembly,
                               const DenseVector<T> & residuals,
                               const Indices & dof_indices,
                               const Real scaling_factor)
{
  addResiduals(assembly, residuals.get_values(), dof_indices, scaling_factor);
}

template <typename Residuals, typename Indices>
void
TaggingInterface::addResidualsWithoutConstraints(Assembly & assembly,
                                                 const Residuals & residuals,
                                                 const Indices & dof_indices,
                                                 const Real scaling_factor)
{
  assembly.cacheResidualsWithoutConstraints(
      residuals, dof_indices, scaling_factor, Assembly::LocalDataKey{}, _vector_tags);
  if (!_abs_vector_tags.empty())
  {
    _absolute_residuals.resize(residuals.size());
    for (const auto i : index_range(residuals))
      _absolute_residuals[i] = std::abs(MetaPhysicL::raw_value(residuals[i]));

    assembly.cacheResidualsWithoutConstraints(_absolute_residuals,
                                              dof_indices,
                                              scaling_factor,
                                              Assembly::LocalDataKey{},
                                              _abs_vector_tags);
  }
}

template <typename Residuals, typename Indices>
void
TaggingInterface::addResidualsAndJacobian(Assembly & assembly,
                                          const Residuals & residuals,
                                          const Indices & dof_indices,
                                          Real scaling_factor)
{
  addResiduals(assembly, residuals, dof_indices, scaling_factor);
  addJacobian(assembly, residuals, dof_indices, scaling_factor);
}

template <typename Residuals, typename Indices>
void
TaggingInterface::addJacobian(Assembly & assembly,
                              const Residuals & residuals,
                              const Indices & dof_indices,
                              Real scaling_factor)
{
  assembly.cacheJacobian(
      residuals, dof_indices, scaling_factor, Assembly::LocalDataKey{}, _matrix_tags);
}

template <typename Residuals, typename Indices>
void
TaggingInterface::addResidualsAndJacobianWithoutConstraints(Assembly & assembly,
                                                            const Residuals & residuals,
                                                            const Indices & dof_indices,
                                                            Real scaling_factor)
{
  addResidualsWithoutConstraints(assembly, residuals, dof_indices, scaling_factor);
  addJacobianWithoutConstraints(assembly, residuals, dof_indices, scaling_factor);
}

template <typename Residuals, typename Indices>
void
TaggingInterface::addJacobianWithoutConstraints(Assembly & assembly,
                                                const Residuals & residuals,
                                                const Indices & dof_indices,
                                                Real scaling_factor)
{
  assembly.cacheJacobianWithoutConstraints(
      residuals, dof_indices, scaling_factor, Assembly::LocalDataKey{}, _matrix_tags);
}

inline void
TaggingInterface::addJacobianElement(Assembly & assembly,
                                     const Real value,
                                     const dof_id_type row_index,
                                     const dof_id_type column_index,
                                     const Real scaling_factor)
{
  assembly.cacheJacobian(
      row_index, column_index, value * scaling_factor, Assembly::LocalDataKey{}, _matrix_tags);
}

inline void
TaggingInterface::addJacobian(Assembly & assembly,
                              DenseMatrix<Real> & local_k,
                              const std::vector<dof_id_type> & row_indices,
                              const std::vector<dof_id_type> & column_indices,
                              const Real scaling_factor)
{
  for (const auto matrix_tag : _matrix_tags)
    assembly.cacheJacobianBlock(
        local_k, row_indices, column_indices, scaling_factor, Assembly::LocalDataKey{}, matrix_tag);
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

template <typename SetResidualFunctor>
void
TaggingInterface::setResidual(SystemBase & sys, const SetResidualFunctor set_residual_functor)
{
  for (const auto tag_id : _vector_tags)
    if (sys.hasVector(tag_id))
      set_residual_functor(sys.getVector(tag_id));
}
