//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosElementUserObject.h"

#ifdef NEML2_ENABLED

#include "KokkosNEML2BatchLayout.h"
#include "MOOSEToNEML2.h"

/**
 * NEML2 base shape of a Kokkos material property type.
 *
 * Only the property types a NEML2 input variable can be filled from are specialized, so an
 * unsupported type is a link error rather than a run-time surprise.
 */
template <typename T>
struct KokkosNEML2BaseShape;

template <>
struct KokkosNEML2BaseShape<Real>
{
  static neml2::TensorShape shape() { return {}; }
};

template <>
struct KokkosNEML2BaseShape<Moose::Kokkos::Real6>
{
  static neml2::TensorShape shape() { return {6}; }
};

/**
 * Gather a Kokkos material property into a NEML2 input variable without leaving the device.
 *
 * The non-Kokkos MOOSEQuantityToNEML2 copies each quadrature point's value into a host buffer during
 * an element loop and transfers the buffer once per solve. Here a device buffer is filled by a single
 * Kokkos kernel and handed to NEML2 as a tensor aliasing that buffer, so the value never crosses the
 * bus.
 *
 * Quadrature points are assigned to batch entries by NEML2BatchLayout, the same numbering the objects
 * retrieving NEML2 outputs into Kokkos material properties use. Because a NEML2 model is evaluated
 * pointwise along the batch dimension, that shared numbering is all that is required for the
 * gathered inputs and the retrieved outputs to correspond.
 *
 * Kokkos user objects run after the Kokkos materials they consume and before the non-Kokkos user
 * objects, so the property this reads is current and the NEML2 executor sees the gathered value.
 *
 * @tparam T The Kokkos property type, whose storage must match the NEML2 variable's component
 * layout. Real corresponds to a NEML2 Scalar and Real6 to SR2, which is Mandel with the component
 * order of SymmetricRankTwoTensor, so components are copied without conversion.
 * @tparam state The property state to read, 0 for the current value and 1 for the old value. A
 * NEML2 model with history takes its old state as an input, and reading it from the Kokkos property
 * the corresponding output was retrieved into keeps that round trip on the device.
 */
template <typename T, unsigned int state>
class KokkosMOOSEQuantityToNEML2 : public Moose::Kokkos::ElementUserObject, public MOOSEToNEML2
{
public:
  static InputParameters validParams();

  KokkosMOOSEQuantityToNEML2(const InputParameters & parameters);

  virtual void initialize() override {}
  virtual void finalize() override {}

  /**
   * Size the device buffer for the current mesh, then fill it
   */
  virtual void compute() override;

  virtual neml2::Tensor gatheredData() const override;

  template <typename Derived>
  KOKKOS_FUNCTION void execute(Moose::Kokkos::Datum & datum) const;

private:
  /// Number of Reals per quadrature point, fixed by the property type's storage
  static constexpr unsigned int _n_components = sizeof(T) / sizeof(Real);

  /// Property to gather
  const Moose::Kokkos::MaterialProperty<T> _prop;

  /// Device buffer the NEML2 input tensor aliases
  Moose::Kokkos::Array<Real> _buffer;

  /// Assignment of quadrature points to NEML2 batch entries
  Moose::Kokkos::NEML2BatchLayout _layout;
};

typedef KokkosMOOSEQuantityToNEML2<Real, 0> KokkosMOOSERealToNEML2;
typedef KokkosMOOSEQuantityToNEML2<Real, 1> KokkosMOOSEOldRealToNEML2;
typedef KokkosMOOSEQuantityToNEML2<Moose::Kokkos::Real6, 0>
    KokkosMOOSESymmetricRankTwoTensorToNEML2;
typedef KokkosMOOSEQuantityToNEML2<Moose::Kokkos::Real6, 1>
    KokkosMOOSEOldSymmetricRankTwoTensorToNEML2;

template <typename T, unsigned int state>
template <typename Derived>
KOKKOS_FUNCTION void
KokkosMOOSEQuantityToNEML2<T, state>::execute(Moose::Kokkos::Datum & datum) const
{
  for (unsigned int qp = 0; qp < datum.n_qps(); ++qp)
  {
    const T & value = _prop(datum, qp);
    const Real * source = reinterpret_cast<const Real *>(&value);

    const auto batch = _layout.index(datum, qp);

    for (unsigned int c = 0; c < _n_components; ++c)
      _buffer[batch * _n_components + c] = source[c];
  }
}

#endif
