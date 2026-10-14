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
#include "KokkosADResidualObject.h"

namespace Moose::Kokkos
{

/**
 * The base class for a user to derive their own Kokkos materials whose properties are computed
 * with pointwise automatic differentiation (AD).
 *
 * The properties are computed by the computeQpProperties() hook, whose dual numbers carry the
 * derivatives with respect to the quadrature point quantities the material reads through its AD
 * accessors:
 *
 * template <typename Derived, unsigned int slots>
 * KOKKOS_FUNCTION void computeQpProperties(const unsigned int qp,
 *                                          ADMaterialDatum<slots> & datum) const;
 *
 * The derivative directions are those of an AD residual object: one slot for each value and three
 * for each reference gradient of a nonlinear variable at its current state, followed on the
 * current configuration by the displacement slots. The derived class declares an upper bound on
 * the number of accessor slots as
 *
 *   static constexpr unsigned int max_ad_slots = ...;
 *
 * and creates its accessors with adCoupledValue() and adCoupledGradient().
 */
class ADMaterial : public Material
{
public:
  static InputParameters validParams();

  ADMaterial(const InputParameters & parameters);

  virtual void initialSetup() override;

  /**
   * Get the slot table of the AD accessors
   */
  KOKKOS_FUNCTION const ADSlotTable & adSlotTable() const { return _ad_slots; }

  /**
   * The parallel computation entry functions called by Kokkos, which evaluate the properties with
   * the dual numbers of the configuration of the material
   */
  ///@{
  using Material::operator();
  template <typename Derived>
  KOKKOS_FUNCTION void
  operator()(ElementCompute, const ThreadID tid, const Derived & material) const;
  template <typename Derived>
  KOKKOS_FUNCTION void operator()(SideCompute, const ThreadID tid, const Derived & material) const;
  template <typename Derived>
  KOKKOS_FUNCTION void
  operator()(NeighborCompute, const ThreadID tid, const Derived & material) const;
  ///@}

protected:
  /**
   * Create AD accessors of a coupled variable
   */
  ///@{
  ADVariableValue adCoupledValue(const std::string & var_name, unsigned int comp = 0);
  ADVariableGradient adCoupledGradient(const std::string & var_name, unsigned int comp = 0);
  ///@}

private:
  /**
   * Evaluate the properties at the quadrature points of an element or side
   * @tparam slots The number of derivative slots of the configuration of the material
   * @param datum The Datum object of the element or side
   * @param material The material of the final derived type
   */
  template <typename Derived, unsigned int slots>
  KOKKOS_FUNCTION void computeADProperties(const Datum & datum, const Derived & material) const;
  /**
   * Evaluate the properties with the instantiation matching the configuration of the material
   */
  template <typename Derived>
  KOKKOS_FUNCTION void dispatchADProperties(const Datum & datum, const Derived & material) const;

  /// Registration of the AD accessors created during construction
  ADSlotSetup _ad_setup;
  /// Slot table of the AD accessors
  ADSlotTable _ad_slots;
};

inline ADVariableValue
ADMaterial::adCoupledValue(const std::string & var_name, const unsigned int comp)
{
  ADVariableValue value(kokkosCoupledValue(var_name, comp));
  _ad_setup.add(value.slot(), value.variable(), false);
  return value;
}

inline ADVariableGradient
ADMaterial::adCoupledGradient(const std::string & var_name, const unsigned int comp)
{
  ADVariableGradient gradient(kokkosCoupledGradient(var_name, comp));
  _ad_setup.add(gradient.slot(), gradient.variable(), true);
  return gradient;
}

template <typename Derived, unsigned int slots>
KOKKOS_FUNCTION void
ADMaterial::computeADProperties(const Datum & datum, const Derived & material) const
{
  ADMaterialDatum<slots> ad_datum(datum, _ad_slots);

  const unsigned int num_qps =
      _constant_option == PropertyConstantOption::NONE ? ad_datum.n_qps() : 1;

  for (unsigned int qp = 0; qp < num_qps; ++qp)
    material.template computeQpProperties<Derived, slots>(qp, ad_datum);
}

template <typename Derived>
KOKKOS_FUNCTION void
ADMaterial::dispatchADProperties(const Datum & datum, const Derived & material) const
{
  // A material on the current configuration evaluates the instantiation whose dual numbers also
  // carry the displacement slots
  if (_configuration.displaced())
    computeADProperties<Derived, Derived::max_ad_slots + AD_DISPLACEMENT_SLOTS>(datum, material);
  else
    computeADProperties<Derived, Derived::max_ad_slots>(datum, material);
}

template <typename Derived>
KOKKOS_FUNCTION void
ADMaterial::operator()(ElementCompute, const ThreadID tid, const Derived & material) const
{
  // When constant option is subdomain, elem is an arbitrary element in each subdomain, and thus
  // datum is invalid
  auto elem = kokkosElementID(tid);

  dispatchADProperties(
      Datum(elem, libMesh::invalid_uint, kokkosAssembly(), kokkosSystems(), _configuration),
      material);
}

template <typename Derived>
KOKKOS_FUNCTION void
ADMaterial::operator()(SideCompute, const ThreadID tid, const Derived & material) const
{
  auto [elem, side] = kokkosElementSideID(tid);

  dispatchADProperties(Datum(elem, side, kokkosAssembly(), kokkosSystems(), _configuration),
                       material);
}

template <typename Derived>
KOKKOS_FUNCTION void
ADMaterial::operator()(NeighborCompute, const ThreadID tid, const Derived & material) const
{
  auto [elem, side] = kokkosElementSideID(tid);

  dispatchADProperties(Datum(elem, side, kokkosAssembly(), kokkosSystems(), _configuration),
                       material);
}

} // namespace Moose::Kokkos
