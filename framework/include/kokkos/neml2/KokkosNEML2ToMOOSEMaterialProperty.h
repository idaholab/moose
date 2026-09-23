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
 * Aliasing requires the NEML2 output to be contiguous and resident on the Kokkos device, both of
 * which are checked. A derivative retrieved through dynamic_expand() is not necessarily contiguous,
 * and such a variable is rejected rather than silently read through the wrong strides.
 *
 * @tparam T The Kokkos property type, whose storage must match the NEML2 variable's component
 * layout. Real6 corresponds to SR2 and Real66 to SSR4; both are Mandel with the component order of
 * SymmetricRankTwoTensor, so the components are copied without conversion.
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

  /// Whether the batch offsets have been gathered for the current mesh
  bool _offsets_current = false;

  /// Aliases the contiguous NEML2 output's device storage; no copy of the values is made
  Moose::Kokkos::Array<Real> _source;
  /// NEML2 batch offset of each element, indexed by contiguous element ID
  Moose::Kokkos::Array<dof_id_type> _batch_offset;

  /// Emitted property
  Moose::Kokkos::MaterialProperty<T> _prop;
};

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
  const auto batch = _batch_offset[datum.elem().id] + qp;

  // The property type's storage matches the NEML2 variable's component layout, so the components
  // transfer without conversion
  T value;
  Real * destination = reinterpret_cast<Real *>(&value);
  for (unsigned int c = 0; c < _n_components; ++c)
    destination[c] = _source[batch * _n_components + c];

  _prop(datum, qp) = value;
}

#endif
