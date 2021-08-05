//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseMesh.h"
#include "MooseTypes.h"
#include "MooseError.h"
#include "FunctorInterface.h"
#include "Moose.h"
#include "SetupInterface.h"

#include "libmesh/elem.h"
#include "libmesh/remote_elem.h"

#include <unordered_map>
#include <functional>

/**
 * A non-templated base class for functor material properties that allow an owner object to hold
 * different class template instantiations of \p FunctorMaterialProperty in a single container
 */
class FunctorPropertyValue
{
public:
  FunctorPropertyValue() = default;
  virtual ~FunctorPropertyValue() = default;
  virtual void timestepSetup() = 0;
  virtual void residualSetup() = 0;
  virtual void jacobianSetup() = 0;
};

/**
 * A material property that is evaluated on-the-fly via calls to various overloads of \p operator()
 */
template <typename T>
class FunctorMaterialProperty : public FunctorPropertyValue, public FunctorInterface<T>

{
public:
  FunctorMaterialProperty(const std::string & name)
    : FunctorPropertyValue(), _name(name), _clearance_schedule({EXEC_ALWAYS})
  {
  }

  using typename FunctorInterface<T>::FaceArg;
  using typename FunctorInterface<T>::ElemAndFaceArg;
  using typename FunctorInterface<T>::QpArg;
  using typename FunctorInterface<T>::FunctorType;
  using typename FunctorInterface<T>::FunctorReturnType;

  using ElemFn = std::function<T(const Elem * const &)>;
  using ElemAndFaceFn = std::function<T(const ElemAndFaceArg &)>;
  using FaceFn = std::function<T(const FaceArg &)>;
  using QpFn = std::function<T(const QpArg &)>;
  using TQpFn = std::function<T(const std::tuple<Moose::ElementType, unsigned int, SubdomainID> &)>;

  /**
   * Set the functor that will be used in calls to \p operator() overloads
   * @param mesh The mesh that the functor is defined on
   * @param block_ids The block/subdomain IDs that the user-provided functor is valid for
   * @param my_lammy The functor that defines this object's \p operator() evaluations
   */
  template <typename PolymorphicLambda>
  void setFunctor(const MooseMesh & mesh,
                  const std::set<SubdomainID> & block_ids,
                  PolymorphicLambda my_lammy);

  T operator()(const Elem * const & elem) const override final;
  T operator()(const ElemAndFaceArg & elem_and_face) const override final;
  T operator()(const FaceArg & face) const override final;
  T operator()(const QpArg & qp) const override final;
  T operator()(
      const std::tuple<Moose::ElementType, unsigned int, SubdomainID> & tqp) const override final;
  void residualSetup() override final;
  void jacobianSetup() override final;
  void timestepSetup() override final;

  /**
   * Set how often to clear the material property cache
   */
  void setCacheClearanceSchedule(const std::set<ExecFlagType> & clearance_schedule);

private:
  /**
   * clear cache data
   */
  void clearCacheData();

  /// Functors that return element average values (or cell centroid values or whatever the
  /// implementer wants to return for a given element argument)
  std::unordered_map<SubdomainID, ElemFn> _elem_functor;

  /// Functors that return the value on the requested element that will perform any necessary
  /// ghosting operations if this object is not technically defined on the requested subdomain
  std::unordered_map<SubdomainID, ElemAndFaceFn> _elem_and_face_functor;

  /// Functors that return potentially limited interpolations at faces
  std::unordered_map<SubdomainID, FaceFn> _face_functor;

  /// Functors that will index elemental data at a provided quadrature point index
  std::unordered_map<SubdomainID, QpFn> _qp_functor;

  /// Functors that will index elemental, neighbor, or lower-dimensional data at a provided
  /// quadrature point index
  std::unordered_map<SubdomainID, TQpFn> _tqp_functor;

  /// The name of this object
  std::string _name;

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
template <typename PolymorphicLambda>
void
FunctorMaterialProperty<T>::setFunctor(const MooseMesh & mesh,
                                       const std::set<SubdomainID> & block_ids,
                                       PolymorphicLambda my_lammy)
{
  auto add_lammy = [this, my_lammy](const SubdomainID block_id) {
    auto pr = _elem_functor.emplace(block_id, my_lammy);
    if (!pr.second)
      mooseError("No insertion for the functor material property '",
                 _name,
                 "' for block id ",
                 block_id,
                 ". Another material must already declare this property on that block.");
    _elem_and_face_functor.emplace(block_id, my_lammy);
    _face_functor.emplace(block_id, my_lammy);
    _qp_functor.emplace(block_id, my_lammy);
    _tqp_functor.emplace(block_id, my_lammy);
  };

  for (const auto block_id : block_ids)
  {
    if (block_id == Moose::ANY_BLOCK_ID)
    {
      const auto & inner_block_ids = mesh.meshSubdomains();
      for (const auto inner_block_id : inner_block_ids)
        add_lammy(inner_block_id);
    }
    else
      add_lammy(block_id);
  }
}

template <typename T>
T
FunctorMaterialProperty<T>::operator()(const Elem * const & elem) const
{
  mooseAssert(elem && elem != libMesh::remote_elem,
              "The element must be non-null and non-remote in functor material properties");
  auto it = _elem_functor.find(elem->subdomain_id());
  mooseAssert(it != _elem_functor.end(), "The provided subdomain ID doesn't exist in the map!");
  return it->second(elem);
}

template <typename T>
T
FunctorMaterialProperty<T>::operator()(const ElemAndFaceArg & elem_and_face) const
{
  mooseAssert((std::get<0>(elem_and_face) && std::get<0>(elem_and_face) != libMesh::remote_elem) ||
                  std::get<1>(elem_and_face),
              "The element must be non-null and non-remote or the face must be non-null in functor "
              "material properties");
  auto it = _elem_and_face_functor.find(std::get<2>(elem_and_face));
  mooseAssert(it != _elem_and_face_functor.end(),
              "The provided subdomain ID doesn't exist in the map!");
  return it->second(elem_and_face);
}

template <typename T>
T
FunctorMaterialProperty<T>::operator()(const FaceArg & face) const
{
  mooseAssert(std::get<0>(face), "FaceInfo must be non-null");
  auto it = _face_functor.find(std::get<3>(face));
  mooseAssert(it != _face_functor.end(), "The provided subdomain ID doesn't exist in the map!");
  return it->second(face);
}

template <typename T>
T
FunctorMaterialProperty<T>::operator()(const QpArg & elem_and_qp) const
{
  auto evaluate = [this, &elem_and_qp]() -> T {
    auto it = _qp_functor.find(elem_and_qp.first->subdomain_id());
    mooseAssert(it != _qp_functor.end(),
                "The provided element has a subdomain ID that doesn't exist in the map!");
    return it->second(elem_and_qp);
  };

  if (_clearance_schedule.count(EXEC_ALWAYS))
    return evaluate();

  const auto elem_id = elem_and_qp.first->id();
  if (elem_id != _current_qp_elem)
  {
    _current_qp_elem = elem_id;
    _current_qp_elem_data = &_qp_to_value[elem_id];
  }
  auto & qp_elem_data_ref = *_current_qp_elem_data;
  const auto qp = elem_and_qp.second;

  // Check and see whether we even have sized for this quadrature point. If we haven't then we must
  // evaluate
  if (qp >= qp_elem_data_ref.size())
  {
    mooseAssert(qp == qp_elem_data_ref.size(),
                "I believe that we always iterate over quadrature points in a contiguous fashion");
    qp_elem_data_ref.resize(qp + 1);
    auto & pr = qp_elem_data_ref.back();
    pr.second = evaluate();
    pr.first = true;
    return pr.second;
  }

  // We've already sized for this qp, so let's see whether we have a valid cache value
  auto & pr = qp_elem_data_ref[qp];
  if (pr.first)
    return pr.second;

  // No valid cache value so evaluate
  pr.second = evaluate();
  pr.first = true;
  return pr.second;
}

template <typename T>
T
FunctorMaterialProperty<T>::operator()(
    const std::tuple<Moose::ElementType, unsigned int, SubdomainID> & tqp) const
{
  auto it = _tqp_functor.find(std::get<2>(tqp));
  mooseAssert(it != _tqp_functor.end(), "The provided subdomain ID doesn't exist in the map!");
  return it->second(tqp);
}

template <typename T>
void
FunctorMaterialProperty<T>::setCacheClearanceSchedule(
    const std::set<ExecFlagType> & clearance_schedule)
{
  _clearance_schedule = clearance_schedule;
}

template <typename T>
void
FunctorMaterialProperty<T>::clearCacheData()
{
  for (auto & map_pr : _qp_to_value)
    for (auto & pr : map_pr.second)
      pr.first = false;

  _current_qp_elem = DofObject::invalid_id;
  _current_qp_elem_data = nullptr;
}

template <typename T>
void
FunctorMaterialProperty<T>::timestepSetup()
{
  if (_clearance_schedule.count(EXEC_TIMESTEP_BEGIN))
    clearCacheData();
}

template <typename T>
void
FunctorMaterialProperty<T>::residualSetup()
{
  if (_clearance_schedule.count(EXEC_LINEAR))
    clearCacheData();
}

template <typename T>
void
FunctorMaterialProperty<T>::jacobianSetup()
{
  if (_clearance_schedule.count(EXEC_NONLINEAR))
    clearCacheData();
}
