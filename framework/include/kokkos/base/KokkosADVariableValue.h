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
#include "KokkosADGeometry.h"
#include "KokkosADSlotBlock.h"

namespace Moose::Kokkos
{

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

  /**
   * Leave unused slots so that the next registered quantity starts at a given slot
   * @param first The slot at which the next registered quantity starts
   */
  void reserve(const unsigned int first) { _num_slots = std::max(_num_slots, first); }
  /**
   * Register the displacements of the current configuration, whose values and reference gradients
   * occupy the AD_DISPLACEMENT_SLOTS slots following those of the accessors
   * @param displacements The displacement variable
   * @param seeded Whether each component carries derivatives, which holds for the nonlinear
   * displacements of the residual system that the coupling matrix couples to the residual variable
   */
  void addDisplacements(const Variable & displacements, const std::vector<bool> & seeded);
  /**
   * Get the first slot of the value of a displacement component, or libMesh::invalid_uint if the
   * component carries no derivatives
   */
  KOKKOS_FUNCTION unsigned int displacementValueSlot(const unsigned int c) const
  {
    return _displacement_first[c];
  }
  /**
   * Get the first slot of the reference gradient of a displacement component, or
   * libMesh::invalid_uint if the component carries no derivatives
   */
  KOKKOS_FUNCTION unsigned int displacementGradientSlot(const unsigned int c) const
  {
    return _displacement_first[c] == libMesh::invalid_uint
               ? libMesh::invalid_uint
               : _displacement_first[c] + AD_VALUE_SLOTS;
  }

private:
  /// Registered slot blocks
  std::vector<ADSlotBlock> _host_blocks;
  /// Slot blocks on device
  Array<ADSlotBlock> _blocks;
  /// Number of slot blocks, available on device
  unsigned int _num_blocks = 0;
  /// Number of slots
  unsigned int _num_slots = 0;
  /// First slot of each displacement component, or libMesh::invalid_uint
  unsigned int _displacement_first[3] = {
      libMesh::invalid_uint, libMesh::invalid_uint, libMesh::invalid_uint};
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
 * The datum of a pointwise AD object, which fixes the number of derivative slots of its dual
 * numbers. On the current configuration it also carries the geometry as dual numbers whose
 * derivatives with respect to the displacement slots give the variation of the geometry, so that
 * every quantity computed from it carries its displacement derivatives.
 * @tparam Base The plain datum: AssemblyDatum for residual objects, which assemble against test
 * and trial functions, and Datum for materials
 * @tparam slots The number of derivative slots
 */
template <typename Base, unsigned int slots>
class ADDatumBase : public Base
{
public:
  /// Number of derivative slots
  static constexpr unsigned int ad_slots = slots;

  /**
   * Constructor
   * @param datum The plain datum of the element, side, or node
   * @param table The slot table of the object
   */
  KOKKOS_FUNCTION ADDatumBase(const Base & datum, const ADSlotTable & table)
    : Base(datum), _table(table)
  {
    // On the current configuration the position carries displacement derivatives only as a dual
    // number
    if constexpr (slots >= AD_DISPLACEMENT_SLOTS)
      this->adDisplaced(this->_configuration.displaced());
  }

  /**
   * Get the slot table of the object
   */
  KOKKOS_FUNCTION const ADSlotTable & table() const { return _table; }

  /**
   * Get the geometry of a quadrature point as dual numbers
   * @param qp The local quadrature point index
   * @returns The geometry
   */
  KOKKOS_FUNCTION const ADGeometry<slots> & adGeometry(const unsigned int qp);

  /**
   * Get the transformed Jacobian weight as a dual number
   */
  KOKKOS_FUNCTION DenseADReal<slots> adJxW(const unsigned int qp)
  {
    if constexpr (slots >= AD_DISPLACEMENT_SLOTS)
      if (this->_configuration.displaced())
        return adGeometry(qp).JxW;

    return this->JxW(qp);
  }

  /**
   * Get the quadrature point coordinates on the configuration as dual numbers, whose derivatives on
   * the current configuration are those of x = X + u with respect to the displacement values
   */
  KOKKOS_FUNCTION DenseADReal3<slots> adQPoint(const unsigned int qp)
  {
    if constexpr (slots >= AD_DISPLACEMENT_SLOTS)
      if (this->_configuration.displaced())
        return adGeometry(qp).q_point;

    const Real3 x = this->q_point(qp);
    return DenseADReal3<slots>(x(0), x(1), x(2));
  }

  /**
   * Push a gradient with respect to the reference position forward to the configuration, with the
   * derivatives of the deformation gradient
   * @param grad_X The gradient with respect to the reference position
   * @param qp The local quadrature point index
   */
  KOKKOS_FUNCTION DenseADReal3<slots> adPushForward(const DenseADReal3<slots> & grad_X,
                                                    const unsigned int qp)
  {
    if constexpr (slots >= AD_DISPLACEMENT_SLOTS)
      if (this->_configuration.displaced())
        return adGeometry(qp).F_inv.transpose() * grad_X;

    return this->F_inv(qp).transpose() * grad_X;
  }

  /**
   * Pull a gradient coefficient back to the reference element, so that its product with the
   * reference test function gradient equals its product with the gradient on the configuration,
   * with the derivatives of the map on the configuration
   * @param grad The gradient coefficient
   * @param qp The local quadrature point index
   */
  KOKKOS_FUNCTION DenseADReal3<slots> adPullBack(const DenseADReal3<slots> & grad,
                                                 const unsigned int qp)
  {
    if constexpr (slots >= AD_DISPLACEMENT_SLOTS)
      if (this->_configuration.displaced())
        return adGeometry(qp).dxi_dx.transpose() * grad;

    return this->dxi_dx(qp).transpose() * grad;
  }

private:
  /// Slot table of the object
  const ADSlotTable & _table;
  /// Quadrature point of the cached dual geometry
  unsigned int _ad_cached_qp = libMesh::invalid_uint;
  /// Cached dual geometry
  ADGeometry<slots> _ad_geometry;
};

template <typename Base, unsigned int slots>
KOKKOS_FUNCTION const ADGeometry<slots> &
ADDatumBase<Base, slots>::adGeometry(const unsigned int qp)
{
  KOKKOS_ASSERT(!this->isNodal());

  if (_ad_cached_qp == qp)
    return _ad_geometry;

  const auto & disp = this->_configuration.displacements();
  const auto & X = this->reference_q_point(qp);
  const auto & ref_dxi_dx = this->reference_dxi_dx(qp);

  // Deformation gradient F = I + grad_X u and position x = X + u, seeded in the displacement
  // slots
  DenseADReal33<slots> F;
  DenseADReal3<slots> x;

  for (unsigned int d = 0; d < 3; ++d)
  {
    F(d, d) = 1;
    x(d) = X(d);
  }

  for (unsigned int c = 0; c < disp.components(); ++c)
  {
    const auto & sys = this->system(disp.sys(c));
    const auto var = disp.var(c);
    const auto tag = disp.tag();
    const auto value_slot = _table.displacementValueSlot(c);
    const auto gradient_slot = _table.displacementGradientSlot(c);

    const auto & elem = this->_elem;
    const auto side = this->_side;
    const auto qp_offset = this->_qp_offset;
    const Real u = !this->isSide() ? sys.getVectorQpValue(elem, qp_offset + qp, var, tag)
                                   : sys.getVectorQpValueFace(elem, side, qp, var, tag);
    const Real3 grad_u = !this->isSide()
                             ? sys.getVectorQpGrad(elem, qp_offset + qp, var, tag)
                             : sys.getVectorQpGradFace(elem, side, ref_dxi_dx, qp, var, tag);

    x(c) = X(c) + seededDenseADReal<slots>(u, value_slot, 1);

    for (unsigned int d = 0; d < 3; ++d)
      F(c, d) += seededDenseADReal<slots>(
          grad_u(d),
          gradient_slot == libMesh::invalid_uint ? libMesh::invalid_uint : gradient_slot + d,
          1);
  }

  const auto & assembly = this->_assembly;
  const auto dim = assembly.getDimension();
  const auto subdomain = this->_elem.subdomain;

  auto & g = _ad_geometry;
  g.F_inv = F.inverse(dim);

  const auto F_inv_T = g.F_inv.transpose();

  // Coordinate transform factor at the current position, with its derivative through the chain
  // rule on the displacement values
  const Real3 x_value(denseADValue(x(0)), denseADValue(x(1)), denseADValue(x(2)));
  const Real coord_value = assembly.coordTransformFactor(subdomain, x_value);
  const Real3 coord_gradient = assembly.coordTransformFactorGradient(subdomain, x_value);

  DenseADReal<slots> coord = coord_value;
  for (unsigned int d = 0; d < 3; ++d)
    coord += coord_gradient(d) * (x(d) - x_value(d));

  DenseADReal<slots> ratio =
      F.determinant(dim) * coord / assembly.coordTransformFactor(subdomain, X);

  if (this->isSide())
  {
    const auto n = F_inv_T * this->reference_normal(qp);
    const auto norm = sqrt(n * n);

    ratio *= norm;
    g.normal = (1 / norm) * n;
  }

  DenseADReal33<slots> composed;
  for (unsigned int i = 0; i < 3; ++i)
    for (unsigned int j = 0; j < 3; ++j)
      for (unsigned int k = 0; k < 3; ++k)
        composed(i, j) += F_inv_T(i, k) * ref_dxi_dx(k, j);

  g.dxi_dx = composed;
  g.JxW = this->reference_JxW(qp) * ratio;
  g.q_point = x;

  _ad_cached_qp = qp;

  return g;
}

/**
 * The datum of a pointwise AD residual object
 * @tparam slots The number of derivative slots
 */
template <unsigned int slots>
using ADAssemblyDatum = ADDatumBase<AssemblyDatum, slots>;

/**
 * The datum of a pointwise AD material
 * @tparam slots The number of derivative slots
 */
template <unsigned int slots>
using ADMaterialDatum = ADDatumBase<Datum, slots>;

/**
 * Accessor of the quadrature point value of a variable as a dense dual number. A time derivative
 * tag seeds the value slot with the derivative of the time derivative with respect to the
 * solution. The number of derivative slots is that of the datum, which the residual object fixes.
 */
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
   * @param datum The AD datum object of the current thread
   * @param qp The local quadrature point index
   * @returns The value, with its derivative with respect to the slots of the object
   */
  template <typename Base, unsigned int slots>
  KOKKOS_FUNCTION DenseADReal<slots> operator()(ADDatumBase<Base, slots> & datum,
                                                unsigned int qp) const
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
 * Accessor of the quadrature point gradient of a variable as a vector of dense dual numbers. The
 * gradient slots hold the gradient with respect to the reference position, whose component k is
 * seeded in slot k of the slot block of the gradient; on the current configuration the accessor
 * pushes it forward with the dual deformation gradient, which carries the derivatives with respect
 * to the displacements.
 */
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
   * @param datum The AD datum object of the current thread
   * @param qp The local quadrature point index
   * @returns The gradient, with its derivatives with respect to the slots of the object
   */
  template <typename Base, unsigned int slots>
  KOKKOS_FUNCTION DenseADReal3<slots> operator()(ADDatumBase<Base, slots> & datum,
                                                 unsigned int qp) const
  {
    KOKKOS_ASSERT(_slot.finalized);

    const auto first = _slot.first;
    const auto displaced = datum.configuration().displaced();
    const Real3 grad = displaced ? _gradient.reference(datum, qp) : _gradient(datum, qp);

    DenseADReal3<slots> result;
    for (unsigned int k = 0; k < AD_GRADIENT_SLOTS; ++k)
      result(k) = seededDenseADReal<slots>(
          grad(k), first == libMesh::invalid_uint ? libMesh::invalid_uint : first + k, _slot.seed);

    return displaced ? datum.adPushForward(result, qp) : result;
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
