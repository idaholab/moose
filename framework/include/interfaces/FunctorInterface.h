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

#include "MooseMesh.h"
#include "MooseTypes.h"
#include "MooseError.h"
#include "Limiter.h"

#include "libmesh/elem.h"
#include "libmesh/remote_elem.h"

#include <unordered_map>
#include <functional>

class FaceInfo;

/**
 * A non-templated base class for functors that allow an owner object to hold
 * different class template instantiations of \p FunctorInterface in a single container
 */
class FunctorBase
{
public:
  FunctorBase() = default;
  virtual ~FunctorBase() = default;

  ///@{
  /**
   * Virtual methods meant to be used for handling functor evaluation cache clearance
   */
  virtual void timestepSetup() = 0;
  virtual void residualSetup() = 0;
  virtual void jacobianSetup() = 0;
  ///@}
};

/**
 * Base class template for functor objects. This class template defines various \p operator()
 * overloads that allow a user to evaluate the functor at arbitrary geometric locations. This
 * template is meant to enable highly flexible on-the-fly variable and material property evaluations
 */
template <typename T>
class FunctorInterface : public FunctorBase
{
public:
  /**
   * A typedef defining a "face" evaluation calling argument. This is composed of
   * - a face information object which defines our location in space
   * - a limiter which defines how the functor evaluated on either side of the face should be\n
   *   interpolated to the face
   * - a boolean which states whether the face information element is upwind of the face
   * - a pair of subdomain IDs. These do not always correspond to the face info element subdomain\n
   *   ID and face info neighbor subdomain ID. For instance if a flux kernel is operating at a\n
   *   subdomain boundary on which the kernel is defined on one side but not the other, the\n
   *   passed-in subdomain IDs will both correspond to the subdomain ID that the flux kernel is\n
   *   defined on
   */
  using FaceArg =
      std::tuple<const FaceInfo *,
                 const Moose::FV::Limiter<typename Moose::FV::LimiterValueType<T>::value_type> *,
                 bool,
                 std::pair<SubdomainID, SubdomainID>>;

  /**
   * People should think of this geometric argument as corresponding to the location in space of the
   * provided element centroid, \b not as corresonding the location of the provided face
   * information. Summary of data in this argument:
   * - an element, whose centroid we should think of as the evaluation point. It is possible that\n
   *   the element will be a nullptr in which case, the evaluation point should be thought of as\n
   *   the location of a ghosted element centroid
   * - a face information object. When the provided element is null or for instance when the\n
   *   functoris a variable that does not exist on the provided element subdomain, this face\n
   *   information object will be used to help construct a ghost value evaluation
   * - a subdomain ID. This is useful when the functor is a material property and the user wants\n
   *   to indicate which material property definition should be used to evaluate the functor. For\n
   *   instance if we are using a flux kernel that is not defined on one side of the face, the\n
   *   subdomain ID will allow us to compute a ghost material property evaluation
   */
  using ElemFromFaceArg = std::tuple<const libMesh::Elem *, const FaceInfo *, SubdomainID>;

  /**
   * Argument for requesting functor evaluation at a quadrature point location in an element. Data
   * in the argument:
   * - The element containing the quadrature point
   * - The quadrature point index, e.g. if there are \p n quadrature points, we are requesting the\n
   *   evaluation of the ith point
   * - The quadrature rule that can be used to initialize the functor on the given element
   */
  using QpArg = std::tuple<const libMesh::Elem *, unsigned int, const QBase *>;

  using FunctorType = FunctorInterface<T>;
  using FunctorReturnType = T;
  virtual ~FunctorInterface() = default;
  FunctorInterface() : _clearance_schedule({EXEC_ALWAYS}) {}

  ///@{
  /**
   * Same as their \p evaluate overloads with the same arguments but allows for caching
   * implementation. These are the methods a user will call in their code
   */
  T operator()(const libMesh::Elem * const & elem, unsigned int state = 0) const;
  T operator()(const ElemFromFaceArg & elem_from_face, unsigned int state = 0) const;
  T operator()(const FaceArg & face, unsigned int state = 0) const;
  T operator()(const QpArg & qp, unsigned int state = 0) const;
  T operator()(const std::tuple<Moose::ElementType, unsigned int, SubdomainID> & tqp,
               unsigned int state = 0) const;
  ///@}

  void residualSetup() override;
  void jacobianSetup() override;
  void timestepSetup() override;

  /**
   * Set how often to clear the functor evaluation cache
   */
  void setCacheClearanceSchedule(const std::set<ExecFlagType> & clearance_schedule);

protected:
  /**
   * Evaluate the functor with a given element. Some example implementations of this method
   * could compute an element-average or evaluate at the element centroid
   */
  virtual T evaluate(const libMesh::Elem * const & elem, unsigned int state) const = 0;

  /**
   * @param elem_from_face See the \p ElemFromFaceArg doxygen
   * @param state Corresponds to a time argument. A value of 0 corresponds to current time, 1
   * corresponds to the old time, 2 corresponds to the older time, etc.
   * @return The functor evaluated at the requested time and space
   */
  virtual T evaluate(const ElemFromFaceArg & elem_from_face, unsigned int state) const = 0;

  /**
   * @param face See the \p FaceArg doxygen
   * @param state Corresponds to a time argument. A value of 0 corresponds to current time, 1
   * corresponds to the old time, 2 corresponds to the older time, etc.
   * @return The functor evaluated at the requested time and space
   */
  virtual T evaluate(const FaceArg & face, unsigned int state) const = 0;

  /**
   * @param qp See the \p QpArg doxygen
   * @param state Corresponds to a time argument. A value of 0 corresponds to current time, 1
   * corresponds to the old time, 2 corresponds to the older time, etc.
   * @return The functor evaluated at the requested time and space
   */
  virtual T evaluate(const QpArg & qp, unsigned int state) const = 0;

  /**
   * @param tqp A tuple with the first member corresponding to an \p ElementType, either Element,
   * Neighbor, or Lower corresponding to the three different possible \p MooseVariableData
   * instances. The second member corresponds to the desired quadrature point. The third member
   * corresponds to the subdomain that this functor is being evaluated on
   * @return The requested element type data indexed at the requested quadrature point. There is a
   * caveat with this \p evaluate overload: any variables involved in the functor evaluation must
   * have their requested element data type properly pre-initialized at the desired quadrature point
   */
  virtual T evaluate(const std::tuple<Moose::ElementType, unsigned int, SubdomainID> & tqp,
                     unsigned int state) const = 0;

private:
  /**
   * clear cache data
   */
  void clearCacheData();

  /// How often to clear the material property cache
  std::set<ExecFlagType> _clearance_schedule;

  // Data for traditional element-quadrature point property evaluations which are useful for caching
  // implementation

  /// Current quadrature point element (current cache key)
  mutable dof_id_type _current_qp_elem = DofObject::invalid_id;

  /// Current quadrature point element data (current cache value)
  mutable std::vector<std::pair<bool, T>> * _current_qp_elem_data = nullptr;

  /// Cached element quadrature point functor property evaluations. The map key is the element
  /// id. The map values should have size corresponding to the number of quadrature points on the
  /// element. The vector elements are pairs. The first member of the pair indicates whether a
  /// cached value has been computed. The second member of the pair is the (cached) value. If the
  /// boolean is false, then the value cannot be trusted
  mutable std::unordered_map<dof_id_type, std::vector<std::pair<bool, T>>> _qp_to_value;
};

template <typename T>
T
FunctorInterface<T>::operator()(const Elem * const & elem, const unsigned int state) const
{
  return evaluate(elem, state);
}

template <typename T>
T
FunctorInterface<T>::operator()(const ElemFromFaceArg & elem_from_face,
                                const unsigned int state) const
{
  return evaluate(elem_from_face, state);
}

template <typename T>
T
FunctorInterface<T>::operator()(const FaceArg & face, const unsigned int state) const
{
  return evaluate(face, state);
}

template <typename T>
T
FunctorInterface<T>::operator()(const QpArg & elem_and_qp, const unsigned int state) const
{
  if (_clearance_schedule.count(EXEC_ALWAYS))
    return evaluate(elem_and_qp, state);

  const auto elem_id = std::get<0>(elem_and_qp)->id();
  if (elem_id != _current_qp_elem)
  {
    _current_qp_elem = elem_id;
    _current_qp_elem_data = &_qp_to_value[elem_id];
  }
  auto & qp_elem_data_ref = *_current_qp_elem_data;
  const auto qp = std::get<1>(elem_and_qp);

  // Check and see whether we even have sized for this quadrature point. If we haven't then we must
  // evaluate
  if (qp >= qp_elem_data_ref.size())
  {
    mooseAssert(qp == qp_elem_data_ref.size(),
                "I believe that we always iterate over quadrature points in a contiguous fashion");
    qp_elem_data_ref.resize(qp + 1);
    auto & pr = qp_elem_data_ref.back();
    pr.second = evaluate(elem_and_qp, state);
    pr.first = true;
    return pr.second;
  }

  // We've already sized for this qp, so let's see whether we have a valid cache value
  auto & pr = qp_elem_data_ref[qp];
  if (pr.first)
    return pr.second;

  // No valid cache value so evaluate
  pr.second = evaluate(elem_and_qp, state);
  pr.first = true;
  return pr.second;
}

template <typename T>
T
FunctorInterface<T>::operator()(
    const std::tuple<Moose::ElementType, unsigned int, SubdomainID> & tqp,
    const unsigned int state) const
{
  return evaluate(tqp, state);
}

template <typename T>
void
FunctorInterface<T>::setCacheClearanceSchedule(const std::set<ExecFlagType> & clearance_schedule)
{
  _clearance_schedule = clearance_schedule;
}

template <typename T>
void
FunctorInterface<T>::clearCacheData()
{
  for (auto & map_pr : _qp_to_value)
    for (auto & pr : map_pr.second)
      pr.first = false;

  _current_qp_elem = DofObject::invalid_id;
  _current_qp_elem_data = nullptr;
}

template <typename T>
void
FunctorInterface<T>::timestepSetup()
{
  if (_clearance_schedule.count(EXEC_TIMESTEP_BEGIN))
    clearCacheData();
}

template <typename T>
void
FunctorInterface<T>::residualSetup()
{
  if (_clearance_schedule.count(EXEC_LINEAR))
    clearCacheData();
}

template <typename T>
void
FunctorInterface<T>::jacobianSetup()
{
  if (_clearance_schedule.count(EXEC_NONLINEAR))
    clearCacheData();
}

/**
 * Class template for creating constants
 */
template <typename T>
class ConstantFunctor : public FunctorInterface<T>
{
public:
  ConstantFunctor(const T & value) : _value(value) {}
  ConstantFunctor(T && value) : _value(value) {}

private:
  using typename FunctorInterface<T>::FaceArg;
  using typename FunctorInterface<T>::ElemFromFaceArg;
  using typename FunctorInterface<T>::QpArg;
  using typename FunctorInterface<T>::FunctorType;
  using typename FunctorInterface<T>::FunctorReturnType;

  T evaluate(const libMesh::Elem * const &, unsigned int) const override final { return _value; }
  T evaluate(const ElemFromFaceArg &, unsigned int) const override final { return _value; }
  T evaluate(const FaceArg &, unsigned int) const override final { return _value; }
  T evaluate(const QpArg &, unsigned int) const override final { return _value; }
  T evaluate(const std::tuple<Moose::ElementType, unsigned int, SubdomainID> &,
             unsigned int) const override final
  {
    return _value;
  }

private:
  T _value;
};
