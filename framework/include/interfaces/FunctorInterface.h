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
  using FaceArg = std::tuple<const FaceInfo *, const Moose::FV::Limiter *, bool, SubdomainID>;
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
  virtual T operator()(const FaceArg & face) const = 0;

  /**
   * Evaluate the functor at the current qp point. Unlike the above overloads, there is a caveat to
   * calling this overload. Any variables involved in the functor evaluation must have their
   * elemental data properly pre-initialized at the desired \p qp
   */
  virtual T operator()(const std::pair<unsigned int, SubdomainID> & qp) const = 0;

  /**
   * @param tqp A pair with the first member corresponding to an \p ElementType, either Element,
   * Neighbor, or Lower corresponding to the three different possible \p MooseVariableData
   * instances. The second member corresponds to the desired quadrature point
   * @return The requested element type data indexed at the requested quadrature point. As for the
   * \p qp overload, there is a caveat: any variables involved in the functor evaluation must have
   * their requested element data type properly pre-initialized at the desired quadrature point
   */
  virtual T
  operator()(const std::tuple<Moose::ElementType, unsigned int, SubdomainID> & tqp) const = 0;
};

/**
 * Class template for creating constants
 */
template <typename T>
class ConstantFunctor : public FunctorInterface<T>
{
public:
  using typename FunctorInterface<T>::FaceArg;
  using typename FunctorInterface<T>::ElemAndFaceArg;
  using typename FunctorInterface<T>::FunctorType;
  using typename FunctorInterface<T>::FunctorReturnType;

  ConstantFunctor(const T & value) : _value(value) {}
  ConstantFunctor(T && value) : _value(value) {}

  virtual T operator()(const libMesh::Elem * const &) const override final { return _value; }

  T operator()(const ElemAndFaceArg &) const override final { return _value; }

  T operator()(const FaceArg &) const override final { return _value; }

  T operator()(const std::pair<unsigned int, SubdomainID> &) const override final { return _value; }

  T
  operator()(const std::tuple<Moose::ElementType, unsigned int, SubdomainID> &) const override final
  {
    return _value;
  }

private:
  T _value;
};
