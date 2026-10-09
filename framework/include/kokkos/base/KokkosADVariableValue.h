//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosVariableValue.h"
#include "KokkosDenseADReal.h"

namespace Moose::Kokkos
{

/**
 * A block of consecutive derivative slots of a pointwise AD residual object, holding the
 * derivatives with respect to one quadrature point quantity of one nonlinear variable
 */
struct ADSlotBlock
{
  /// System number of the variable
  unsigned int sys;
  /// Variable number of the variable in its system
  unsigned int var;
  /// First slot of the block
  unsigned int first;
  /// Whether the block holds the gradient, of AD_GRADIENT_SLOTS slots, or the value, of one slot
  bool gradient;
};

/**
 * Slot table of a pointwise AD residual object. The AD accessors of the object register the
 * quadrature point quantities they read, each nonlinear variable of the object's system receiving
 * a slot block per quantity. The derivatives of the residual with respect to the slots are
 * contracted with the shape functions of the column variable of each block.
 */
class ADSlotTable
{
public:
  /**
   * Register a quadrature point quantity of a variable
   * @param sys The system number of the variable
   * @param var The variable number in its system
   * @param gradient Whether the quantity is the gradient
   * @returns The first slot of the quantity
   */
  unsigned int add(unsigned int sys, unsigned int var, bool gradient);
  /**
   * Copy the registered slot blocks to the device
   */
  void finalize();
  /**
   * Get the number of slots used by the registered quantities
   */
  unsigned int numSlots() const { return _num_slots; }
  /**
   * Get the number of slot blocks
   */
  KOKKOS_FUNCTION unsigned int numBlocks() const { return _num_blocks; }
  /**
   * Get a slot block
   */
  KOKKOS_FUNCTION const ADSlotBlock & block(unsigned int b) const { return _blocks[b]; }
  /**
   * Get the slot blocks on host
   */
  const std::vector<ADSlotBlock> & hostBlocks() const { return _host_blocks; }

private:
  /// Registered slot blocks
  std::vector<ADSlotBlock> _host_blocks;
  /// Slot blocks on device
  Array<ADSlotBlock> _blocks;
  /// Number of slot blocks, available on device
  unsigned int _num_blocks = 0;
  /// Number of slots
  unsigned int _num_slots = 0;
};

/**
 * Slot assignment of an AD accessor, written by its residual object during initial setup. An
 * accessor whose variable is not a nonlinear variable coupled to the residual variable keeps no
 * slot, so its derivatives are zero.
 */
struct ADSlotAssignment
{
  /// First slot of the quantity, or libMesh::invalid_uint when the quantity carries no derivatives
  unsigned int first = libMesh::invalid_uint;
  /// Derivative of the quantity with respect to its first slot
  Real seed = 1;
  /// Host source of the seed of a time derivative, the derivative of the time derivative with
  /// respect to the solution, which the time integrator updates between dispatches
  const Real * seed_source = nullptr;
  /// Whether the residual object has assigned the slot
  bool finalized = false;

  /**
   * Refresh the seed from its source, before the slot assignment is copied to the device
   */
  void refresh()
  {
    if (seed_source)
      seed = *seed_source;
  }
};

/**
 * Accessor of the quadrature point value of a variable as a dense dual number. A time derivative
 * tag seeds the value slot with the derivative of the time derivative with respect to the
 * solution.
 * @tparam slots The number of derivative slots of the residual object
 */
template <unsigned int slots>
class ADVariableValue
{
public:
  ADVariableValue() = default;
  /**
   * Copy constructor for parallel dispatch, which refreshes the seed of a time derivative
   */
  ADVariableValue(const ADVariableValue & object) : _value(object._value), _slot(object._slot)
  {
    _slot.refresh();
  }
  ADVariableValue & operator=(const ADVariableValue & object) = default;
  /**
   * Constructor
   * @param value The plain value accessor of the variable
   */
  ADVariableValue(const VariableValue & value) : _value(value) {}

  KOKKOS_FUNCTION operator bool() const { return static_cast<bool>(_value); }

  /**
   * Get the value at a quadrature point
   * @param datum The AssemblyDatum object of the current thread
   * @param qp The local quadrature point index
   * @returns The value, with its derivative with respect to the slots of the residual object
   */
  KOKKOS_FUNCTION DenseADReal<slots> operator()(AssemblyDatum & datum, unsigned int qp) const
  {
    KOKKOS_ASSERT(_slot.finalized);

    return seededDenseADReal<slots>(_value(datum, qp), _slot.first, _slot.seed);
  }

  KOKKOS_FUNCTION const Variable & variable() const { return _value.variable(); }

  /**
   * Get the slot assignment, written by the residual object during initial setup
   */
  ADSlotAssignment & slot() { return _slot; }

private:
  /// Plain value accessor
  VariableValue _value;
  /// Slot assignment of the value
  ADSlotAssignment _slot;
};

/**
 * Accessor of the quadrature point gradient of a variable as a vector of dense dual numbers, whose
 * component k is seeded in slot k of the slot block of the gradient
 * @tparam slots The number of derivative slots of the residual object
 */
template <unsigned int slots>
class ADVariableGradient
{
public:
  ADVariableGradient() = default;
  /**
   * Copy constructor for parallel dispatch, which refreshes the seed of a time derivative
   */
  ADVariableGradient(const ADVariableGradient & object)
    : _gradient(object._gradient), _slot(object._slot)
  {
    _slot.refresh();
  }
  ADVariableGradient & operator=(const ADVariableGradient & object) = default;
  /**
   * Constructor
   * @param gradient The plain gradient accessor of the variable
   */
  ADVariableGradient(const VariableGradient & gradient) : _gradient(gradient) {}

  KOKKOS_FUNCTION operator bool() const { return static_cast<bool>(_gradient); }

  /**
   * Get the gradient at a quadrature point
   * @param datum The AssemblyDatum object of the current thread
   * @param qp The local quadrature point index
   * @returns The gradient, with its derivatives with respect to the slots of the residual object
   */
  KOKKOS_FUNCTION DenseADReal3<slots> operator()(AssemblyDatum & datum, unsigned int qp) const
  {
    KOKKOS_ASSERT(_slot.finalized);

    const auto grad = _gradient(datum, qp);
    const auto first = _slot.first;

    DenseADReal3<slots> result;
    for (unsigned int k = 0; k < AD_GRADIENT_SLOTS; ++k)
      result(k) = seededDenseADReal<slots>(
          grad(k), first == libMesh::invalid_uint ? libMesh::invalid_uint : first + k, _slot.seed);

    return result;
  }

  KOKKOS_FUNCTION const Variable & variable() const { return _gradient.variable(); }

  /**
   * Get the slot assignment, written by the residual object during initial setup
   */
  ADSlotAssignment & slot() { return _slot; }

private:
  /// Plain gradient accessor
  VariableGradient _gradient;
  /// Slot assignment of the gradient
  ADSlotAssignment _slot;
};

} // namespace Moose::Kokkos
