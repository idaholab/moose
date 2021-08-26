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
#include "FunctorInterface.h"

#include "libmesh/elem.h"
#include "libmesh/remote_elem.h"

#include <unordered_map>
#include <functional>

class FaceInfo;
namespace Moose
{
namespace FV
{
class Limiter;
}
}

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
  using FaceArg = std::tuple<const FaceInfo *, const Moose::FV::Limiter *, bool, SubdomainID>;
  using ElemAndFaceArg = std::tuple<const libMesh::Elem *, const FaceInfo *, SubdomainID>;
  using QpArg = std::tuple<const libMesh::Elem *, unsigned int, const QBase *>;
  using FunctorType = FunctorInterface<T>;
  using FunctorReturnType = T;
  virtual ~FunctorInterface() = default;
  FunctorInterface() : _clearance_schedule({EXEC_ALWAYS}) {}

  ///@{
  /**
   * Same as their \p evaluate overloads with the same argument but allows for caching
   * implementation. These are the methods a user will call
   */
  T operator()(const libMesh::Elem * const & elem, unsigned int state = 0) const;
  T operator()(const ElemAndFaceArg & elem_and_face, unsigned int state = 0) const;
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
   * Evaluate the functor with a given element. A possible implementation of this method could
   * compute an element-average
   */
  virtual T evaluate(const libMesh::Elem * const & elem, unsigned int state) const = 0;

  /**
   * @param elem_and_face This is a tuple packing an element, \p FaceInfo, and subdomain ID
   * @return For a variable: the value associated with the element argument of the tuple if the
   * variable exists on the element. If it does not, a ghost value is computed given that the
   * element across the face from the provided element supports the variable. For a material
   * property: the subdomain argument is critical. One can imagine having two different computations
   * of diffusivity on different subdomains, let's call them D_E and D_N (denoting element and
   * neighbor in MOOSE lingo). A flux kernel on a subdomain boundary may very well want a
   * D_E evaluation on the N side of the face. By providing the subdomain ID corresponding to E, the
   * flux kernel ensures that it will retrieve a D_E evaluation, even on the N side of the face.
   */
  virtual T evaluate(const ElemAndFaceArg & elem_and_face, unsigned int state) const = 0;

  /**
   * @param face A tuple packing a \p FaceInfo, a \p Limiter, and a boolean denoting whether the
   * element side of the face is in the upwind direction
   * @return A limited interpolation to the provided face of the variable or material property. If
   * the material property is a composite of variables, it's important to note that each variable
   * will be interpolated individually. If aggregate interpolation is desired, then the user should
   * make two calls to the \p elem_and_face overload (one for element and one for neighbor) and
   * then use the global interpolate functions in the \p Moose::FV namespace
   */
  virtual T evaluate(const FaceArg & face, unsigned int state) const = 0;

  /**
   * Evaluate the functor at the current qp point. Unlike the above overloads, there is a caveat to
   * calling this overload. Any variables involved in the functor evaluation must have their
   * elemental data properly pre-initialized at the desired quadrature point
   */
  virtual T evaluate(const QpArg & qp, unsigned int state) const = 0;

  /**
   * @param tqp A pair with the first member corresponding to an \p ElementType, either Element,
   * Neighbor, or Lower corresponding to the three different possible \p MooseVariableData
   * instances. The second member corresponds to the desired quadrature point
   * @return The requested element type data indexed at the requested quadrature point. As for the
   * \p qp overload, there is a caveat: any variables involved in the functor evaluation must have
   * their requested element data type properly pre-initialized at the desired quadrature point
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
FunctorInterface<T>::operator()(const ElemAndFaceArg & elem_and_face,
                                const unsigned int state) const
{
  return evaluate(elem_and_face, state);
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
  using typename FunctorInterface<T>::ElemAndFaceArg;
  using typename FunctorInterface<T>::QpArg;
  using typename FunctorInterface<T>::FunctorType;
  using typename FunctorInterface<T>::FunctorReturnType;

  T evaluate(const libMesh::Elem * const &, unsigned int) const override final { return _value; }
  T evaluate(const ElemAndFaceArg &, unsigned int) const override final { return _value; }
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
