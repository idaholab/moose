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
#include "MooseMesh.h"
#include "MooseTypes.h"
#include "MooseError.h"
#include "Limiter.h"

#include "libmesh/elem.h"
#include "libmesh/quadrature.h"
#include "libmesh/remote_elem.h"
#include "libmesh/tensor_tools.h"

#include <unordered_map>
#include <functional>

class FaceInfo;

namespace Moose
{
/**
 * A structure that is used to evaluate Moose functors logically at an element/cell center
 */
struct ElemArg
{
  const libMesh::Elem * elem;
  bool correct_skewness;
  bool apply_gradient_to_skewness;
};

/**
 * People should think of this geometric argument to Moose functors as corresponding to the location
 * in space of the provided element centroid, \b not as corresponding to the location of the
 * provided face information.
 */
struct ElemFromFaceArg
{
  /// an element, whose centroid we should think of as the evaluation point. It is possible that
  /// the element will be a nullptr in which case, the evaluation point should be thought of as
  /// the location of a ghosted element centroid
  const libMesh::Elem * elem;

  /// a face information object. When the provided element is null or for instance when the
  /// functor is a variable that does not exist on the provided element subdomain, this face
  /// information object will be used to help construct a ghost value evaluation
  const FaceInfo * fi;

  /// Whether to apply skew correction weights
  bool correct_skewness;

  /// Whether to apply the face gradient when computing a skew corrected face value. A true value
  /// for this data member in conjunction with a false value for \p correct_skewness does not make
  /// sense
  bool apply_gradient_to_skewness;

  /// a subdomain ID. This is useful when the functor is a material property and the user wants
  /// to indicate which material property definition should be used to evaluate the functor. For
  /// instance if we are using a flux kernel that is not defined on one side of the face, the
  /// subdomain ID will allow us to compute a ghost material property evaluation
  SubdomainID sub_id;

  /**
   * Make a \p ElemArg from our data
   */
  ElemArg makeElem() const { return {elem, correct_skewness, apply_gradient_to_skewness}; }
};

/**
 * A structure defining a "face" evaluation calling argument for Moose functors
 */
struct FaceArg
{
  /// a face information object which defines our location in space
  const FaceInfo * fi;

  /// a limiter which defines how the functor evaluated on either side of the face should be
  /// interpolated to the face
  Moose::FV::LimiterType limiter_type;

  /// a boolean which states whether the face information element is upwind of the face
  bool elem_is_upwind;

  /// Whether to apply skew correction weights
  bool correct_skewness;

  /// Whether to apply the face gradient when computing a skew corrected face value. A true value
  /// for this data member in conjunction with a false value for \p correct_skewness does not make
  /// sense
  bool apply_gradient_to_skewness;

  ///@{
  /**
   * a pair of subdomain IDs. These do not always correspond to the face info element subdomain
   * ID and face info neighbor subdomain ID. For instance if a flux kernel is operating at a
   * subdomain boundary on which the kernel is defined on one side but not the other, the
   * passed-in subdomain IDs will both correspond to the subdomain ID that the flux kernel is
   * defined on
   */
  SubdomainID elem_sub_id;
  SubdomainID neighbor_sub_id;
  ///@}

  /**
   * Make a \p ElemArg from our data using the face information element
   */
  ElemArg makeElem() const { return {&fi->elem(), correct_skewness, apply_gradient_to_skewness}; }

  /**
   * Make a \p ElemArg from our data using the face information neighbor
   */
  ElemArg makeNeighbor() const
  {
    return {fi->neighborPtr(), correct_skewness, apply_gradient_to_skewness};
  }

  /**
   * Make a \p ElemFromFaceArg from our data using the face information element
   */
  ElemFromFaceArg elemFromFace() const
  {
    return {&fi->elem(), fi, correct_skewness, apply_gradient_to_skewness, elem_sub_id};
  }

  /**
   * Make a \p ElemFromFaceArg from our data using the face information neighbor
   */
  ElemFromFaceArg neighborFromFace() const
  {
    return {fi->neighborPtr(), fi, correct_skewness, apply_gradient_to_skewness, neighbor_sub_id};
  }
};

/**
 * A structure defining a "single-sided face" evaluation argument for Moose functors. This is
 * identical to the \p FaceArg argument with the exception that a single SubdomainID is given as the
 * last argument to the tuple. This allows disambiguation, in \p FunctorMaterialProperty contexts,
 * of which property definition to use at a face on which there may be different property
 * definitions on either side of the face. Additionally this overload can be useful when on an
 * external boundary face when we want to avoid using ghost information
 */
struct SingleSidedFaceArg
{
  /// a face information object which defines our location in space
  const FaceInfo * fi;

  /// a limiter which defines how the functor evaluated on this side of the face should be
  /// interpolated to the face
  Moose::FV::LimiterType limiter_type;

  /// a boolean which states whether the face information element is upwind of the face
  bool elem_is_upwind;

  /// Whether to apply skew correction weights
  bool correct_skewness;

  /// Whether to apply the face gradient when computing a skew corrected face value. A true value
  /// for this data member in conjunction with a false value for \p correct_skewness does not make
  /// sense
  bool apply_gradient_to_skewness;

  /// The subdomain ID which denotes the side of the face information we are evaluating on
  SubdomainID sub_id;

  /**
   * Make a \p ElemArg from our data using the face information element
   */
  ElemArg makeElem() const { return {&fi->elem(), correct_skewness, apply_gradient_to_skewness}; }

  /**
   * Make a \p ElemArg from our data using the face information neighbor
   */
  ElemArg makeNeighbor() const
  {
    return {fi->neighborPtr(), correct_skewness, apply_gradient_to_skewness};
  }
};

/**
 * Argument for requesting functor evaluation at a quadrature point location in an element. Data
 * in the argument:
 * - The element containing the quadrature point
 * - The quadrature point index, e.g. if there are \p n quadrature points, we are requesting the\n
 *   evaluation of the ith point
 * - The quadrature rule that can be used to initialize the functor on the given element
 */
using ElemQpArg = std::tuple<const libMesh::Elem *, unsigned int, const QBase *>;

/**
 * Argument for requesting functor evaluation at quadrature point locations on an element side.
 * Data in the argument:
 * - The element
 * - The element side on which the quadrature points are located
 * - The quadrature point index, e.g. if there are \p n quadrature points, we are requesting the\n
 *   evaluation of the ith point
 * - The quadrature rule that can be used to initialize the functor on the given element and side
 */
using ElemSideQpArg = std::tuple<const libMesh::Elem *, unsigned int, unsigned int, const QBase *>;

/**
 * Base class template for functor objects. This class template defines various \p operator()
 * overloads that allow a user to evaluate the functor at arbitrary geometric locations. This
 * template is meant to enable highly flexible on-the-fly variable and material property
 * evaluations
 */
template <typename T>
class FunctorBase
{
public:
  using FunctorType = FunctorBase<T>;
  using FunctorReturnType = T;
  using ValueType = T;
  using GradientType = typename libMesh::TensorTools::IncrementRank<T>::type;
  using DotType = ValueType;

  virtual ~FunctorBase() = default;
  FunctorBase() : _clearance_schedule({EXEC_ALWAYS}) {}
  FunctorBase(const std::set<ExecFlagType> & clearance_schedule)
    : _clearance_schedule(clearance_schedule)
  {
  }

  ///@{
  /**
   * Same as their \p evaluate overloads with the same arguments but allows for caching
   * implementation. These are the methods a user will call in their code
   */
  ValueType operator()(const ElemArg & elem, unsigned int state = 0) const;
  ValueType operator()(const ElemFromFaceArg & elem_from_face, unsigned int state = 0) const;
  ValueType operator()(const FaceArg & face, unsigned int state = 0) const;
  ValueType operator()(const SingleSidedFaceArg & face, unsigned int state = 0) const;
  ValueType operator()(const ElemQpArg & qp, unsigned int state = 0) const;
  ValueType operator()(const ElemSideQpArg & qp, unsigned int state = 0) const;
  ///@}

  ///@{
  /**
   * Same as their \p evaluateGradient overloads with the same arguments but allows for caching
   * implementation. These are the methods a user will call in their code
   */
  GradientType gradient(const ElemArg & elem, unsigned int state = 0) const;
  GradientType gradient(const ElemFromFaceArg & elem_from_face, unsigned int state = 0) const;
  GradientType gradient(const FaceArg & face, unsigned int state = 0) const;
  GradientType gradient(const SingleSidedFaceArg & face, unsigned int state = 0) const;
  GradientType gradient(const ElemQpArg & qp, unsigned int state = 0) const;
  GradientType gradient(const ElemSideQpArg & qp, unsigned int state = 0) const;
  ///@}

  ///@{
  /**
   * Same as their \p evaluateDot overloads with the same arguments but allows for caching
   * implementation. These are the methods a user will call in their code
   */
  DotType dot(const ElemArg & elem, unsigned int state = 0) const;
  DotType dot(const ElemFromFaceArg & elem_from_face, unsigned int state = 0) const;
  DotType dot(const FaceArg & face, unsigned int state = 0) const;
  DotType dot(const SingleSidedFaceArg & face, unsigned int state = 0) const;
  DotType dot(const ElemQpArg & qp, unsigned int state = 0) const;
  DotType dot(const ElemSideQpArg & qp, unsigned int state = 0) const;
  ///@}

  virtual void residualSetup();
  virtual void jacobianSetup();
  virtual void timestepSetup();

  /**
   * Set how often to clear the functor evaluation cache
   */
  void setCacheClearanceSchedule(const std::set<ExecFlagType> & clearance_schedule);

  /**
   * Returns whether this face is an extrapolated boundary face for this functor
   */
  virtual bool isExtrapolatedBoundaryFace(const FaceInfo &) const { mooseError("not implemented"); }

protected:
  /**
   * Evaluate the functor with a given element. Some example implementations of this method
   * could compute an element-average or evaluate at the element centroid
   */
  virtual ValueType evaluate(const ElemArg & elem, unsigned int state) const = 0;

  /**
   * @param elem_from_face See the \p ElemFromFaceArg doxygen
   * @param state Corresponds to a time argument. A value of 0 corresponds to current time, 1
   * corresponds to the old time, 2 corresponds to the older time, etc.
   * @return The functor evaluated at the requested time and space
   */
  virtual ValueType evaluate(const ElemFromFaceArg & elem_from_face, unsigned int state) const = 0;

  /**
   * @param face See the \p FaceArg doxygen
   * @param state Corresponds to a time argument. A value of 0 corresponds to current time, 1
   * corresponds to the old time, 2 corresponds to the older time, etc.
   * @return The functor evaluated at the requested time and space
   */
  virtual ValueType evaluate(const FaceArg & face, unsigned int state) const = 0;

  /**
   * @param face See the \p SingleSidedFaceArg doxygen
   * @param state Corresponds to a time argument. A value of 0 corresponds to current time, 1
   * corresponds to the old time, 2 corresponds to the older time, etc.
   * @return The functor evaluated at the requested time and space
   */
  virtual ValueType evaluate(const SingleSidedFaceArg & face, unsigned int state) const = 0;

  /**
   * @param qp See the \p ElemQpArg doxygen
   * @param state Corresponds to a time argument. A value of 0 corresponds to current time, 1
   * corresponds to the old time, 2 corresponds to the older time, etc.
   * @return The functor evaluated at the requested time and space
   */
  virtual ValueType evaluate(const ElemQpArg & qp, unsigned int state) const = 0;

  /**
   * @param side_qp See the \p ElemSideQpArg doxygen
   * @param state Corresponds to a time argument. A value of 0 corresponds to current time, 1
   * corresponds to the old time, 2 corresponds to the older time, etc.
   * @return The functor evaluated at the requested time and space
   */
  virtual ValueType evaluate(const ElemSideQpArg & side_qp, unsigned int state) const = 0;

  /**
   * Evaluate the functor gradient with a given element. Some example implementations of this
   * method could compute an element-average or evaluate at the element centroid
   */
  virtual GradientType evaluateGradient(const ElemArg &, unsigned int) const
  {
    mooseError("not implemented");
  }

  /**
   * @param elem_from_face See the \p ElemFromFaceArg doxygen
   * @param state Corresponds to a time argument. A value of 0 corresponds to current time, 1
   * corresponds to the old time, 2 corresponds to the older time, etc.
   * @return The functor gradient evaluated at the requested time and space
   */
  virtual GradientType evaluateGradient(const ElemFromFaceArg &, unsigned int) const
  {
    mooseError("not implemented");
  }

  /**
   * @param face See the \p FaceArg doxygen
   * @param state Corresponds to a time argument. A value of 0 corresponds to current time, 1
   * corresponds to the old time, 2 corresponds to the older time, etc.
   * @return The functor gradient evaluated at the requested time and space
   */
  virtual GradientType evaluateGradient(const FaceArg &, unsigned int) const
  {
    mooseError("not implemented");
  }

  /**
   * @param face See the \p SingleSidedFaceArg doxygen
   * @param state Corresponds to a time argument. A value of 0 corresponds to current time, 1
   * corresponds to the old time, 2 corresponds to the older time, etc.
   * @return The functor gradient evaluated at the requested time and space
   */
  virtual GradientType evaluateGradient(const SingleSidedFaceArg &, unsigned int) const
  {
    mooseError("not implemented");
  }

  /**
   * @param qp See the \p ElemQpArg doxygen
   * @param state Corresponds to a time argument. A value of 0 corresponds to current time, 1
   * corresponds to the old time, 2 corresponds to the older time, etc.
   * @return The functor gradient evaluated at the requested time and space
   */
  virtual GradientType evaluateGradient(const ElemQpArg &, unsigned int) const
  {
    mooseError("not implemented");
  }

  /**
   * @param side_qp See the \p ElemSideQpArg doxygen
   * @param state Corresponds to a time argument. A value of 0 corresponds to current time, 1
   * corresponds to the old time, 2 corresponds to the older time, etc.
   * @return The functor gradient evaluated at the requested time and space
   */
  virtual GradientType evaluateGradient(const ElemSideQpArg &, unsigned int) const
  {
    mooseError("not implemented");
  }

  /**
   * Evaluate the functor time derivative with a given element. Some example implementations of
   * this method could compute an element-average or evaluate at the element centroid
   */
  virtual DotType evaluateDot(const ElemArg &, unsigned int) const
  {
    mooseError("not implemented");
  }

  /**
   * @param elem_from_face See the \p ElemFromFaceArg doxygen
   * @param state Corresponds to a time argument. A value of 0 corresponds to current time, 1
   * corresponds to the old time, 2 corresponds to the older time, etc.
   * @return The functor time derivative evaluated at the requested time and space
   */
  virtual DotType evaluateDot(const ElemFromFaceArg &, unsigned int) const
  {
    mooseError("not implemented");
  }

  /**
   * @param face See the \p FaceArg doxygen
   * @param state Corresponds to a time argument. A value of 0 corresponds to current time, 1
   * corresponds to the old time, 2 corresponds to the older time, etc.
   * @return The functor time derivative evaluated at the requested time and space
   */
  virtual DotType evaluateDot(const FaceArg &, unsigned int) const
  {
    mooseError("not implemented");
  }

  /**
   * @param face See the \p SingleSidedFaceArg doxygen
   * @param state Corresponds to a time argument. A value of 0 corresponds to current time, 1
   * corresponds to the old time, 2 corresponds to the older time, etc.
   * @return The functor time derivative evaluated at the requested time and space
   */
  virtual DotType evaluateDot(const SingleSidedFaceArg &, unsigned int) const
  {
    mooseError("not implemented");
  }

  /**
   * @param qp See the \p ElemQpArg doxygen
   * @param state Corresponds to a time argument. A value of 0 corresponds to current time, 1
   * corresponds to the old time, 2 corresponds to the older time, etc.
   * @return The functor time derivative evaluated at the requested time and space
   */
  virtual DotType evaluateDot(const ElemQpArg &, unsigned int) const
  {
    mooseError("not implemented");
  }

  /**
   * @param side_qp See the \p ElemSideQpArg doxygen
   * @param state Corresponds to a time argument. A value of 0 corresponds to current time, 1
   * corresponds to the old time, 2 corresponds to the older time, etc.
   * @return The functor time derivative evaluated at the requested time and space
   */
  virtual DotType evaluateDot(const ElemSideQpArg &, unsigned int) const
  {
    mooseError("not implemented");
  }

private:
  /**
   * clear cache data
   */
  void clearCacheData();

  /**
   * check a qp cache and if invalid then evaluate
   */
  template <typename SpaceArg, typename TimeArg>
  ValueType queryQpCache(unsigned int qp,
                         const QBase & qrule,
                         std::vector<std::pair<bool, T>> & qp_cache_data,
                         const SpaceArg & space,
                         const TimeArg & time) const;

  /// How often to clear the material property cache
  std::set<ExecFlagType> _clearance_schedule;

  // Data for traditional element-quadrature point property evaluations which are useful for
  // caching implementation

  /// Current key for qp map cache
  mutable dof_id_type _current_qp_map_key = DofObject::invalid_id;

  /// Current value for qp map cache
  mutable std::vector<std::pair<bool, T>> * _current_qp_map_value = nullptr;

  /// Cached element quadrature point functor property evaluations. The map key is the element
  /// id. The map values should have size corresponding to the number of quadrature points on the
  /// element. The vector elements are pairs. The first member of the pair indicates whether a
  /// cached value has been computed. The second member of the pair is the (cached) value. If the
  /// boolean is false, then the value cannot be trusted
  mutable std::unordered_map<dof_id_type, std::vector<std::pair<bool, T>>> _qp_to_value;

  // Data for traditional element-side-quadrature point property evaluations which are useful for
  // caching implementation

  /// Current key for side-qp map cache
  mutable dof_id_type _current_side_qp_map_key = DofObject::invalid_id;

  /// Current value for side-qp map cache
  mutable std::vector<std::vector<std::pair<bool, T>>> * _current_side_qp_map_value = nullptr;

  /// Cached element quadrature point functor property evaluations. The map key is the element
  /// id. The map values are a multi-dimensional vector (or vector of vectors) with the first index
  /// corresponding to the side and the second index corresponding to the quadrature point
  /// index. The elements returned after double indexing are pairs. The first member of the pair
  /// indicates whether a cached value has been computed. The second member of the pair is the
  /// (cached) value. If the boolean is false, then the value cannot be trusted
  mutable std::unordered_map<dof_id_type, std::vector<std::vector<std::pair<bool, T>>>>
      _side_qp_to_value;
};

template <typename T>
typename FunctorBase<T>::ValueType
FunctorBase<T>::operator()(const ElemArg & elem, const unsigned int state) const
{
  return evaluate(elem, state);
}

template <typename T>
typename FunctorBase<T>::ValueType
FunctorBase<T>::operator()(const ElemFromFaceArg & elem_from_face, const unsigned int state) const
{
  return evaluate(elem_from_face, state);
}

template <typename T>
typename FunctorBase<T>::ValueType
FunctorBase<T>::operator()(const FaceArg & face, const unsigned int state) const
{
  return evaluate(face, state);
}

template <typename T>
typename FunctorBase<T>::ValueType
FunctorBase<T>::operator()(const SingleSidedFaceArg & face, const unsigned int state) const
{
  return evaluate(face, state);
}

template <typename T>
template <typename SpaceArg, typename TimeArg>
typename FunctorBase<T>::ValueType
FunctorBase<T>::queryQpCache(const unsigned int qp,
                             const QBase & qrule,
                             std::vector<std::pair<bool, T>> & qp_cache_data,
                             const SpaceArg & space,
                             const TimeArg & time) const
{
  // Check and see whether we even have sized for this quadrature point. If we haven't then we
  // must evaluate
  if (qp >= qp_cache_data.size())
  {
    qp_cache_data.resize(qrule.n_points(), std::make_pair(false, T()));
    auto & pr = qp_cache_data[qp];
    pr.second = evaluate(space, time);
    pr.first = true;
    return pr.second;
  }

  // We've already sized for this qp, so let's see whether we have a valid cache value
  auto & pr = qp_cache_data[qp];
  if (pr.first)
    return pr.second;

  // No valid cache value so evaluate
  pr.second = evaluate(space, time);
  pr.first = true;
  return pr.second;
}

template <typename T>
typename FunctorBase<T>::ValueType
FunctorBase<T>::operator()(const ElemQpArg & elem_qp, const unsigned int state) const
{
  if (_clearance_schedule.count(EXEC_ALWAYS))
    return evaluate(elem_qp, state);

  const auto elem_id = std::get<0>(elem_qp)->id();
  if (elem_id != _current_qp_map_key)
  {
    _current_qp_map_key = elem_id;
    _current_qp_map_value = &_qp_to_value[elem_id];
  }
  auto & qp_data = *_current_qp_map_value;
  const auto qp = std::get<1>(elem_qp);
  const auto * const qrule = std::get<2>(elem_qp);
  mooseAssert(qrule, "qrule must be non-null");

  return queryQpCache(qp, *qrule, qp_data, elem_qp, state);
}

template <typename T>
typename FunctorBase<T>::ValueType
FunctorBase<T>::operator()(const ElemSideQpArg & elem_side_qp, const unsigned int state) const
{
  if (_clearance_schedule.count(EXEC_ALWAYS))
    return evaluate(elem_side_qp, state);

  const Elem * const elem = std::get<0>(elem_side_qp);
  mooseAssert(elem, "elem must be non-null");
  const auto elem_id = elem->id();
  if (elem_id != _current_side_qp_map_key)
  {
    _current_side_qp_map_key = elem_id;
    _current_side_qp_map_value = &_side_qp_to_value[elem_id];
  }
  auto & side_qp_data = *_current_side_qp_map_value;
  const auto side = std::get<1>(elem_side_qp);
  const auto qp = std::get<2>(elem_side_qp);
  const auto * const qrule = std::get<3>(elem_side_qp);
  mooseAssert(qrule, "qrule must be non-null");

  // Check and see whether we even have sized for this side
  if (side >= side_qp_data.size())
    side_qp_data.resize(elem->n_sides());

  // Ok we were sized enough for our side
  auto & qp_data = side_qp_data[side];
  return queryQpCache(qp, *qrule, qp_data, elem_side_qp, state);
}

template <typename T>
void
FunctorBase<T>::setCacheClearanceSchedule(const std::set<ExecFlagType> & clearance_schedule)
{
  _clearance_schedule = clearance_schedule;
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

  _current_qp_map_key = DofObject::invalid_id;
  _current_qp_map_value = nullptr;
  _current_side_qp_map_key = DofObject::invalid_id;
  _current_side_qp_map_value = nullptr;
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
typename FunctorBase<T>::GradientType
FunctorBase<T>::gradient(const ElemArg & elem, const unsigned int state) const
{
  return evaluateGradient(elem, state);
}

template <typename T>
typename FunctorBase<T>::GradientType
FunctorBase<T>::gradient(const ElemFromFaceArg & elem_from_face, const unsigned int state) const
{
  return evaluateGradient(elem_from_face, state);
}

template <typename T>
typename FunctorBase<T>::GradientType
FunctorBase<T>::gradient(const FaceArg & face, const unsigned int state) const
{
  return evaluateGradient(face, state);
}

template <typename T>
typename FunctorBase<T>::GradientType
FunctorBase<T>::gradient(const SingleSidedFaceArg & face, const unsigned int state) const
{
  return evaluateGradient(face, state);
}

template <typename T>
typename FunctorBase<T>::GradientType
FunctorBase<T>::gradient(const ElemQpArg & elem_qp, const unsigned int state) const
{
  return evaluateGradient(elem_qp, state);
}

template <typename T>
typename FunctorBase<T>::GradientType
FunctorBase<T>::gradient(const ElemSideQpArg & elem_side_qp, const unsigned int state) const
{
  return evaluateGradient(elem_side_qp, state);
}

template <typename T>
typename FunctorBase<T>::DotType
FunctorBase<T>::dot(const ElemArg & elem, const unsigned int state) const
{
  return evaluateDot(elem, state);
}

template <typename T>
typename FunctorBase<T>::DotType
FunctorBase<T>::dot(const ElemFromFaceArg & elem_from_face, const unsigned int state) const
{
  return evaluateDot(elem_from_face, state);
}

template <typename T>
typename FunctorBase<T>::DotType
FunctorBase<T>::dot(const FaceArg & face, const unsigned int state) const
{
  return evaluateDot(face, state);
}

template <typename T>
typename FunctorBase<T>::DotType
FunctorBase<T>::dot(const SingleSidedFaceArg & face, const unsigned int state) const
{
  return evaluateDot(face, state);
}

template <typename T>
typename FunctorBase<T>::DotType
FunctorBase<T>::dot(const ElemQpArg & elem_qp, const unsigned int state) const
{
  return evaluateDot(elem_qp, state);
}

template <typename T>
typename FunctorBase<T>::DotType
FunctorBase<T>::dot(const ElemSideQpArg & elem_side_qp, const unsigned int state) const
{
  return evaluateDot(elem_side_qp, state);
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

  ///@{
  /**
   * Virtual methods meant to be used for handling functor evaluation cache clearance
   */
  virtual void timestepSetup() = 0;
  virtual void residualSetup() = 0;
  virtual void jacobianSetup() = 0;
  virtual bool wrapsNull() const = 0;
  virtual std::string returnType() const = 0;
  ///@}
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
  FunctorEnvelope(const FunctorBase<T> & wrapped) : FunctorEnvelopeBase(), _wrapped(&wrapped) {}

  /**
   * @param wrapped A unique pointer around the functor to wrap. We *will* own the wrapped object,
   * e.g. if we are ever destructed or we are reassigned to wrap another functor, then this functor
   * will be destructed
   */
  FunctorEnvelope(std::unique_ptr<FunctorBase<T>> && wrapped)
    : FunctorEnvelopeBase(), _owned(std::move(wrapped)), _wrapped(_owned.get())
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
  bool wrapsNull() const override { return wrapsType<NullFunctor<T>>(); }

  /**
   * @return a string representation of the return type of this functor
   */
  std::string returnType() const override { return libMesh::demangle(typeid(T).name()); }

  /**
   * @return whether the wrapped object is of the requested type
   */
  template <typename T2>
  bool wrapsType() const
  {
    return dynamic_cast<const T2 *>(_wrapped);
  }

  void timestepSetup() override
  {
    if (_owned)
      _owned->timestepSetup();
  }
  void residualSetup() override
  {
    if (_owned)
      _owned->residualSetup();
  }
  void jacobianSetup() override
  {
    if (_owned)
      _owned->jacobianSetup();
  }

protected:
  ///@{
  /**
   * Forward calls to wrapped object
   */
  ValueType evaluate(const ElemArg & elem, unsigned int state = 0) const override
  {
    return _wrapped->operator()(elem, state);
  }
  ValueType evaluate(const ElemFromFaceArg & elem_from_face, unsigned int state = 0) const override
  {
    return _wrapped->operator()(elem_from_face, state);
  }
  ValueType evaluate(const FaceArg & face, unsigned int state = 0) const override
  {
    return _wrapped->operator()(face, state);
  }
  ValueType evaluate(const SingleSidedFaceArg & face, unsigned int state = 0) const override
  {
    return _wrapped->operator()(face, state);
  }
  ValueType evaluate(const ElemQpArg & qp, unsigned int state = 0) const override
  {
    return _wrapped->operator()(qp, state);
  }
  ValueType evaluate(const ElemSideQpArg & qp, unsigned int state = 0) const override
  {
    return _wrapped->operator()(qp, state);
  }

  GradientType evaluateGradient(const ElemArg & elem, unsigned int state = 0) const override
  {
    return _wrapped->gradient(elem, state);
  }
  GradientType evaluateGradient(const ElemFromFaceArg & elem_from_face,
                                unsigned int state = 0) const override
  {
    return _wrapped->gradient(elem_from_face, state);
  }
  GradientType evaluateGradient(const FaceArg & face, unsigned int state = 0) const override
  {
    return _wrapped->gradient(face, state);
  }
  GradientType evaluateGradient(const SingleSidedFaceArg & face,
                                unsigned int state = 0) const override
  {
    return _wrapped->gradient(face, state);
  }
  GradientType evaluateGradient(const ElemQpArg & qp, unsigned int state = 0) const override
  {
    return _wrapped->gradient(qp, state);
  }
  GradientType evaluateGradient(const ElemSideQpArg & qp, unsigned int state = 0) const override
  {
    return _wrapped->gradient(qp, state);
  }

  DotType evaluateDot(const ElemArg & elem, unsigned int state = 0) const override
  {
    return _wrapped->dot(elem, state);
  }
  DotType evaluateDot(const ElemFromFaceArg & elem_from_face, unsigned int state = 0) const override
  {
    return _wrapped->dot(elem_from_face, state);
  }
  DotType evaluateDot(const FaceArg & face, unsigned int state = 0) const override
  {
    return _wrapped->dot(face, state);
  }
  DotType evaluateDot(const SingleSidedFaceArg & face, unsigned int state = 0) const override
  {
    return _wrapped->dot(face, state);
  }
  DotType evaluateDot(const ElemQpArg & qp, unsigned int state = 0) const override
  {
    return _wrapped->dot(qp, state);
  }
  DotType evaluateDot(const ElemSideQpArg & qp, unsigned int state = 0) const override
  {
    return _wrapped->dot(qp, state);
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
  using typename FunctorBase<T>::FunctorReturnType;
  using typename FunctorBase<T>::ValueType;
  using typename FunctorBase<T>::GradientType;
  using typename FunctorBase<T>::DotType;

  ConstantFunctor(const ValueType & value) : _value(value) {}
  ConstantFunctor(ValueType && value) : _value(value) {}

private:
  ValueType evaluate(const ElemArg &, unsigned int) const override { return _value; }
  ValueType evaluate(const ElemFromFaceArg &, unsigned int) const override { return _value; }
  ValueType evaluate(const FaceArg &, unsigned int) const override { return _value; }
  ValueType evaluate(const SingleSidedFaceArg &, unsigned int) const override { return _value; }
  ValueType evaluate(const ElemQpArg &, unsigned int) const override { return _value; }
  ValueType evaluate(const ElemSideQpArg &, unsigned int) const override { return _value; }

  GradientType evaluateGradient(const ElemArg &, unsigned int) const override { return 0; }
  GradientType evaluateGradient(const ElemFromFaceArg &, unsigned int) const override { return 0; }
  GradientType evaluateGradient(const FaceArg &, unsigned int) const override { return 0; }
  GradientType evaluateGradient(const SingleSidedFaceArg &, unsigned int) const override
  {
    return 0;
  }
  GradientType evaluateGradient(const ElemQpArg &, unsigned int) const override { return 0; }
  GradientType evaluateGradient(const ElemSideQpArg &, unsigned int) const override { return 0; }

  DotType evaluateDot(const ElemArg &, unsigned int) const override { return 0; }
  DotType evaluateDot(const ElemFromFaceArg &, unsigned int) const override { return 0; }
  DotType evaluateDot(const FaceArg &, unsigned int) const override { return 0; }
  DotType evaluateDot(const SingleSidedFaceArg &, unsigned int) const override { return 0; }
  DotType evaluateDot(const ElemQpArg &, unsigned int) const override { return 0; }
  DotType evaluateDot(const ElemSideQpArg &, unsigned int) const override { return 0; }

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
  using typename FunctorBase<T>::FunctorReturnType;
  using typename FunctorBase<T>::ValueType;
  using typename FunctorBase<T>::GradientType;
  using typename FunctorBase<T>::DotType;

private:
  ValueType evaluate(const ElemArg &, unsigned int) const override
  {
    mooseError("We should never get here. If you have, contact a MOOSE developer and tell them "
               "they've written broken code");
  }
  ValueType evaluate(const ElemFromFaceArg &, unsigned int) const override
  {
    mooseError("We should never get here. If you have, contact a MOOSE developer and tell them "
               "they've written broken code");
  }
  ValueType evaluate(const FaceArg &, unsigned int) const override
  {
    mooseError("We should never get here. If you have, contact a MOOSE developer and tell them "
               "they've written broken code");
  }
  ValueType evaluate(const SingleSidedFaceArg &, unsigned int) const override
  {
    mooseError("We should never get here. If you have, contact a MOOSE developer and tell them "
               "they've written broken code");
  }
  ValueType evaluate(const ElemQpArg &, unsigned int) const override
  {
    mooseError("We should never get here. If you have, contact a MOOSE developer and tell them "
               "they've written broken code");
  }
  ValueType evaluate(const ElemSideQpArg &, unsigned int) const override
  {
    mooseError("We should never get here. If you have, contact a MOOSE developer and tell them "
               "they've written broken code");
  }
};
}
