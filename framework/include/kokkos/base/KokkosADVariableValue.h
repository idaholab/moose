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
#include "KokkosADSlotBlock.h"

#include "libmesh/utility.h"

namespace Moose::Kokkos
{

/**
 * A dual slot of the instantiation of a pointwise AD object that carries the displacement
 * derivatives. Its dual numbers hold intermediate quantities, whose derivatives map to the true
 * slots of the slot table after the hooks are evaluated.
 */
struct ADDualSlot
{
  /// Quantity held by a dual slot
  enum class Kind : unsigned char
  {
    /// No quantity, for the slots beyond those assigned
    NONE,
    /// The value of a variable, whose derivative is that of its true slot
    VALUE,
    /// A component of the gradient of a variable on the configuration
    GRADIENT,
    /// A component of the quadrature point position on the configuration
    POSITION,
    /// An AD material property, whose stored derivatives map to the true slots
    PROPERTY
  };

  /// Quantity held by the slot
  Kind kind = Kind::NONE;
  /// For VALUE, the true slot. For GRADIENT, the first true slot of the reference gradient, or
  /// libMesh::invalid_uint if it carries no derivatives. For POSITION, the displacement component.
  /// For PROPERTY, the registration index of the property reader.
  unsigned int target = libMesh::invalid_uint;
  /// For GRADIENT, the component of the gradient
  unsigned int component = 0;
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

  /**
   * Set the dual slots of the instantiation carrying the displacement derivatives
   * @param slots The quantity of each dual slot, padded to the number of dual slots
   * @param accessor_dual The first dual slot of each registered accessor, or libMesh::invalid_uint
   * @param property_dual The dual slot of each registered property reader
   * @param position_dual The first dual slot of the position, or libMesh::invalid_uint
   */
  void setDualSlots(const std::vector<ADDualSlot> & slots,
                    const std::vector<unsigned int> & accessor_dual,
                    const std::vector<unsigned int> & property_dual,
                    unsigned int position_dual);
  /**
   * Get the quantity of a dual slot of the instantiation carrying the displacement derivatives
   */
  KOKKOS_FUNCTION const ADDualSlot & dual(const unsigned int s) const
  {
    KOKKOS_ASSERT(s < _dual.size());
    return _dual[s];
  }
  /**
   * Get the first dual slot of a registered accessor in the instantiation carrying the
   * displacement derivatives, or libMesh::invalid_uint if it carries no derivatives
   * @param accessor The registration index of the accessor
   */
  KOKKOS_FUNCTION unsigned int accessorDual(const unsigned int accessor) const
  {
    KOKKOS_ASSERT(accessor < _accessor_dual.size());
    return _accessor_dual[accessor];
  }
  /**
   * Get the dual slot of a registered property reader in the instantiation carrying the
   * displacement derivatives
   * @param property The registration index of the reader
   */
  KOKKOS_FUNCTION unsigned int propertyDual(const unsigned int property) const
  {
    KOKKOS_ASSERT(property < _property_dual.size());
    return _property_dual[property];
  }
  /**
   * Get the first dual slot of the position in the instantiation carrying the displacement
   * derivatives, or libMesh::invalid_uint if the instantiation has no position slots
   */
  KOKKOS_FUNCTION unsigned int positionDual() const { return _position_dual; }

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
  /// Quantity of each dual slot of the instantiation carrying the displacement derivatives
  Array<ADDualSlot> _dual;
  /// First dual slot of each registered accessor
  Array<unsigned int> _accessor_dual;
  /// Dual slot of each registered property reader
  Array<unsigned int> _property_dual;
  /// First dual slot of the position, or libMesh::invalid_uint
  unsigned int _position_dual = libMesh::invalid_uint;
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
  KOKKOS_FUNCTION unsigned int index() const { return _index; }
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
 * Plain values that a pointwise AD object records at a quadrature point while its hooks are
 * evaluated with the dual slots of the displaced instantiation. The chain rule after the hooks reads
 * them to map the derivatives with respect to the dual slots to the true slots.
 * @tparam slots The number of dual slots
 */
template <unsigned int slots>
struct ADChainRecord
{
  /// Value of each gradient dual slot on the configuration, zero for other slots
  Real value[slots > 0 ? slots : 1] = {};
  /// Stored derivatives of the property of each property dual slot at the quadrature point, or
  /// nullptr. They are contiguous in the property storage.
  const Real * derivatives[slots > 0 ? slots : 1] = {};
};

/**
 * The datum of a pointwise AD object, which fixes the number of derivative slots of its dual
 * numbers. The instantiation carrying the displacement derivatives has intermediate dual slots:
 * - the value of each variable,
 * - the gradient of each gradient accessor on the configuration,
 * - the position,
 * - each AD material property.
 * The datum then holds a chain record. The accessors seed the dual slots and record their plain
 * values in it. Otherwise the dual slots are the true slots, and the geometry is plain.
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
   * Constructor, which constructs the plain datum in place
   * @param table The slot table of the object
   * @param chain The chain record of the displaced instantiation, or nullptr
   * @param args The constructor arguments of the plain datum of the element, side, or node. Pass a
   * host constant such as libMesh::invalid_uint as a prvalue. Device code cannot bind a reference
   * to it.
   */
  template <typename... Args>
  KOKKOS_FUNCTION
  ADDatumBase(const ADSlotTable & table, ADChainRecord<slots> * const chain, Args &&... args)
    : Base(std::forward<Args>(args)...), _table(table), _chain(chain)
  {
    // On the current configuration the position carries displacement derivatives only as a dual
    // number
    if (chain)
      this->adDisplaced(this->displaced());
  }

  /**
   * Get the slot table of the object
   */
  KOKKOS_FUNCTION const ADSlotTable & table() const { return _table; }
  /**
   * Get the chain record, or nullptr if the dual slots are the true slots
   */
  KOKKOS_FUNCTION ADChainRecord<slots> * chain() const { return _chain; }

  /**
   * Get the quadrature point coordinates on the configuration as dual numbers. In the instantiation
   * carrying the displacement derivatives, the position slots carry their derivatives.
   * @param qp The local quadrature point index
   */
  KOKKOS_FUNCTION DenseADReal3<slots> adQPoint(const unsigned int qp)
  {
    if (_chain && this->displaced())
    {
      const auto first = _table.positionDual();
      if (first == libMesh::invalid_uint)
        ::Kokkos::abort("A pointwise AD Kokkos object read the quadrature point position on the "
                        "current configuration, but its max_ad_displaced_slots leaves no slots for "
                        "the position. Remove max_ad_displaced_slots from the class, or increase "
                        "it by 3.");

      const Real3 x = this->current_q_point(qp);
      DenseADReal3<slots> result;
      for (unsigned int c = 0; c < AD_POSITION_SLOTS; ++c)
        result(c) = seededDenseADReal<slots>(x(c), first + c, 1);

      return result;
    }

    const Real3 x = this->q_point(qp);
    return DenseADReal3<slots>(x(0), x(1), x(2));
  }

  /**
   * Map the derivatives of dual numbers with respect to the dual slots of the displaced
   * instantiation to the true slots of the slot table. Let g = F^{-T} G be the gradient of an
   * accessor on the configuration, with reference gradient G and F = I + grad_X u. Then
   *
   *   df/dG_k            = sum_i df/dg_i (F^{-1})_{ki}
   *   df/d(grad_X u_c)_d = -g_c sum_i df/dg_i (F^{-1})_{di}
   *
   * The position x = X + u gives df/du_c = df/dx_c. The stored derivatives of a property give
   * the derivatives with respect to its inputs.
   * @param qp The local quadrature point index
   * @param f The dual numbers
   * @param add The function that adds a derivative with respect to a true slot, called as
   * add(index of the dual number, slot, derivative)
   */
  template <unsigned int n, typename Accumulate>
  KOKKOS_FUNCTION void
  chainDerivatives(const unsigned int qp, const DenseADReal<slots> (&f)[n], Accumulate && add);

  /**
   * Add the derivatives of the JxW weight and of the map to the reference element with respect to
   * the displacement slots to the derivatives of the residual coefficients
   * @param qp The local quadrature point index
   * @param coef The JxW-weighted coefficients: the value coefficient followed by the gradient
   * coefficient mapped to the reference element
   * @param grad The JxW-weighted gradient coefficient before the map
   * @param add The function adding a derivative of a coefficient with respect to a true slot,
   * called as add(coefficient, slot, derivative)
   */
  template <typename Accumulate>
  KOKKOS_FUNCTION void geometryDerivatives(const unsigned int qp,
                                           const Real (&coef)[4],
                                           const Real3 & grad,
                                           Accumulate && add);

private:
  /// Slot table of the object
  const ADSlotTable & _table;
  /// Chain record of the instantiation carrying the displacement derivatives, or nullptr
  ADChainRecord<slots> * const _chain;
};

template <typename Base, unsigned int slots>
template <unsigned int n, typename Accumulate>
KOKKOS_FUNCTION void
ADDatumBase<Base, slots>::chainDerivatives(const unsigned int qp,
                                           const DenseADReal<slots> (&f)[n],
                                           Accumulate && add)
{
  KOKKOS_ASSERT(_chain);

  if constexpr (slots > 0)
  {
    const auto displaced = this->displaced();
    const auto & disp = this->_configuration.displacements();
    const auto & F_inv = this->F_inv(qp);

    // F^{-1} applied to the derivatives with respect to the gradient of the current accessor. Its
    // three dual slots are consecutive.
    Real h[n][3] = {};

    constexprFor<slots>(
        [&](auto slot)
        {
          constexpr unsigned int s = decltype(slot)::value;
          const auto & dual = _table.dual(s);

          switch (dual.kind)
          {
            case ADDualSlot::Kind::VALUE:
              for (unsigned int t = 0; t < n; ++t)
                add(t, dual.target, f[t].derivatives()[s]);
              break;

            case ADDualSlot::Kind::POSITION:
              if (const auto vs = _table.displacementValueSlot(dual.target);
                  vs != libMesh::invalid_uint)
                for (unsigned int t = 0; t < n; ++t)
                  add(t, vs, f[t].derivatives()[s]);
              break;

            case ADDualSlot::Kind::PROPERTY:
              if (const Real * const stored = _chain->derivatives[s])
              {
                const auto & map = _table.propertySlots(dual.target, this->subdomain());
                for (unsigned int m = 0; m < map.size(); ++m)
                  if (map[m] != libMesh::invalid_uint)
                    for (unsigned int t = 0; t < n; ++t)
                      add(t, map[m], f[t].derivatives()[s] * stored[m]);
              }
              break;

            case ADDualSlot::Kind::GRADIENT:
            {
              // A runtime component index would place F_inv, with the datum, in local memory
              Real3 F_inv_column;
              constexprIndex<3>(dual.component,
                                [&](auto c)
                                {
                                  for (unsigned int k = 0; k < 3; ++k)
                                    F_inv_column(k) = F_inv(k, c);
                                });

              for (unsigned int t = 0; t < n; ++t)
                for (unsigned int k = 0; k < 3; ++k)
                  h[t][k] += F_inv_column(k) * f[t].derivatives()[s];

              // The last component of the accessor flushes its reference and deformation terms
              if (dual.component == AD_GRADIENT_SLOTS - 1)
              {
                if (dual.target != libMesh::invalid_uint)
                  for (unsigned int t = 0; t < n; ++t)
                    for (unsigned int k = 0; k < 3; ++k)
                      add(t, dual.target + k, h[t][k]);

                if (displaced)
                  for (unsigned int c = 0; c < disp.components(); ++c)
                    if (const auto gs = _table.displacementGradientSlot(c);
                        gs != libMesh::invalid_uint)
                    {
                      const Real g_c = _chain->value[s - (AD_GRADIENT_SLOTS - 1) + c];
                      for (unsigned int t = 0; t < n; ++t)
                        for (unsigned int k = 0; k < 3; ++k)
                          add(t, gs + k, -g_c * h[t][k]);
                    }

                for (unsigned int t = 0; t < n; ++t)
                  for (unsigned int k = 0; k < 3; ++k)
                    h[t][k] = 0;
              }
              break;
            }

            case ADDualSlot::Kind::NONE:
              break;
          }
        });
  }
}

template <typename Base, unsigned int slots>
template <typename Accumulate>
KOKKOS_FUNCTION void
ADDatumBase<Base, slots>::geometryDerivatives(const unsigned int qp,
                                              const Real (&coef)[4],
                                              const Real3 & grad,
                                              Accumulate && add)
{
  if (!this->displaced())
    return;

  const auto & disp = this->_configuration.displacements();
  const auto & F_inv = this->F_inv(qp);
  const auto & dxi_dx = this->dxi_dx(qp);
  const auto & grad_log_coord = this->grad_log_coord(qp);
  const Real3 normal = this->isSide() ? this->normals(qp) : Real3(0);

  // The coefficients are linear in JxW, whose logarithm has the derivatives
  //
  //   d(ln JxW)/du_c             = (grad ln coord)_c
  //   d(ln JxW)/d(grad_X u_c)_d  = (F^{-1})_{dc} - n_c (n . (F^{-T})_{:,d})
  //
  // where the normal term applies on sides only. The map to the reference element, dxi/dx =
  // F^{-T} dxi/dX, has d(dxi/dx)_{ci}/d(grad_X u_c)_d = -(dxi/dx)_{ci} (F^{-1})_{d:}.
  for (unsigned int c = 0; c < disp.components(); ++c)
  {
    // Gather the entries of component c, since a runtime component index would place the
    // geometry, with the datum, in local memory
    Real grad_log_coord_c = 0, normal_c = 0;
    Real3 F_inv_column, dxi_dx_row;
    constexprIndex<3>(c,
                      [&](auto i)
                      {
                        grad_log_coord_c = grad_log_coord(i);
                        normal_c = normal(i);
                        for (unsigned int k = 0; k < 3; ++k)
                        {
                          F_inv_column(k) = F_inv(k, i);
                          dxi_dx_row(k) = dxi_dx(i, k);
                        }
                      });

    if (const auto vs = _table.displacementValueSlot(c); vs != libMesh::invalid_uint)
      for (unsigned int t = 0; t < 4; ++t)
        add(t, vs, coef[t] * grad_log_coord_c);

    if (const auto gs = _table.displacementGradientSlot(c); gs != libMesh::invalid_uint)
      for (unsigned int d = 0; d < 3; ++d)
      {
        Real dlog = F_inv_column(d);
        if (this->isSide())
        {
          Real n_col = 0;
          for (unsigned int k = 0; k < 3; ++k)
            n_col += normal(k) * F_inv(d, k);
          dlog -= normal_c * n_col;
        }

        Real F_inv_grad_d = 0;
        for (unsigned int k = 0; k < 3; ++k)
          F_inv_grad_d += F_inv(d, k) * grad(k);

        add(0, gs + d, coef[0] * dlog);
        for (unsigned int i = 0; i < 3; ++i)
          add(1 + i, gs + d, coef[1 + i] * dlog - dxi_dx_row(i) * F_inv_grad_d);
      }
  }
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
    // A value slot needs no chain rule, so the dual slot carries the seed
    const auto & table = datum.table();
    const auto slot = datum.chain() ? table.accessorDual(_slot.index()) : _slot.first(table);

    return seededDenseADReal<slots>(_value(datum, qp), slot, _slot.seed());
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
 * true gradient slots hold the gradient with respect to the reference position. Slot k of the
 * block holds component k. The accessor pushes it forward with the plain deformation gradient. In
 * the displaced instantiation, the dual slots hold the gradient on the configuration instead. The
 * accessor records that value. The chain rule after the hooks uses it to recover the derivatives
 * with respect to the reference gradient and the displacements.
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
    DenseADReal3<slots> result;

    if (auto * const chain = datum.chain())
    {
      const Real3 grad = _gradient(datum, qp);
      const auto first = datum.table().accessorDual(_slot.index());

      for (unsigned int k = 0; k < AD_GRADIENT_SLOTS; ++k)
        result(k) = seededDenseADReal<slots>(
            grad(k), first == libMesh::invalid_uint ? libMesh::invalid_uint : first + k, 1);

      // A gradient accessor reads no time derivative, so its seed is one
      KOKKOS_ASSERT(_slot.seed() == 1);

      if (first != libMesh::invalid_uint)
        for (unsigned int k = 0; k < AD_GRADIENT_SLOTS; ++k)
          constexprIndex<slots>(first + k, [&](auto s) { chain->value[s] = grad(k); });

      return result;
    }

    const auto first = _slot.first(datum.table());
    const auto displaced = datum.displaced();
    const Real3 grad = displaced ? _gradient.reference(datum, qp) : _gradient(datum, qp);

    for (unsigned int k = 0; k < AD_GRADIENT_SLOTS; ++k)
      result(k) = seededDenseADReal<slots>(grad(k),
                                           first == libMesh::invalid_uint ? libMesh::invalid_uint
                                                                          : first + k,
                                           _slot.seed());

    return displaced ? datum.F_inv(qp).transpose() * result : result;
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
