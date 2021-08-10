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
 * Base class template for functor objects. This class template defines various \p operator()
 * overloads that allow a user to evaluate the functor at arbitrary geometric locations. This
 * template is meant to enable highly flexible on-the-fly variable and material property evaluations
 */
template <typename T>
class FunctorInterface
{
public:
  using FaceArg = std::tuple<const FaceInfo *, const Moose::FV::Limiter *, bool>;
  using ElemAndFaceArg = std::tuple<const libMesh::Elem *, const FaceInfo *, SubdomainID>;
  using FunctorType = FunctorInterface<T>;
  using FunctorReturnType = T;
  virtual ~FunctorInterface() = default;

  /**
   * Evaluate the functor with a given element. A possible implementation of this method could
   * compute an element-average
   */
  virtual T operator()(const libMesh::Elem * const & elem) const = 0;

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
  virtual T operator()(const ElemAndFaceArg & elem_and_face) const = 0;

  /**
   * @param face A tuple packing a \p FaceInfo, a \p Limiter, and a boolean denoting whether the
   * element side of the face is in the upwind direction
   * @return A limited interpolation to the provided face of the variable or material property. If
   * the material property is a composite of variables, it's important to note that each variable
   * will be interpolated individually. If aggregate interpolation is desired, then the user should
   * make two calls to the \p elem_and_face overload (one for element and one for neighbor) and
   * then use the global interpolate functions in the \p Moose::FV namespace
   */
  virtual T
  operator()(const std::tuple<const FaceInfo *, const Moose::FV::Limiter *, bool> & face) const = 0;

  /**
   * Evaluate the functor at the current qp point. Unlike the above overloads, there is a caveat to
   * calling this overload. Any variables involved in the functor evaluation must have their
   * elemental data properly pre-initialized at the desired \p qp
   */
  virtual T operator()(const unsigned int & qp) const = 0;

  /**
   * @param tqp A pair with the first member corresponding to an \p ElementType, either Element,
   * Neighbor, or Lower corresponding to the three different possible \p MooseVariableData
   * instances. The second member corresponds to the desired quadrature point
   * @return The requested element type data indexed at the requested quadrature point. As for the
   * \p qp overload, there is a caveat: any variables involved in the functor evaluation must have
   * their requested element data type properly pre-initialized at the desired quadrature point
   */
  virtual T operator()(const std::pair<Moose::ElementType, unsigned int> & tqp) const = 0;
};

/**
 * This derivative of the \p FunctorInterface template allows a user to provide a function object
 * that will be used whenever \p operator() overloads are called
 */
template <typename T>
class GenericFunctor : public FunctorInterface<T>
{
public:
  GenericFunctor(const std::string & name) : _name(name) {}

  using typename FunctorInterface<T>::FaceArg;
  using typename FunctorInterface<T>::ElemAndFaceArg;
  using typename FunctorInterface<T>::FunctorType;
  using typename FunctorInterface<T>::FunctorReturnType;

  using ElemFn = std::function<T(const Elem * const &)>;
  using ElemAndFaceFn = std::function<T(const ElemAndFaceArg &)>;
  using FaceFn = std::function<T(const FaceArg &)>;
  using QpFn = std::function<T(const unsigned int &)>;
  using TQpFn = std::function<T(const std::pair<Moose::ElementType, unsigned int> &)>;

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
  T operator()(const unsigned int & qp) const override final;
  T operator()(const std::pair<Moose::ElementType, unsigned int> & tqp) const override final;

private:
  /// Functors that return element average values (or cell centroid values or whatever the
  /// implementer wants to return for a given element argument)
  std::unordered_map<SubdomainID, ElemFn> _elem_functor;

  /// Functors that return the value on the requested element that will perform any necessary
  /// ghosting operations if this object is not technically defined on the requested subdomain
  std::unordered_map<SubdomainID, ElemAndFaceFn> _elem_and_face_functor;

  /// Functors that return potentially limited interpolations at faces
  FaceFn _face_functor;

  /// Functors that will index elemental data at a provided quadrature point index
  QpFn _qp_functor;

  /// Functors that will index elemental, neighbor, or lower-dimensional data at a provided
  /// quadrature point index
  TQpFn _tqp_functor;

  /// The name of this object
  std::string _name;
};

template <typename T>
template <typename PolymorphicLambda>
void
GenericFunctor<T>::setFunctor(const MooseMesh & mesh,
                              const std::set<SubdomainID> & block_ids,
                              PolymorphicLambda my_lammy)
{
  auto add_lammy = [this, my_lammy](const SubdomainID block_id) {
    _elem_functor.emplace(block_id, my_lammy);
    _elem_and_face_functor.emplace(block_id, my_lammy);
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

  _face_functor = my_lammy;
  _qp_functor = my_lammy;
  _tqp_functor = my_lammy;
}

template <typename T>
T
GenericFunctor<T>::operator()(const Elem * const & elem) const
{
  mooseAssert(elem && elem != libMesh::remote_elem,
              "The element must be non-null and non-remote in functor material properties");
  auto it = _elem_functor.find(elem->subdomain_id());
  mooseAssert(it != _elem_functor.end(), "The provided subdomain ID doesn't exist in the map!");
  return it->second(elem);
}

template <typename T>
T
GenericFunctor<T>::operator()(const ElemAndFaceArg & elem_and_face) const
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
GenericFunctor<T>::operator()(const GenericFunctor<T>::FaceArg & face) const
{
  mooseAssert(std::get<0>(face), "FaceInfo must be non-null");
  return _face_functor(face);
}

template <typename T>
T
GenericFunctor<T>::operator()(const unsigned int & qp) const
{
  return _qp_functor(qp);
}

template <typename T>
T
GenericFunctor<T>::operator()(const std::pair<Moose::ElementType, unsigned int> & tqp) const
{
  return _tqp_functor(tqp);
}
