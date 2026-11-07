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

#include "libmesh/utility.h"

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
   * Set the first slot of a registered accessor
   * @param accessor The registration index of the accessor
   * @param first The first slot of its quantity, or libMesh::invalid_uint if it carries no
   * derivatives
   */
  void setAccessorFirst(unsigned int accessor, unsigned int first);
  /**
   * Set the slot map of a registered AD material property reader in a subdomain
   * @param property The registration index of the reader
   * @param num_subdomains The number of contiguous subdomains
   * @param subdomain The contiguous subdomain ID
   * @param slots The slot of the reader for each stored derivative, or libMesh::invalid_uint
   */
  void setPropertySlots(unsigned int property,
                        unsigned int num_subdomains,
                        unsigned int subdomain,
                        const std::vector<unsigned int> & slots);
  /**
   * Copy the registered slot blocks, the first slots of the accessors, and the slot maps of the
   * property readers to the device
   */
  void finalize();
  /**
   * Get the first slot of a registered accessor, or libMesh::invalid_uint if it carries no
   * derivatives
   * @param accessor The registration index of the accessor
   */
  KOKKOS_FUNCTION unsigned int accessorFirst(const unsigned int accessor) const
  {
    KOKKOS_ASSERT(accessor < _accessor_first.size());
    return _accessor_first[accessor];
  }
  /**
   * Get the slot map of a registered AD material property reader in a subdomain
   * @param property The registration index of the reader
   * @param subdomain The contiguous subdomain ID
   * @returns The slot of the reader for each stored derivative, or libMesh::invalid_uint
   */
  KOKKOS_FUNCTION const Array<unsigned int> & propertySlots(const unsigned int property,
                                                           const unsigned int subdomain) const
  {
    KOKKOS_ASSERT(property < _property_slots.size());
    return _property_slots[property][subdomain];
  }
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
   * Get the first slot of a registered accessor on host, or libMesh::invalid_uint if it carries no
   * derivatives
   * @param accessor The registration index of the accessor
   */
  unsigned int hostAccessorFirst(const unsigned int accessor) const
  {
    return libmesh_vector_at(_host_accessor_first, accessor);
  }
  /**
   * Get the slot map of a registered AD material property reader in a subdomain on host
   * @param property The registration index of the reader
   * @param subdomain The contiguous subdomain ID
   */
  const std::vector<unsigned int> & hostPropertySlots(const unsigned int property,
                                                      const unsigned int subdomain) const
  {
    return libmesh_vector_at(libmesh_vector_at(_host_property_slots, property), subdomain);
  }

  /**
   * Leave unused slots so that the next registered quantity starts at a given slot
   * @param first The slot at which the next registered quantity starts
   */
  void reserve(const unsigned int first) { _num_slots = std::max(_num_slots, first); }
  /**
   * Register the displacements of the current configuration, whose values and reference gradients
   * occupy the AD_DISPLACEMENT_SLOTS slots following those of the accessors. Register them only
   * when at least one component is seeded.
   * @param components The system and variable numbers of each displacement component
   * @param seeded Whether each component carries derivatives, which holds for the nonlinear
   * displacements of the residual system that the coupling matrix couples to the residual variable
   */
  void addDisplacements(const std::vector<std::pair<unsigned int, unsigned int>> & components,
                        const std::vector<bool> & seeded);
  /**
   * Get whether the displacement slots are registered, so that the object dispatches the
   * instantiation carrying them
   */
  KOKKOS_FUNCTION bool hasDisplacements() const { return _has_displacements; }
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
  /// First slot of each registered accessor
  std::vector<unsigned int> _host_accessor_first;
  /// First slot of each registered accessor, available on device
  Array<unsigned int> _accessor_first;
  /// Slot map of each registered property reader in each contiguous subdomain
  std::vector<std::vector<std::vector<unsigned int>>> _host_property_slots;
  /// Slot map of each registered property reader in each contiguous subdomain, available on
  /// device
  Array<Array<Array<unsigned int>>> _property_slots;
  /// Number of slot blocks, available on device
  unsigned int _num_blocks = 0;
  /// Number of slots
  unsigned int _num_slots = 0;
  /// First slot of each displacement component, or libMesh::invalid_uint
  unsigned int _displacement_first[3] = {
      libMesh::invalid_uint, libMesh::invalid_uint, libMesh::invalid_uint};
  /// Whether the displacement slots are registered
  bool _has_displacements = false;
};

/**
 * Registration of an AD accessor with the slot setup of its object. The accessor keeps the index,
 * by which it finds its first slot in the slot table of the object, so that copies of the accessor
 * stay valid.
 */
struct ADAccessorRegistration
{
  /// Registration index of the accessor
  unsigned int index = libMesh::invalid_uint;
  /// Host source of the seed of a time derivative, the derivative of the time derivative with
  /// respect to the solution, which the time integrator updates between dispatches, or nullptr
  const Real * seed_source = nullptr;
};

/**
 * Registration index and derivative seed of an AD accessor
 */
class ADAccessorSlot
{
public:
  ADAccessorSlot() = default;
  /**
   * Copy constructor for parallel dispatch, which refreshes the seed of a time derivative
   */
  ADAccessorSlot(const ADAccessorSlot & object)
    : _index(object._index), _seed_source(object._seed_source), _seed(object._seed)
  {
    refresh();
  }
  ADAccessorSlot & operator=(const ADAccessorSlot & object) = default;

  /**
   * Set the registration of the accessor
   * @param registration The registration returned by the slot setup of the object
   */
  void set(const ADAccessorRegistration & registration)
  {
    _index = registration.index;
    _seed_source = registration.seed_source;
    refresh();
  }
  /**
   * Get the registration index
   */
  unsigned int index() const { return _index; }
  /**
   * Get the first slot of the quantity, or libMesh::invalid_uint if it carries no derivatives
   * @param table The slot table of the object
   */
  KOKKOS_FUNCTION unsigned int first(const ADSlotTable & table) const
  {
    return table.accessorFirst(_index);
  }
  /**
   * Get the derivative of the quantity with respect to its first slot
   */
  KOKKOS_FUNCTION Real seed() const { return _seed; }

private:
  /**
   * Refresh the seed from its source, before the accessor is copied to the device
   */
  void refresh()
  {
    if (_seed_source)
      _seed = *_seed_source;
  }

  /// Registration index of the accessor
  unsigned int _index = libMesh::invalid_uint;
  /// Host source of the seed of a time derivative, or nullptr
  const Real * _seed_source = nullptr;
  /// Derivative of the quantity with respect to its first slot
  Real _seed = 1;
};

/**
 * The datum of a pointwise AD object, which fixes the number of derivative slots of its dual
 * numbers. On the current configuration with displacement slots it also carries the geometry as
 * dual numbers whose derivatives with respect to the displacement slots give the variation of the
 * geometry, so that every quantity computed from it carries its displacement derivatives.
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
   * Get the geometry of a quadrature point of the current configuration
   * @param qp The local quadrature point index
   * @returns The geometry
   */
  KOKKOS_FUNCTION const ADGeometry & adGeometry(const unsigned int qp);

  /**
   * Get the transformed Jacobian weight as a dual number
   */
  KOKKOS_FUNCTION DenseADReal<slots> adJxW(const unsigned int qp)
  {
    if constexpr (slots >= AD_DISPLACEMENT_SLOTS)
      if (this->_configuration.displaced())
      {
        const auto & g = adGeometry(qp);
        DenseADReal<slots> JxW = g.JxW;

        for (unsigned int c = 0; c < this->_configuration.displacements().components(); ++c)
        {
          if (const auto vs = _table.displacementValueSlot(c); vs != libMesh::invalid_uint)
            JxW.derivatives()[vs] += g.JxW * g.grad_log_coord(c);

          if (const auto gs = _table.displacementGradientSlot(c); gs != libMesh::invalid_uint)
            for (unsigned int d = 0; d < 3; ++d)
            {
              Real dlog = g.F_inv_T(c, d);
              if (this->isSide())
              {
                Real n_col = 0;
                for (unsigned int k = 0; k < 3; ++k)
                  n_col += g.normal(k) * g.F_inv_T(k, d);
                dlog -= g.normal(c) * n_col;
              }
              JxW.derivatives()[gs + d] += g.JxW * dlog;
            }
        }

        return JxW;
      }

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
      {
        const auto & g = adGeometry(qp);
        DenseADReal3<slots> x(g.q_point(0), g.q_point(1), g.q_point(2));

        for (unsigned int c = 0; c < this->_configuration.displacements().components(); ++c)
          if (const auto vs = _table.displacementValueSlot(c); vs != libMesh::invalid_uint)
            x(c).derivatives()[vs] = 1;

        return x;
      }

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
      {
        const auto & g = adGeometry(qp);
        DenseADReal3<slots> grad = g.F_inv_T * grad_X;

        for (unsigned int c = 0; c < this->_configuration.displacements().components(); ++c)
          if (const auto gs = _table.displacementGradientSlot(c); gs != libMesh::invalid_uint)
          {
            const Real grad_c = denseADValue(grad(c));
            for (unsigned int d = 0; d < 3; ++d)
              for (unsigned int i = 0; i < 3; ++i)
                grad(i).derivatives()[gs + d] -= grad_c * g.F_inv_T(i, d);
          }

        return grad;
      }

    return this->F_inv(qp).transpose() * grad_X;
  }

  /**
   * Map a gradient coefficient to the reference element, so that its product with the reference
   * test function gradient equals its product with the gradient on the configuration, with the
   * derivatives of the map on the configuration
   * @param grad The gradient coefficient
   * @param qp The local quadrature point index
   */
  KOKKOS_FUNCTION DenseADReal3<slots> adToReferenceElement(const DenseADReal3<slots> & grad,
                                                           const unsigned int qp)
  {
    if constexpr (slots >= AD_DISPLACEMENT_SLOTS)
      if (this->_configuration.displaced())
      {
        const auto & g = adGeometry(qp);
        DenseADReal3<slots> mapped = g.dxi_dx.transpose() * grad;

        for (unsigned int c = 0; c < this->_configuration.displacements().components(); ++c)
          if (const auto gs = _table.displacementGradientSlot(c); gs != libMesh::invalid_uint)
            for (unsigned int d = 0; d < 3; ++d)
            {
              // Component d of F^{-1} applied to the value of the coefficient
              Real F_inv_grad_d = 0;
              for (unsigned int k = 0; k < 3; ++k)
                F_inv_grad_d += g.F_inv_T(k, d) * denseADValue(grad(k));

              for (unsigned int i = 0; i < 3; ++i)
                mapped(i).derivatives()[gs + d] -= g.dxi_dx(c, i) * F_inv_grad_d;
            }

        return mapped;
      }

    return this->dxi_dx(qp).transpose() * grad;
  }

private:
  /// Slot table of the object
  const ADSlotTable & _table;
  /// Quadrature point of the cached dual geometry
  unsigned int _ad_cached_qp = libMesh::invalid_uint;
  /// Cached geometry of the current configuration
  ADGeometry _ad_geometry;
};

template <typename Base, unsigned int slots>
KOKKOS_FUNCTION const ADGeometry &
ADDatumBase<Base, slots>::adGeometry(const unsigned int qp)
{
  KOKKOS_ASSERT(!this->isNodal());

  if (_ad_cached_qp == qp)
    return _ad_geometry;

  auto & g = _ad_geometry;

  // The plain datum composes the deformation gradient into its geometry on the current
  // configuration
  g.F_inv_T = this->F_inv(qp).transpose();
  g.dxi_dx = this->dxi_dx(qp);
  g.JxW = this->JxW(qp);
  g.grad_log_coord = this->grad_log_coord(qp);
  g.normal = this->isSide() ? this->normals(qp) : Real3(0);

  // The plain datum withholds the position of an AD object on the current configuration, so that
  // objects read it with its derivatives through adQPoint()
  const auto & disp = this->_configuration.displacements();
  g.q_point = this->reference_q_point(qp);

  for (unsigned int c = 0; c < disp.components(); ++c)
  {
    const auto & sys = this->system(disp.sys(c));
    const auto var = disp.var(c);
    const auto tag = disp.tag();

    g.q_point(c) += !this->isSide()
                        ? sys.getVectorQpValue(this->_elem, this->_qp_offset + qp, var, tag)
                        : sys.getVectorQpValueFace(this->_elem, this->_side, qp, var, tag);
  }

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
    return seededDenseADReal<slots>(_value(datum, qp), _slot.first(datum.table()), _slot.seed());
  }

  KOKKOS_FUNCTION const Variable & variable() const { return _value.variable(); }

  /**
   * Set the registration of the accessor with the slot setup of its object
   */
  void setRegistration(const ADAccessorRegistration & registration) { _slot.set(registration); }
  /**
   * Get the registration index of the accessor
   */
  unsigned int registrationIndex() const { return _slot.index(); }

private:
  /// Plain value accessor
  VariableValue _value;
  /// Registration and seed of the value
  ADAccessorSlot _slot;
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
    const auto first = _slot.first(datum.table());
    const auto displaced = datum.configuration().displaced();
    const Real3 grad = displaced ? _gradient.reference(datum, qp) : _gradient(datum, qp);

    DenseADReal3<slots> result;
    for (unsigned int k = 0; k < AD_GRADIENT_SLOTS; ++k)
      result(k) = seededDenseADReal<slots>(grad(k),
                                           first == libMesh::invalid_uint ? libMesh::invalid_uint
                                                                          : first + k,
                                           _slot.seed());

    return displaced ? datum.adPushForward(result, qp) : result;
  }

  KOKKOS_FUNCTION const Variable & variable() const { return _gradient.variable(); }

  /**
   * Set the registration of the accessor with the slot setup of its object
   */
  void setRegistration(const ADAccessorRegistration & registration) { _slot.set(registration); }
  /**
   * Get the registration index of the accessor
   */
  unsigned int registrationIndex() const { return _slot.index(); }

private:
  /// Plain gradient accessor
  VariableGradient _gradient;
  /// Registration and seed of the gradient
  ADAccessorSlot _slot;
};

} // namespace Moose::Kokkos
