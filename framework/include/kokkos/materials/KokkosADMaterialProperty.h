//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosMaterialProperty.h"
#include "KokkosDenseADReal.h"
#include "KokkosADVariableValue.h"

#include <optional>
#include <sstream>

namespace Moose::Kokkos
{

/**
 * Get the name of the property holding the derivatives of a pointwise AD property
 * @param prop_name The name of the AD property, which holds its value
 */
inline std::string
adDerivativesPropertyName(const std::string & prop_name)
{
  return prop_name + "_ad_derivatives";
}

/**
 * Parse a material property name that gives a constant value
 * @param prop_name The property name
 * @returns The constant value, or nothing if the name does not parse cleanly as a number
 */
inline std::optional<Real>
adConstantPropertyValue(const std::string & prop_name)
{
  std::istringstream ss(prop_name);
  Real value;

  if (ss >> value && ss.eof())
    return value;

  return {};
}

class ADMaterialPropertyReader;

/**
 * An input of a pointwise AD material property: an AD accessor of the declaring material, whose
 * quantity is one of those the property has derivatives with respect to, or an AD material property
 * read by the declaring material, all of whose inputs the property depends on
 */
class ADPropertyInput
{
public:
  /**
   * Constructor
   * @param value The AD accessor of a variable value
   */
  ADPropertyInput(const ADVariableValue & value)
    : _slot(&const_cast<ADVariableValue &>(value).slot()), _gradient(false)
  {
  }
  /**
   * Constructor
   * @param gradient The AD accessor of a variable gradient
   */
  ADPropertyInput(const ADVariableGradient & gradient)
    : _slot(&const_cast<ADVariableGradient &>(gradient).slot()), _gradient(true)
  {
  }
  /**
   * Constructor
   * @param property The reader of an AD material property
   */
  ADPropertyInput(const ADMaterialPropertyReader & property) : _property(&property) {}

  /**
   * Get the slot assignment of the accessor, or nullptr for a property
   */
  const ADSlotAssignment * slot() const { return _slot; }
  /**
   * Get whether the accessor reads a gradient
   */
  bool gradient() const { return _gradient; }
  /**
   * Get the reader of the property, or nullptr for an accessor
   */
  const ADMaterialPropertyReader * property() const { return _property; }

private:
  /// Slot assignment of the accessor
  const ADSlotAssignment * _slot = nullptr;
  /// Whether the accessor reads a gradient
  bool _gradient = false;
  /// Reader of the property
  const ADMaterialPropertyReader * _property = nullptr;
};

/**
 * Writeable access to a pointwise AD property at a quadrature point, which stores the value and
 * the derivatives of a dual number of the declaring material
 * @tparam slots The number of derivative slots of the declaring material
 */
template <unsigned int slots>
class ADMaterialPropertyValue
{
public:
  /**
   * Constructor
   * @param value The property holding the value
   * @param derivatives The property holding the derivatives
   * @param map The storage index of the derivative with respect to each slot of the material
   * @param datum The Datum object of the current thread
   * @param qp The local quadrature point index
   */
  KOKKOS_FUNCTION ADMaterialPropertyValue(const MaterialProperty<Real> & value,
                                          const MaterialProperty<Real, 1> & derivatives,
                                          const Array<unsigned int> & map,
                                          const Datum & datum,
                                          const unsigned int qp)
    : _value(value, datum, qp), _derivatives(derivatives, datum, qp), _map(map)
  {
  }

  /**
   * Store a dual number
   * @param dual The value with its derivatives with respect to the slots of the material
   */
  KOKKOS_FUNCTION void operator=(const DenseADReal<slots> & dual)
  {
    KOKKOS_ASSERT(_map.size() >= slots);

    _value = denseADValue(dual);

    for (unsigned int s = 0; s < slots; ++s)
    {
      const auto d = denseADDerivative(dual, s);

      if (_map[s] != libMesh::invalid_uint)
        _derivatives(_map[s]) = d;
#ifndef NDEBUG
      else if (d != 0)
        ::Kokkos::abort("A Kokkos AD material property has a derivative with respect to a "
                        "quadrature point quantity missing from the input list of its "
                        "declaration. Add the accessor of the quantity to the input list.");
#endif
    }
  }

private:
  /// Value of the property
  MaterialPropertyValue<Real, 0> _value;
  /// Derivatives of the property
  MaterialPropertyValue<Real, 1> _derivatives;
  /// Storage index of the derivative with respect to each slot of the material, or
  /// libMesh::invalid_uint for a slot outside the input list
  const Array<unsigned int> & _map;
};

/**
 * A pointwise AD material property, declared by a Kokkos AD material. The value is an ordinary
 * property of the declared name, which any consumer reads; the derivatives are a companion property
 * named by adDerivativesPropertyName(), with one entry per slot of the declaring material, or per
 * slot of the inputs of the property when its declaration lists them, followed on the current
 * configuration by the displacement slots. The record of the companion lists the quadrature point
 * quantity of each block of stored derivatives in each subdomain.
 * @tparam T The property data type
 */
template <typename T>
class ADMaterialProperty
{
  static_assert(std::is_same_v<T, Real>, "Kokkos AD material properties are of type Real.");

public:
  ADMaterialProperty() = default;
  /**
   * Constructor
   * @param value The property holding the value
   * @param derivatives The property holding the derivatives
   */
  ADMaterialProperty(const MaterialProperty<Real> & value,
                     const MaterialProperty<Real, 1> & derivatives,
                     const Array<unsigned int> & map)
    : _value(value), _derivatives(derivatives), _map(map)
  {
  }

  /**
   * Get the property holding the derivatives
   */
  const MaterialProperty<Real, 1> & derivatives() const { return _derivatives; }

#ifdef MOOSE_KOKKOS_SCOPE
  /**
   * Get writeable access to the property at a quadrature point
   * @param datum The AD datum object of the current thread
   * @param qp The local quadrature point index
   */
  template <typename DatumType, unsigned int slots = DatumType::ad_slots>
  KOKKOS_FUNCTION ADMaterialPropertyValue<slots> operator()(const DatumType & datum,
                                                            const unsigned int qp) const
  {
    return ADMaterialPropertyValue<slots>(_value, _derivatives, _map, datum, qp);
  }
#endif

private:
  /// Property holding the value
  MaterialProperty<Real> _value;
  /// Property holding the derivatives
  MaterialProperty<Real, 1> _derivatives;
  /// Storage index of the derivative with respect to each slot of the material, shared with the
  /// declaring material, which fills it during initial setup
  Array<unsigned int> _map;
};

/**
 * Read access to a pointwise AD material property by a pointwise AD object, which assembles the
 * stored value and derivatives into a dual number of the slots of the reader through the slot map
 * of the subdomain
 */
class ADMaterialPropertyReader
{
public:
  ADMaterialPropertyReader() = default;
  /**
   * Constructor
   * @param value The property holding the value
   * @param derivatives The property holding the derivatives
   */
  ADMaterialPropertyReader(const MaterialProperty<Real> & value,
                           const MaterialProperty<Real, 1> & derivatives)
    : _value(value), _derivatives(derivatives)
  {
  }
  /**
   * Constructor for a constant property, which carries no derivatives
   * @param value The constant value
   */
  ADMaterialPropertyReader(const Real value) : _value(value) { _map.finalized = true; }

  /**
   * Get the property holding the derivatives
   */
  const MaterialProperty<Real, 1> & derivatives() const { return _derivatives; }
  /**
   * Get the slot map, written by the reader during initial setup
   */
  ADPropertySlotMap & slotMap() { return _map; }
  /**
   * Get the slot map
   */
  const ADPropertySlotMap & slotMap() const { return _map; }

#ifdef MOOSE_KOKKOS_SCOPE
  /**
   * Get the property at a quadrature point
   * @param datum The AD datum object of the current thread
   * @param qp The local quadrature point index
   * @returns The property, with its derivatives with respect to the slots of the reader
   */
  template <typename Base, unsigned int slots>
  KOKKOS_FUNCTION DenseADReal<slots> operator()(ADDatumBase<Base, slots> & datum,
                                                const unsigned int qp) const
  {
    KOKKOS_ASSERT(_map.finalized);

    DenseADReal<slots> result = Real(_value(datum, qp));

    if constexpr (slots > 0)
    {
      result.derivatives() = 0;

      if (!_derivatives)
        return result;

      const auto & map = _map.slots[datum.subdomain()];
      const auto derivatives = _derivatives(datum, qp);

      for (unsigned int s = 0; s < map.size(); ++s)
        if (map[s] != libMesh::invalid_uint)
          result.derivatives()[map[s]] += derivatives(s);
    }

    return result;
  }
#endif

private:
  /// Property holding the value
  MaterialProperty<Real> _value;
  /// Property holding the derivatives
  MaterialProperty<Real, 1> _derivatives;
  /// Slot of the reader for each stored derivative in each subdomain
  ADPropertySlotMap _map;
};

} // namespace Moose::Kokkos
