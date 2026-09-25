//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosMaterial.h"
#include "KokkosNEML2BatchLayout.h"

#ifdef NEML2_ENABLED

#include "neml2/tensors/Tensor.h"

class NEML2ModelExecutor;

/**
 * Copies a NEML2 output into a Kokkos material property without leaving the device.
 *
 * The non-Kokkos NEML2ToMOOSEMaterialProperty slices one batch entry per quadrature point and
 * copies each into host memory, which costs one device-to-host transfer per point per property.
 * Here the NEML2 output tensor's device storage is aliased and read on the device, so the whole
 * scatter is a single kernel and no data crosses the bus.
 *
 * Aliasing requires contiguous storage resident on the Kokkos device, and a NEML2 output is not
 * always contiguous. The two ways it can fail have opposite remedies.
 *
 * A state variable is a slice of the model's state axis, so its batch stride is the width of that axis,
 * and compacting it copies exactly the values needed. That copy is made on the device once per solve.
 *
 * A derivative that does not vary along the batch, such as the tangent of a linear elastic model,
 * arrives from dynamic_expand() as a broadcast with a batch stride of zero: one set of components that
 * every batch entry aliases. Compacting that would duplicate it once per quadrature point, so the
 * single set of components is aliased and every quadrature point reads it instead. Declaring this
 * material constant_on = SUBDOMAIN then stores it once per subdomain as well, which is only correct
 * for a broadcast output and is rejected otherwise.
 *
 * NEML2 batch entries correspond to element quadrature points, so only the element copy of this
 * material is evaluated. A face or neighbor copy is handed a datum carrying a facial quadrature point
 * offset, which does not index the batch, so those copies leave their property alone.
 *
 * @tparam T The Kokkos property type, whose storage must match the NEML2 variable's component
 * layout. Real corresponds to a NEML2 Scalar, Real6 to SR2 and Real66 to SSR4; the latter two are
 * Mandel with the component order of SymmetricRankTwoTensor, so the components are copied without
 * conversion.
 */
template <typename T>
class KokkosNEML2ToMOOSEMaterialProperty : public Moose::Kokkos::Material
{
public:
  static InputParameters validParams();

  KokkosNEML2ToMOOSEMaterialProperty(const InputParameters & parameters);

  /**
   * Refresh the device alias before dispatching. NEML2 reassigns its output tensors on every solve,
   * so the pointer aliased last time is not necessarily still valid.
   */
  virtual void computeProperties() override;

  template <typename Derived>
  KOKKOS_FUNCTION void computeQpProperties(const unsigned int qp, Datum & datum) const;

private:
  /**
   * Determine whether the NEML2 output does not vary along the batch, and check that its components
   * are packed so that the one set of them can be aliased
   */
  bool isBatchBroadcast() const;

  /// Number of Reals per quadrature point, fixed by the property type's storage
  static constexpr unsigned int _n_components = sizeof(T) / sizeof(Real);

  /// User object managing the execution of the NEML2 model
  const NEML2ModelExecutor & _execute_neml2_model;

  /**
   * Reference to the requested output. The executor requires outputs to be retrieved during
   * construction and refills this slot on every solve, so the reference stays valid while the
   * storage it refers to does not.
   */
  const neml2::Tensor & _value;

  /// Aliases the contiguous NEML2 output's device storage; no copy of the values is made
  Moose::Kokkos::Array<Real> _source;
  /**
   * Owns the compacted copy of a strided NEML2 output. Allocated once and rewritten in place, so that
   * it is reused between solves and so that the functor copy a dispatcher holds shares this storage
   * rather than pinning a tensor that would never be released. Unallocated when the output is already
   * contiguous or is a broadcast, in which cases its own storage is aliased.
   */
  Moose::Kokkos::Array<Real> _compacted;

  /**
   * Whether the NEML2 output does not vary along the batch, in which case _source holds the one set of
   * components that every quadrature point reads
   */
  bool _broadcast = false;
  /// Assignment of quadrature points to NEML2 batch entries
  Moose::Kokkos::NEML2BatchLayout _layout;

  /// Emitted property
  Moose::Kokkos::MaterialProperty<T> _prop;
};

typedef KokkosNEML2ToMOOSEMaterialProperty<Real> KokkosNEML2ToMOOSERealMaterialProperty;
typedef KokkosNEML2ToMOOSEMaterialProperty<Moose::Kokkos::Real6>
    KokkosNEML2ToMOOSESymmetricRankTwoTensorMaterialProperty;
typedef KokkosNEML2ToMOOSEMaterialProperty<Moose::Kokkos::Real66>
    KokkosNEML2ToMOOSESymmetricRankFourTensorMaterialProperty;

template <typename T>
template <typename Derived>
KOKKOS_FUNCTION void
KokkosNEML2ToMOOSEMaterialProperty<T>::computeQpProperties(const unsigned int qp,
                                                           Datum & datum) const
{
  // A broadcast output holds one set of components that the whole batch shares
  const auto batch = _broadcast ? 0 : _layout.index(datum, qp);

  // The property type's storage matches the NEML2 variable's component layout, so the components
  // transfer without conversion
  T value;
  Real * destination = reinterpret_cast<Real *>(&value);
  for (unsigned int c = 0; c < _n_components; ++c)
    destination[c] = _source[batch * _n_components + c];

  _prop(datum, qp) = value;
}

#endif
