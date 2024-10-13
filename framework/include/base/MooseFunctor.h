//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include <tuple>

#include "MooseFunctorForward.h"
#include "MooseFunctorArguments.h"
#include "FaceArgInterface.h"
#include "MooseMesh.h"
#include "MooseTypes.h"
#include "MooseError.h"
#include "MooseUtils.h"

#include "libmesh/remote_elem.h"
#include "libmesh/tensor_tools.h"

#include "metaphysicl/ct_types.h"

#include <unordered_map>
#include <functional>

namespace Moose
{
/**
 * An enumeration of possible functor evaluation kinds. The available options are value, gradient,
 * time derivative (dot), and gradient of time derivative (gradDot)
 */
enum class FunctorEvaluationKind
{
  Value,
  Gradient,
  Dot,
  GradDot
};

/**
 * A structure that defines the return type of a functor based on the type of the functor and the
 * requested evaluation kind, e.g. value, gradient, time derivative, or gradient of time derivative
 */
template <typename, FunctorEvaluationKind>
struct FunctorReturnType;

/**
 * The return type for a value evaluation is just the type of the functor
 */
template <typename T>
struct FunctorReturnType<T, FunctorEvaluationKind::Value>
{
  typedef T type;
};

/**
 * The return type of a gradient evaluation is the rank increment of a value return type. So if the
 * value type is Real, then a gradient will be a VectorValue<Real>. This also allows for containers
 * of mathematical types. So if a value type is std::vector<Real>, then the gradient type will be
 * std::vector<VectorValue<Real>>
 */
template <typename T>
struct FunctorReturnType<T, FunctorEvaluationKind::Gradient>
{
  typedef typename MetaPhysicL::ReplaceAlgebraicType<
      T,
      typename libMesh::TensorTools::IncrementRank<
          typename MetaPhysicL::ValueType<T>::type>::type>::type type;
};

/**
 * The return type of a time derivative evaluation is the same as the value type
 */
template <typename T>
struct FunctorReturnType<T, FunctorEvaluationKind::Dot>
{
  typedef T type;
};

/**
 * The return type of a gradient of time derivative evaluation is the same as the gradient type
 */
template <typename T>
struct FunctorReturnType<T, FunctorEvaluationKind::GradDot>
{
  typedef typename FunctorReturnType<T, FunctorEvaluationKind::Gradient>::type type;
};

/**
 * This structure takes an evaluation kind as a template argument and defines a constant expression
 * indicating the associated gradient kind
 */
template <FunctorEvaluationKind>
struct FunctorGradientEvaluationKind;

/**
 * The gradient kind associated with a value is simply the gradient
 */
template <>
struct FunctorGradientEvaluationKind<FunctorEvaluationKind::Value>
{
  static constexpr FunctorEvaluationKind value = FunctorEvaluationKind::Gradient;
};

/**
 * The gradient kind associated with a time derivative is the gradient of the time derivative
 */
template <>
struct FunctorGradientEvaluationKind<FunctorEvaluationKind::Dot>
{
  static constexpr FunctorEvaluationKind value = FunctorEvaluationKind::GradDot;
};

/**
 * Abstract base class that can be used to hold collections of functors
 */
class FunctorAbstract : public FaceArgInterface
{
public:
  virtual void residualSetup() = 0;
  virtual void jacobianSetup() = 0;
  virtual void timestepSetup() = 0;
  virtual void customSetup(const ExecFlagType & exec_type) = 0;
};

/**
 * Base class template for functor objects. This class template defines various \p operator()
 * overloads that allow a user to evaluate the functor at arbitrary geometric locations. This
 * template is meant to enable highly flexible on-the-fly variable and material property
 * evaluations
 */
template <typename T>
class FunctorBase : public FunctorAbstract
{
public:
  using FunctorType = FunctorBase<T>;
  using ValueType = T;
  /// This rigmarole makes it so that a user can create functors that return containers (std::vector,
  /// std::array). This logic will make it such that if a user requests a functor type T that is a
  /// container of algebraic types, for example Reals, then the GradientType will be a container of
  /// the gradients of those algebraic types, in this example VectorValue<Reals>. So if T is
  /// std::vector<Real>, then GradientType will be std::vector<VectorValue<Real>>. As another
  /// example: T = std::array<VectorValue<Real>, 1> -> GradientType = std::array<TensorValue<Real>,
  /// 1>
  using GradientType = typename FunctorReturnType<T, FunctorEvaluationKind::Gradient>::type;
  using DotType = ValueType;

  virtual ~FunctorBase() = default;
  FunctorBase(const MooseFunctorName & name,
              const std::set<ExecFlagType> & clearance_schedule = {EXEC_ALWAYS})
    : _always_evaluate(true), _functor_name(name)

  {
    setCacheClearanceSchedule(clearance_schedule);
  }

  /**
   * Perform a generic evaluation based on the supplied template argument \p FET and supplied
   * spatial and temporal arguments
   */
  template <FunctorEvaluationKind FET, typename Space, typename State>
  typename FunctorReturnType<T, FET>::type genericEvaluate(const Space & r,
                                                           const State & state) const;

  /// Return the functor name
  const MooseFunctorName & functorName() const { return _functor_name; }

  ///@{
  /**
   * Same as their \p evaluate overloads with the same arguments but allows for caching
   * implementation. These are the methods a user will call in their code
   */
  ValueType operator()(const ElemArg & elem, const StateArg & state) const;
  ValueType operator()(const FaceArg & face, const StateArg & state) const;
  ValueType operator()(const ElemQpArg & qp, const StateArg & state) const;
  ValueType operator()(const ElemSideQpArg & qp, const StateArg & state) const;
  ValueType operator()(const ElemPointArg & elem_point, const StateArg & state) const;
  ValueType operator()(const NodeArg & node, const StateArg & state) const;
  ///@}

  ///@{
  /**
   * Same as their \p evaluateGradient overloads with the same arguments but allows for caching
   * implementation. These are the methods a user will call in their code
   */
  GradientType gradient(const ElemArg & elem, const StateArg & state) const;
  GradientType gradient(const FaceArg & face, const StateArg & state) const;
  GradientType gradient(const ElemQpArg & qp, const StateArg & state) const;
  GradientType gradient(const ElemSideQpArg & qp, const StateArg & state) const;
  GradientType gradient(const ElemPointArg & elem_point, const StateArg & state) const;
  GradientType gradient(const NodeArg & node, const StateArg & state) const;
  ///@}

  ///@{
  /**
   * Same as their \p evaluateDot overloads with the same arguments but allows for caching
   * implementation. These are the methods a user will call in their code
   */
  DotType dot(const ElemArg & elem, const StateArg & state) const;
  DotType dot(const FaceArg & face, const StateArg & state) const;
  DotType dot(const ElemQpArg & qp, const StateArg & state) const;
  DotType dot(const ElemSideQpArg & qp, const StateArg & state) const;
  DotType dot(const ElemPointArg & elem_point, const StateArg & state) const;
  DotType dot(const NodeArg & node, const StateArg & state) const;
  ///@}

  ///@{
  /**
   * Same as their \p evaluateGradDot overloads with the same arguments but allows for caching
   * implementation. These are the methods a user will call in their code
   */
  GradientType gradDot(const ElemArg & elem, const StateArg & state) const;
  GradientType gradDot(const FaceArg & face, const StateArg & state) const;
  GradientType gradDot(const ElemQpArg & qp, const StateArg & state) const;
  GradientType gradDot(const ElemSideQpArg & qp, const StateArg & state) const;
  GradientType gradDot(const ElemPointArg & elem_point, const StateArg & state) const;
  GradientType gradDot(const NodeArg & node, const StateArg & state) const;
  ///@}

  virtual void residualSetup() override;
  virtual void jacobianSetup() override;
  virtual void timestepSetup() override;
  virtual void customSetup(const ExecFlagType & exec_type) override;

  /**
   * Set how often to clear the functor evaluation cache
   */
  void setCacheClearanceSchedule(const std::set<ExecFlagType> & clearance_schedule);

  /**
   * Returns whether the functor is defined on this block
   */
  virtual bool hasBlocks(SubdomainID /* id */) const
  {
    mooseError("Block restriction has not been implemented for functor " + functorName());
    return false;
  }

  /**
   * Returns whether this (sided) face is an extrapolated boundary face for
   * this functor
   */
  virtual bool isExtrapolatedBoundaryFace(const FaceInfo &, const Elem *, const StateArg &) const
  {
    mooseError("not implemented");
  }

  /**
   * Returns true if the face is an internal face
   */
  bool isInternalFace(const FaceInfo &) const;

  /**
   * Returns true if this functor is a constant
   */
  virtual bool isConstant() const { return false; }

  virtual bool hasFaceSide(const FaceInfo & fi, const bool fi_elem_side) const override;

  /**
   * Examines the incoming face argument. If the face argument producer (residual object,
   * postprocessor, etc.) did not indicate a sidedness to the face, e.g. if the \p face_side member
   * of the \p FaceArg is \p nullptr, then we may "modify" the sidedness of the argument if we are
   * only defined on one side of the face. If the face argument producer \emph has indicated a
   * sidedness and we are not defined on that side, then we will error
   * @param face The face argument created by the face argument producer, likely a residual object
   * @return A face with possibly changed sidedness depending on whether we aren't defined on both
   * sides of the face
   */
  void checkFace(const Moose::FaceArg & face) const;

  /**
   * Whether this functor supports evaluation with FaceArg
   */
  virtual bool supportsFaceArg() const = 0;

  /**
   * Whether this functor supports evaluation with ElemSideQpArg
   */
  virtual bool supportsElemSideQpArg() const = 0;

protected:
  /** @name Functor evaluation routines
   * These methods are all for evaluating functors with different kinds of spatial arguments. Each
   * of these methods also takes a state argument. For a description of the state argument, please
   * see the \p StateArg doxygen
   */
  ///@{
  /**
   * Evaluate the functor with a given element. Some example implementations of this method
   * could compute an element-average or evaluate at the element centroid
   */
  virtual ValueType evaluate(const ElemArg & elem, const StateArg & state) const = 0;

  /**
   * @param face See the \p FaceArg doxygen
   * @param state See the \p StateArg doxygen
   * @return The functor evaluated at the requested state and space
   */
  virtual ValueType evaluate(const FaceArg & face, const StateArg & state) const = 0;

  /**
   * @param qp See the \p ElemQpArg doxygen
   * @param state See the \p StateArg doxygen
   * @return The functor evaluated at the requested state and space
   */
  virtual ValueType evaluate(const ElemQpArg & qp, const StateArg & state) const = 0;

  /**
   * @param side_qp See the \p ElemSideQpArg doxygen
   * @param state See the \p StateArg doxygen
   * @return The functor evaluated at the requested state and space
   */
  virtual ValueType evaluate(const ElemSideQpArg & side_qp, const StateArg & state) const = 0;

  /**
   * Evaluate the functor with a given element and point. Some example implementations of this
   * method could perform a two-term Taylor expansion using cell-centered value and gradient
   */
  virtual ValueType evaluate(const ElemPointArg & elem_point, const StateArg & state) const = 0;

  virtual ValueType evaluate(const NodeArg & node, const StateArg & state) const = 0;

  /**
   * Evaluate the functor gradient with a given element. Some example implementations of this
   * method could compute an element-average or evaluate at the element centroid
   */
  virtual GradientType evaluateGradient(const ElemArg &, const StateArg &) const
  {
    mooseError("Element gradient not implemented for functor " + functorName());
  }

  /**
   * @param face See the \p FaceArg doxygen
   * @param state See the \p StateArg doxygen
   * @return The functor gradient evaluated at the requested state and space
   */
  virtual GradientType evaluateGradient(const FaceArg &, const StateArg &) const
  {
    mooseError("Face gradient not implemented for functor " + functorName());
  }

  /**
   * @param qp See the \p ElemQpArg doxygen
   * @param state See the \p StateArg doxygen
   * @return The functor gradient evaluated at the requested state and space
   */
  virtual GradientType evaluateGradient(const ElemQpArg &, const StateArg &) const
  {
    mooseError("Element quadrature point gradient not implemented for functor " + functorName());
  }

  /**
   * @param side_qp See the \p ElemSideQpArg doxygen
   * @param state See the \p StateArg doxygen
   * @return The functor gradient evaluated at the requested state and space
   */
  virtual GradientType evaluateGradient(const ElemSideQpArg &, const StateArg &) const
  {
    mooseError("Element side quadrature point gradient not implemented for functor " +
               functorName());
  }

  /**
   * Evaluate the functor gradient with a given element and point
   */
  virtual GradientType evaluateGradient(const ElemPointArg &, const StateArg &) const
  {
    mooseError("Element-point gradient not implemented for functor " + functorName());
  }

  virtual GradientType evaluateGradient(const NodeArg &, const StateArg &) const
  {
    mooseError("Gradient at node not implemented for functor " + functorName());
  }

  /**
   * Evaluate the functor time derivative with a given element. Some example implementations of
   * this method could compute an element-average or evaluate at the element centroid
   */
  virtual DotType evaluateDot(const ElemArg &, const StateArg &) const
  {
    mooseError("Element time derivative not implemented for functor " + functorName());
  }

  /**
   * @param face See the \p FaceArg doxygen
   * @param state See the \p StateArg doxygen
   * @return The functor time derivative evaluated at the requested state and space
   */
  virtual DotType evaluateDot(const FaceArg &, const StateArg &) const
  {
    mooseError("Face time derivative not implemented for functor " + functorName());
  }

  /**
   * @param qp See the \p ElemQpArg doxygen
   * @param state See the \p StateArg doxygen
   * @return The functor time derivative evaluated at the requested state and space
   */
  virtual DotType evaluateDot(const ElemQpArg &, const StateArg &) const
  {
    mooseError("Element quadrature point time derivative not implemented for functor " +
               functorName());
  }

  /**
   * @param side_qp See the \p ElemSideQpArg doxygen
   * @param state See the \p StateArg doxygen
   * @return The functor time derivative evaluated at the requested state and space
   */
  virtual DotType evaluateDot(const ElemSideQpArg &, const StateArg &) const
  {
    mooseError("Element side quadrature point time derivative not implemented for functor " +
               functorName());
  }

  /**
   * Evaluate the functor time derivative with a given element and point
   */
  virtual DotType evaluateDot(const ElemPointArg &, const StateArg &) const
  {
    mooseError("Element-point time derivative not implemented for functor " + functorName());
  }

  virtual DotType evaluateDot(const NodeArg &, const StateArg &) const
  {
    mooseError("Time derivative at node not implemented for functor " + functorName());
  }

  /**
   * Evaluate the functor gradient-dot with a given element. Some example implementations of this
   * method could compute an element-average or evaluate at the element centroid
   */
  virtual GradientType evaluateGradDot(const ElemArg &, const StateArg &) const
  {
    mooseError("Element gradient-dot not implemented for functor " + functorName());
  }

  /**
   * @param face See the \p FaceArg doxygen
   * @param state See the \p StateArg doxygen
   * @return The functor gradient-dot evaluated at the requested state and space
   */
  virtual GradientType evaluateGradDot(const FaceArg &, const StateArg &) const
  {
    mooseError("Face gradient-dot not implemented for functor " + functorName());
  }

  /**
   * @param qp See the \p ElemQpArg doxygen
   * @param state See the \p StateArg doxygen
   * @return The functor gradient-dot evaluated at the requested state and space
   */
  virtual GradientType evaluateGradDot(const ElemQpArg &, const StateArg &) const
  {
    mooseError("Element quadrature point gradient-dot not implemented for functor " +
               functorName());
  }

  /**
   * @param side_qp See the \p ElemSideQpArg doxygen
   * @param state See the \p StateArg doxygen
   * @return The functor gradient-dot evaluated at the requested state and space
   */
  virtual GradientType evaluateGradDot(const ElemSideQpArg &, const StateArg &) const
  {
    mooseError("Element side quadrature point gradient-dot not implemented for functor " +
               functorName());
  }

  /**
   * Evaluate the functor gradient-dot with a given element and point
   */
  virtual GradientType evaluateGradDot(const ElemPointArg &, const StateArg &) const
  {
    mooseError("Element-point gradient-dot not implemented for functor " + functorName());
  }

  virtual GradientType evaluateGradDot(const NodeArg &, const StateArg &) const
  {
    mooseError("Gradient-dot at node not implemented for functor " + functorName());
  }
  ///@}

private:
  /**
   * clear cache data
   */
  void clearCacheData();

  /**
   * check a qp cache and if invalid then evaluate
   */
  template <typename SpaceArg, typename StateArg>
  ValueType queryQpCache(unsigned int qp,
                         const libMesh::QBase & qrule,
                         std::vector<std::pair<bool, T>> & qp_cache_data,
                         const SpaceArg & space,
                         const StateArg & state) const;

  /**
   * check a finite volume spatial argument cache and if invalid then evaluate
   */
  template <typename SpaceArg>
  ValueType queryFVArgCache(std::map<SpaceArg, ValueType> & cache_data,
                            const SpaceArg & space) const;

  /// How often to clear the material property cache
  std::set<ExecFlagType> _clearance_schedule;

  /// Boolean to check if we always need evaluation
  bool _always_evaluate;

  // Data for traditional element-quadrature point property evaluations which are useful for
  // caching implementation

  /// Current key for qp map cache
  mutable dof_id_type _current_qp_map_key = libMesh::DofObject::invalid_id;

  /// Current value for qp map cache
  mutable std::vector<std::pair<bool, ValueType>> * _current_qp_map_value = nullptr;

  /// Cached element quadrature point functor property evaluations. The map key is the element
  /// id. The map values should have size corresponding to the number of quadrature points on the
  /// element. The vector elements are pairs. The first member of the pair indicates whether a
  /// cached value has been computed. The second member of the pair is the (cached) value. If the
  /// boolean is false, then the value cannot be trusted
  mutable std::unordered_map<dof_id_type, std::vector<std::pair<bool, ValueType>>> _qp_to_value;

  // Data for traditional element-side-quadrature point property evaluations which are useful for
  // caching implementation

  /// Current key for side-qp map cache
  mutable dof_id_type _current_side_qp_map_key = libMesh::DofObject::invalid_id;

  /// Current value for side-qp map cache
  mutable std::vector<std::vector<std::pair<bool, ValueType>>> * _current_side_qp_map_value =
      nullptr;

  /// Cached element quadrature point functor property evaluations. The map key is the element
  /// id. The map values are a multi-dimensional vector (or vector of vectors) with the first index
  /// corresponding to the side and the second index corresponding to the quadrature point
  /// index. The elements returned after double indexing are pairs. The first member of the pair
  /// indicates whether a cached value has been computed. The second member of the pair is the
  /// (cached) value. If the boolean is false, then the value cannot be trusted
  mutable std::unordered_map<dof_id_type, std::vector<std::vector<std::pair<bool, ValueType>>>>
      _side_qp_to_value;

  /// Map from element arguments to their cached evaluations
  mutable std::map<ElemArg, ValueType> _elem_arg_to_value;

  /// Map from face arguments to their cached evaluations
  mutable std::map<FaceArg, ValueType> _face_arg_to_value;

  /// Map from nodal arguments to their cached evaluations
  mutable std::map<NodeArg, ValueType> _node_arg_to_value;

  /// name of the functor
  MooseFunctorName _functor_name;
};

template <typename T>
bool
FunctorBase<T>::isInternalFace(const FaceInfo & fi) const
{
  if (!fi.neighborPtr())
    return false;

  return hasBlocks(fi.elem().subdomain_id()) && hasBlocks(fi.neighborPtr()->subdomain_id());
}

template <typename T>
template <typename SpaceArg>
typename FunctorBase<T>::ValueType
FunctorBase<T>::queryFVArgCache(std::map<SpaceArg, ValueType> & cache_data,
                                const SpaceArg & space) const
{
  // We don't want to evaluate if the key already exists, so instead we value initialize
  auto [it, inserted] = cache_data.try_emplace(space, ValueType());
  auto & value = it->second;

  if (inserted)
    // value not ready to go
    // this function is only called from functions that assert we are in the current time state
    value = evaluate(space, currentState());

  return value;
}

template <typename T>
typename FunctorBase<T>::ValueType
FunctorBase<T>::operator()(const ElemArg & elem, const StateArg & state) const
{
  if (_always_evaluate)
    return evaluate(elem, state);

  mooseAssert(state.state == 0,
              "Cached evaluations are only currently supported for the current state.");

  return queryFVArgCache(_elem_arg_to_value, elem);
}

template <typename T>
typename FunctorBase<T>::ValueType
FunctorBase<T>::operator()(const FaceArg & face_in, const StateArg & state) const
{
  checkFace(face_in);

  if (_always_evaluate)
    return evaluate(face_in, state);

  mooseAssert(state.state == 0,
              "Cached evaluations are only currently supported for the current state.");

  return queryFVArgCache(_face_arg_to_value, face_in);
}

template <typename T>
template <typename SpaceArg, typename StateArg>
typename FunctorBase<T>::ValueType
FunctorBase<T>::queryQpCache(const unsigned int qp,
                             const libMesh::QBase & qrule,
                             std::vector<std::pair<bool, ValueType>> & qp_cache_data,
                             const SpaceArg & space,
                             const StateArg & state) const
{
  // Check and see whether we even have sized for this quadrature point. If we haven't then we
  // must evaluate
  if (qp >= qp_cache_data.size())
  {
    qp_cache_data.resize(qrule.n_points(), std::make_pair(false, ValueType()));
    auto & pr = qp_cache_data[qp];
    pr.second = evaluate(space, state);
    pr.first = true;
    return pr.second;
  }

  // We've already sized for this qp, so let's see whether we have a valid cache value
  auto & pr = qp_cache_data[qp];
  if (pr.first)
    return pr.second;

  // No valid cache value so evaluate
  pr.second = evaluate(space, state);
  pr.first = true;
  return pr.second;
}

template <typename T>
typename FunctorBase<T>::ValueType
FunctorBase<T>::operator()(const ElemQpArg & elem_qp, const StateArg & state) const
{
  if (_always_evaluate)
    return evaluate(elem_qp, state);

  const auto elem_id = elem_qp.elem->id();
  if (elem_id != _current_qp_map_key)
  {
    _current_qp_map_key = elem_id;
    _current_qp_map_value = &_qp_to_value[elem_id];
  }
  auto & qp_data = *_current_qp_map_value;
  const auto qp = elem_qp.qp;
  const auto * const qrule = elem_qp.qrule;
  mooseAssert(qrule, "qrule must be non-null");

  return queryQpCache(qp, *qrule, qp_data, elem_qp, state);
}

template <typename T>
typename FunctorBase<T>::ValueType
FunctorBase<T>::operator()(const ElemSideQpArg & elem_side_qp, const StateArg & state) const
{
  if (_always_evaluate)
    return evaluate(elem_side_qp, state);

  const Elem * const elem = elem_side_qp.elem;
  mooseAssert(elem, "elem must be non-null");
  const auto elem_id = elem->id();
  if (elem_id != _current_side_qp_map_key)
  {
    _current_side_qp_map_key = elem_id;
    _current_side_qp_map_value = &_side_qp_to_value[elem_id];
  }
  auto & side_qp_data = *_current_side_qp_map_value;
  const auto side = elem_side_qp.side;
  const auto qp = elem_side_qp.qp;
  const auto * const qrule = elem_side_qp.qrule;
  mooseAssert(qrule, "qrule must be non-null");

  // Check and see whether we even have sized for this side
  if (side >= side_qp_data.size())
    side_qp_data.resize(elem->n_sides());

  // Ok we were sized enough for our side
  auto & qp_data = side_qp_data[side];
  return queryQpCache(qp, *qrule, qp_data, elem_side_qp, state);
}

template <typename T>
typename FunctorBase<T>::ValueType
FunctorBase<T>::operator()(const ElemPointArg & elem_point, const StateArg & state) const
{
  return evaluate(elem_point, state);
}

template <typename T>
void
FunctorBase<T>::setCacheClearanceSchedule(const std::set<ExecFlagType> & clearance_schedule)
{
  if (clearance_schedule.count(EXEC_ALWAYS))
    _always_evaluate = true;

  _clearance_schedule = clearance_schedule;
}

template <typename T>
typename FunctorBase<T>::ValueType
FunctorBase<T>::operator()(const NodeArg & node, const StateArg & state) const
{
  return evaluate(node, state);
}

template <typename T>
void
FunctorBase<T>::checkFace(const Moose::FaceArg &
#if DEBUG
                              face
#endif
) const
{
#if DEBUG
  const Elem * const elem = face.face_side;
  const FaceInfo * const fi = face.fi;
  mooseAssert(fi, "face info should be non-null");
  bool check_elem_def = false;
  bool check_neighbor_def = false;
  // We check if the functor is defined on both sides of the face
  if (!elem)
  {
    if (!hasFaceSide(*fi, true))
      check_neighbor_def = true;
    else if (!hasFaceSide(*fi, false))
      check_elem_def = true;
  }
  else if (elem == fi->elemPtr())
    check_elem_def = true;
  else
  {
    mooseAssert(elem == fi->neighborPtr(), "This has to match something");
    check_neighbor_def = true;
  }

  if (check_elem_def && !hasFaceSide(*fi, true))
  {
    std::string additional_message = "It is not defined on the neighbor side either.";
    if (hasFaceSide(*fi, false))
      additional_message = "It is however defined on the neighbor side.";
    additional_message += " Face centroid: " + Moose::stringify(fi->faceCentroid());
    mooseError(_functor_name,
               " is not defined on the element side of the face information, but a face argument "
               "producer "
               "(e.g. residual object, postprocessor, etc.) has requested evaluation there.\n",
               additional_message);
  }
  if (check_neighbor_def && !hasFaceSide(*fi, false))
  {
    std::string additional_message = "It is not defined on the element side either.";
    if (hasFaceSide(*fi, true))
      additional_message = "It is however defined on the element side.";
    additional_message += " Face centroid: " + Moose::stringify(fi->faceCentroid());
    mooseError(
        _functor_name,
        " is not defined on the neighbor side of the face information, but a face argument "
        "producer (e.g. residual object, postprocessor, etc.) has requested evaluation there.\n",
        additional_message);
  }
#endif
}

template <typename T>
void
FunctorBase<T>::clearCacheData()
{
  for (auto & map_pr : _qp_to_value)
    for (auto & pr : map_pr.second)
      pr.first = false;

  for (auto & map_pr : _side_qp_to_value)
  {
    auto & side_vector = map_pr.second;
    for (auto & qp_vector : side_vector)
      for (auto & pr : qp_vector)
        pr.first = false;
  }

  _current_qp_map_key = libMesh::DofObject::invalid_id;
  _current_qp_map_value = nullptr;
  _current_side_qp_map_key = libMesh::DofObject::invalid_id;
  _current_side_qp_map_value = nullptr;

  _elem_arg_to_value.clear();
  _face_arg_to_value.clear();
  _node_arg_to_value.clear();
}

template <typename T>
void
FunctorBase<T>::timestepSetup()
{
  if (_clearance_schedule.count(EXEC_TIMESTEP_BEGIN))
    clearCacheData();
}

template <typename T>
void
FunctorBase<T>::residualSetup()
{
  if (_clearance_schedule.count(EXEC_LINEAR))
    clearCacheData();
}

template <typename T>
void
FunctorBase<T>::jacobianSetup()
{
  if (_clearance_schedule.count(EXEC_NONLINEAR))
    clearCacheData();
}

template <typename T>
void
FunctorBase<T>::customSetup(const ExecFlagType & exec_type)
{
  if (_clearance_schedule.count(exec_type))
    clearCacheData();
}

template <typename T>
typename FunctorBase<T>::GradientType
FunctorBase<T>::gradient(const ElemArg & elem, const StateArg & state) const
{
  return evaluateGradient(elem, state);
}

template <typename T>
typename FunctorBase<T>::GradientType
FunctorBase<T>::gradient(const FaceArg & face, const StateArg & state) const
{
  checkFace(face);
  return evaluateGradient(face, state);
}

template <typename T>
typename FunctorBase<T>::GradientType
FunctorBase<T>::gradient(const ElemQpArg & elem_qp, const StateArg & state) const
{
  return evaluateGradient(elem_qp, state);
}

template <typename T>
typename FunctorBase<T>::GradientType
FunctorBase<T>::gradient(const ElemSideQpArg & elem_side_qp, const StateArg & state) const
{
  return evaluateGradient(elem_side_qp, state);
}

template <typename T>
typename FunctorBase<T>::GradientType
FunctorBase<T>::gradient(const ElemPointArg & elem_point, const StateArg & state) const
{
  return evaluateGradient(elem_point, state);
}

template <typename T>
typename FunctorBase<T>::GradientType
FunctorBase<T>::gradient(const NodeArg & node, const StateArg & state) const
{
  return evaluateGradient(node, state);
}

template <typename T>
typename FunctorBase<T>::DotType
FunctorBase<T>::dot(const ElemArg & elem, const StateArg & state) const
{
  return evaluateDot(elem, state);
}

template <typename T>
typename FunctorBase<T>::DotType
FunctorBase<T>::dot(const FaceArg & face, const StateArg & state) const
{
  checkFace(face);
  return evaluateDot(face, state);
}

template <typename T>
typename FunctorBase<T>::DotType
FunctorBase<T>::dot(const ElemQpArg & elem_qp, const StateArg & state) const
{
  return evaluateDot(elem_qp, state);
}

template <typename T>
typename FunctorBase<T>::DotType
FunctorBase<T>::dot(const ElemSideQpArg & elem_side_qp, const StateArg & state) const
{
  return evaluateDot(elem_side_qp, state);
}

template <typename T>
typename FunctorBase<T>::DotType
FunctorBase<T>::dot(const ElemPointArg & elem_point, const StateArg & state) const
{
  return evaluateDot(elem_point, state);
}

template <typename T>
typename FunctorBase<T>::DotType
FunctorBase<T>::dot(const NodeArg & node, const StateArg & state) const
{
  return evaluateDot(node, state);
}

template <typename T>
typename FunctorBase<T>::GradientType
FunctorBase<T>::gradDot(const ElemArg & elem, const StateArg & state) const
{
  return evaluateGradDot(elem, state);
}

template <typename T>
typename FunctorBase<T>::GradientType
FunctorBase<T>::gradDot(const FaceArg & face, const StateArg & state) const
{
  checkFace(face);
  return evaluateGradDot(face, state);
}

template <typename T>
typename FunctorBase<T>::GradientType
FunctorBase<T>::gradDot(const ElemQpArg & elem_qp, const StateArg & state) const
{
  return evaluateGradDot(elem_qp, state);
}

template <typename T>
typename FunctorBase<T>::GradientType
FunctorBase<T>::gradDot(const ElemSideQpArg & elem_side_qp, const StateArg & state) const
{
  return evaluateGradDot(elem_side_qp, state);
}

template <typename T>
typename FunctorBase<T>::GradientType
FunctorBase<T>::gradDot(const ElemPointArg & elem_point, const StateArg & state) const
{
  return evaluateGradDot(elem_point, state);
}

template <typename T>
typename FunctorBase<T>::GradientType
FunctorBase<T>::gradDot(const NodeArg & node, const StateArg & state) const
{
  return evaluateGradDot(node, state);
}

template <typename T>
bool
FunctorBase<T>::hasFaceSide(const FaceInfo & fi, const bool fi_elem_side) const
{
  if (fi_elem_side)
    return hasBlocks(fi.elem().subdomain_id());
  else
    return fi.neighborPtr() && hasBlocks(fi.neighbor().subdomain_id());
}

template <typename T>
template <FunctorEvaluationKind FET, typename Space, typename State>
typename FunctorReturnType<T, FET>::type
FunctorBase<T>::genericEvaluate(const Space & r, const State & state) const
{
  if constexpr (FET == FunctorEvaluationKind::Value)
    return (*this)(r, state);
  else if constexpr (FET == FunctorEvaluationKind::Gradient)
    return gradient(r, state);
  else if constexpr (FET == FunctorEvaluationKind::Dot)
    return dot(r, state);
  else
    return gradDot(r, state);
}

/**
 * A non-templated base class for functors that allow an owner object to hold
 * different class template instantiations of \p Functor in a single container
 */
class FunctorEnvelopeBase
{
public:
  FunctorEnvelopeBase() = default;
  virtual ~FunctorEnvelopeBase() = default;

  /**
   * @return Whether this envelope wraps a null functor
   */
  virtual bool wrapsNull() const = 0;

  /**
   * @return The return type, as a string, of the functor this envelope wraps
   */
  virtual std::string returnType() const = 0;

  /**
   * @return Whether this envelope wraps a constant functor
   */
  virtual bool isConstant() const = 0;

  /**
   * @return Whether this envelope owns its wrapped functor. This envelope may briefly own null
   * functors during simulation setup or it may own non-AD or AD wrappers of "true" functors, but we
   * should never own any "true" functors, e.g. we expect memory of "true" functors to be managed by
   * warehouses (e.g. variable, function, etc.), or by the \p SubProblem itself. With this
   * expectation, we don't have to worry about performing setup calls
   */
  virtual bool ownsWrappedFunctor() const = 0;
};

/**
 * This is a wrapper that forwards calls to the implementation,
 * which can be switched out at any time without disturbing references to
 * FunctorBase. Implementation motivated by https://stackoverflow.com/a/65455485/4493669
 */
template <typename T>
class FunctorEnvelope final : public FunctorBase<T>, public FunctorEnvelopeBase
{
public:
  using typename Moose::FunctorBase<T>::ValueType;
  using typename Moose::FunctorBase<T>::GradientType;
  using typename Moose::FunctorBase<T>::DotType;

  /**
   * @param wrapped The functor to wrap. We will *not* not own the wrapped object
   */
  FunctorEnvelope(const FunctorBase<T> & wrapped)
    : FunctorBase<T>("wraps_" + wrapped.functorName()), FunctorEnvelopeBase(), _wrapped(&wrapped)
  {
  }

  /**
   * @param wrapped A unique pointer around the functor to wrap. We *will* own the wrapped object,
   * e.g. if we are ever destructed or we are reassigned to wrap another functor, then this functor
   * will be destructed
   */
  FunctorEnvelope(std::unique_ptr<FunctorBase<T>> && wrapped)
    : FunctorBase<T>("wraps_" + wrapped->functorName()),
      FunctorEnvelopeBase(),
      _owned(std::move(wrapped)),
      _wrapped(_owned.get())
  {
  }

  /**
   * Prevent wrapping of a temporary object. If we are to own a functor, the unique_ptr constructor
   * overload should be used
   */
  FunctorEnvelope(FunctorBase<T> &&) = delete;

  /**
   * @param wrapped The functor to wrap. We will *not* not own the wrapped object. If we previously
   * owned a functor, it will be destructed
   */
  void assign(const FunctorBase<T> & wrapped)
  {
    _owned.reset();
    _wrapped = &wrapped;
  }

  /**
   * @param wrapped A unique pointer around the functor to wrap. We *will* own the wrapped object.
   * If we previously owned a functor, it will be destructed
   */
  void assign(std::unique_ptr<FunctorBase<T>> && wrapped)
  {
    _owned = std::move(wrapped);
    _wrapped = _owned.get();
  }

  /**
   * Prevent wrapping of a temporary object. If we are to own a functor, the unique_ptr assign
   * overload should be used
   */
  void assign(FunctorBase<T> &&) = delete;

  FunctorEnvelope(const FunctorEnvelope &) = delete;
  FunctorEnvelope(FunctorEnvelope &&) = delete;
  FunctorEnvelope & operator=(const FunctorEnvelope &) = delete;
  FunctorEnvelope & operator=(FunctorEnvelope &&) = delete;

  virtual ~FunctorEnvelope() = default;

  /**
   * @return whether this object wraps a null functor
   */
  virtual bool wrapsNull() const override { return wrapsType<NullFunctor<T>>(); }

  /**
   * @return a string representation of the return type of this functor
   */
  virtual std::string returnType() const override { return libMesh::demangle(typeid(T).name()); }

  virtual bool ownsWrappedFunctor() const override { return _owned.get(); }

  /**
   * @return whether the wrapped object is of the requested type
   */
  template <typename T2>
  bool wrapsType() const
  {
    return dynamic_cast<const T2 *>(_wrapped);
  }

  virtual bool isExtrapolatedBoundaryFace(const FaceInfo & fi,
                                          const Elem * const elem,
                                          const StateArg & state) const override
  {
    return _wrapped->isExtrapolatedBoundaryFace(fi, elem, state);
  }
  virtual bool isConstant() const override { return _wrapped->isConstant(); }
  virtual bool hasBlocks(const SubdomainID id) const override { return _wrapped->hasBlocks(id); }
  virtual bool hasFaceSide(const FaceInfo & fi, const bool fi_elem_side) const override
  {
    return _wrapped->hasFaceSide(fi, fi_elem_side);
  }

  bool supportsFaceArg() const override final { return true; }
  bool supportsElemSideQpArg() const override final { return true; }

protected:
  ///@{
  /**
   * Forward calls to wrapped object
   */
  virtual ValueType evaluate(const ElemArg & elem, const StateArg & state) const override
  {
    return _wrapped->operator()(elem, state);
  }
  virtual ValueType evaluate(const FaceArg & face, const StateArg & state) const override
  {
    return _wrapped->operator()(face, state);
  }
  virtual ValueType evaluate(const ElemQpArg & qp, const StateArg & state) const override
  {
    return _wrapped->operator()(qp, state);
  }
  virtual ValueType evaluate(const ElemSideQpArg & qp, const StateArg & state) const override
  {
    return _wrapped->operator()(qp, state);
  }
  virtual ValueType evaluate(const ElemPointArg & elem_point, const StateArg & state) const override
  {
    return _wrapped->operator()(elem_point, state);
  }
  virtual ValueType evaluate(const NodeArg & node, const StateArg & state) const override
  {
    return _wrapped->operator()(node, state);
  }

  virtual GradientType evaluateGradient(const ElemArg & elem, const StateArg & state) const override
  {
    return _wrapped->gradient(elem, state);
  }
  virtual GradientType evaluateGradient(const FaceArg & face, const StateArg & state) const override
  {
    return _wrapped->gradient(face, state);
  }
  virtual GradientType evaluateGradient(const ElemQpArg & qp, const StateArg & state) const override
  {
    return _wrapped->gradient(qp, state);
  }
  virtual GradientType evaluateGradient(const ElemSideQpArg & qp,
                                        const StateArg & state) const override
  {
    return _wrapped->gradient(qp, state);
  }
  virtual GradientType evaluateGradient(const ElemPointArg & elem_point,
                                        const StateArg & state) const override
  {
    return _wrapped->gradient(elem_point, state);
  }
  virtual GradientType evaluateGradient(const NodeArg & node, const StateArg & state) const override
  {
    return _wrapped->gradient(node, state);
  }

  virtual DotType evaluateDot(const ElemArg & elem, const StateArg & state) const override
  {
    return _wrapped->dot(elem, state);
  }
  virtual DotType evaluateDot(const FaceArg & face, const StateArg & state) const override
  {
    return _wrapped->dot(face, state);
  }
  virtual DotType evaluateDot(const ElemQpArg & qp, const StateArg & state) const override
  {
    return _wrapped->dot(qp, state);
  }
  virtual DotType evaluateDot(const ElemSideQpArg & qp, const StateArg & state) const override
  {
    return _wrapped->dot(qp, state);
  }
  virtual DotType evaluateDot(const ElemPointArg & elem_point,
                              const StateArg & state) const override
  {
    return _wrapped->dot(elem_point, state);
  }
  virtual DotType evaluateDot(const NodeArg & node, const StateArg & state) const override
  {
    return _wrapped->dot(node, state);
  }

  virtual GradientType evaluateGradDot(const ElemArg & elem, const StateArg & state) const override
  {
    return _wrapped->gradDot(elem, state);
  }
  virtual GradientType evaluateGradDot(const FaceArg & face, const StateArg & state) const override
  {
    return _wrapped->gradDot(face, state);
  }
  virtual GradientType evaluateGradDot(const ElemQpArg & qp, const StateArg & state) const override
  {
    return _wrapped->gradDot(qp, state);
  }
  virtual GradientType evaluateGradDot(const ElemSideQpArg & qp,
                                       const StateArg & state) const override
  {
    return _wrapped->gradDot(qp, state);
  }
  virtual GradientType evaluateGradDot(const ElemPointArg & elem_point,
                                       const StateArg & state) const override
  {
    return _wrapped->gradDot(elem_point, state);
  }
  virtual GradientType evaluateGradDot(const NodeArg & node, const StateArg & state) const override
  {
    return _wrapped->gradDot(node, state);
  }
  ///@}

private:
  /// Our wrapped object
  std::unique_ptr<FunctorBase<T>> _owned;
  const FunctorBase<T> * _wrapped;

  friend class ::SubProblem;
};

/**
 * Class template for creating constant functors
 */
template <typename T>
class ConstantFunctor final : public FunctorBase<T>
{
public:
  using typename FunctorBase<T>::FunctorType;
  using typename FunctorBase<T>::ValueType;
  using typename FunctorBase<T>::GradientType;
  using typename FunctorBase<T>::DotType;

  ConstantFunctor(const ValueType & value)
    : FunctorBase<T>("constant_" + std::to_string(value)), _value(value)
  {
  }
  ConstantFunctor(ValueType && value)
    : FunctorBase<T>("constant_" + std::to_string(MetaPhysicL::raw_value(value))), _value(value)
  {
  }

  virtual bool isConstant() const override { return true; }

  bool hasBlocks(SubdomainID /* id */) const override { return true; }

  bool supportsFaceArg() const override final { return true; }
  bool supportsElemSideQpArg() const override final { return true; }

private:
  ValueType evaluate(const ElemArg &, const StateArg &) const override { return _value; }
  ValueType evaluate(const FaceArg &, const StateArg &) const override { return _value; }
  ValueType evaluate(const ElemQpArg &, const StateArg &) const override { return _value; }
  ValueType evaluate(const ElemSideQpArg &, const StateArg &) const override { return _value; }
  ValueType evaluate(const ElemPointArg &, const StateArg &) const override { return _value; }
  ValueType evaluate(const NodeArg &, const StateArg &) const override { return _value; }

  GradientType evaluateGradient(const ElemArg &, const StateArg &) const override { return 0; }
  GradientType evaluateGradient(const FaceArg &, const StateArg &) const override { return 0; }
  GradientType evaluateGradient(const ElemQpArg &, const StateArg &) const override { return 0; }
  GradientType evaluateGradient(const ElemSideQpArg &, const StateArg &) const override
  {
    return 0;
  }
  GradientType evaluateGradient(const ElemPointArg &, const StateArg &) const override { return 0; }
  GradientType evaluateGradient(const NodeArg &, const StateArg &) const override { return 0; }

  DotType evaluateDot(const ElemArg &, const StateArg &) const override { return 0; }
  DotType evaluateDot(const FaceArg &, const StateArg &) const override { return 0; }
  DotType evaluateDot(const ElemQpArg &, const StateArg &) const override { return 0; }
  DotType evaluateDot(const ElemSideQpArg &, const StateArg &) const override { return 0; }
  DotType evaluateDot(const ElemPointArg &, const StateArg &) const override { return 0; }
  DotType evaluateDot(const NodeArg &, const StateArg &) const override { return 0; }

  GradientType evaluateGradDot(const ElemArg &, const StateArg &) const override { return 0; }
  GradientType evaluateGradDot(const FaceArg &, const StateArg &) const override { return 0; }
  GradientType evaluateGradDot(const ElemQpArg &, const StateArg &) const override { return 0; }
  GradientType evaluateGradDot(const ElemSideQpArg &, const StateArg &) const override { return 0; }
  GradientType evaluateGradDot(const ElemPointArg &, const StateArg &) const override { return 0; }
  GradientType evaluateGradDot(const NodeArg &, const StateArg &) const override { return 0; }

private:
  ValueType _value;
};

/**
 * A functor that serves as a placeholder during the simulation setup phase if a functor consumer
 * requests a functor that has not yet been constructed.
 */
template <typename T>
class NullFunctor final : public FunctorBase<T>
{
public:
  using typename FunctorBase<T>::FunctorType;
  using typename FunctorBase<T>::ValueType;
  using typename FunctorBase<T>::GradientType;
  using typename FunctorBase<T>::DotType;

  NullFunctor() : FunctorBase<T>("null") {}

  // For backwards compatiblity of unit testing
  bool hasFaceSide(const FaceInfo & fi, bool) const override;

  bool supportsFaceArg() const override final { return false; }
  bool supportsElemSideQpArg() const override final { return false; }

private:
  ValueType evaluate(const ElemArg &, const StateArg &) const override
  {
    mooseError("We should never get here. If you have, contact a MOOSE developer and tell them "
               "they've written broken code");
  }
  ValueType evaluate(const FaceArg &, const StateArg &) const override
  {
    mooseError("We should never get here. If you have, contact a MOOSE developer and tell them "
               "they've written broken code");
  }
  ValueType evaluate(const ElemQpArg &, const StateArg &) const override
  {
    mooseError("We should never get here. If you have, contact a MOOSE developer and tell them "
               "they've written broken code");
  }
  ValueType evaluate(const ElemSideQpArg &, const StateArg &) const override
  {
    mooseError("We should never get here. If you have, contact a MOOSE developer and tell them "
               "they've written broken code");
  }
  ValueType evaluate(const ElemPointArg &, const StateArg &) const override
  {
    mooseError("We should never get here. If you have, contact a MOOSE developer and tell them "
               "they've written broken code");
  }
  ValueType evaluate(const NodeArg &, const StateArg &) const override
  {
    mooseError("We should never get here. If you have, contact a MOOSE developer and tell them "
               "they've written broken code");
  }
};

template <typename T>
bool
NullFunctor<T>::hasFaceSide(const FaceInfo &, const bool) const
{
  // For backwards compatiblity of unit testing
  return true;
}
}
