//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosKernelBase.h"
#include "KokkosADMaterialProperty.h"

namespace Moose::Kokkos
{

/**
 * The base class for a user to derive their own Kokkos kernels whose Jacobians are computed with
 * pointwise automatic differentiation (AD).
 *
 * The residual takes the form of a Kokkos kernel, defined through the precomputeQpResidualValue()
 * and precomputeQpResidualGrad() hooks, of which at least one must be defined:
 *
 * template <typename Derived, unsigned int slots>
 * KOKKOS_FUNCTION DenseADReal<slots> precomputeQpResidualValue(const unsigned int qp,
 *                                                              ADAssemblyDatum<slots> & datum)
 * const; template <typename Derived, unsigned int slots> KOKKOS_FUNCTION DenseADReal3<slots>
 * precomputeQpResidualGrad(const unsigned int qp, ADAssemblyDatum<slots> & datum) const;
 *
 * The derivative directions of the dual numbers are the quadrature point quantities the kernel
 * reads through its AD accessors: one slot for each value and three for each gradient of a
 * nonlinear variable that the coupling matrix couples to the kernel variable. The derived class
 * declares an upper bound on that number as
 *
 *   static constexpr unsigned int max_ad_slots = ...;
 *
 * and creates its accessors with adCoupledValue(), adCoupledGradient(), adCoupledDot(), or their
 * counterparts on the kernel variable. A pointwise AD material property, read with
 * getKokkosADMaterialProperty(), carries the derivatives with respect to the inputs of its
 * declaring material: those coupled to the kernel variable share the slots of the accessors reading
 * the same quantities and count toward max_ad_slots, and a dependence on the displacements selects
 * the instantiation carrying the displacement slots even on the reference configuration.
 */
class ADKernel : public KernelBase
{
public:
  static InputParameters validParams();

  /// AD kernels dispatch one team per element, whose threads share the quadrature point terms
  static constexpr bool uses_team_dispatch = true;

  /**
   * Kokkos function tag of the residual and Jacobian of an object posed on the current
   * configuration
   */
  struct DisplacedResidualLoop
  {
  };

  ADKernel(const InputParameters & parameters);

  virtual void initialSetup() override;
  virtual void computeResidual() override;
  virtual void computeJacobian() override;
  virtual void computeResidualAndJacobian() override;

  /**
   * The team operation called by Kokkos for each element
   */
  template <typename Derived>
  KOKKOS_FUNCTION void
  operator()(ResidualLoop, const TeamMember<ResidualLoop> & team, const Derived & kernel) const;
  /**
   * The team operation called by Kokkos for each element of an object posed on the current
   * configuration, whose dual numbers carry the displacement slots
   */
  template <typename Derived>
  KOKKOS_FUNCTION void operator()(DisplacedResidualLoop,
                                  const TeamMember<DisplacedResidualLoop> & team,
                                  const Derived & kernel) const;

  /**
   * Get the slot table of the AD accessors
   */
  KOKKOS_FUNCTION const ADSlotTable & adSlotTable() const { return _ad_slots; }

protected:
  /**
   * Create AD accessors of a coupled variable or of the kernel variable
   */
  ///@{
  ADVariableValue adCoupledValue(const std::string & var_name, unsigned int comp = 0);
  ADVariableGradient adCoupledGradient(const std::string & var_name, unsigned int comp = 0);
  ADVariableValue adCoupledDot(const std::string & var_name, unsigned int comp = 0);
  ADVariableValue adValue();
  ADVariableGradient adGradient();
  ADVariableValue adDot();
  ///@}

  /**
   * Get a pointwise AD material property declared by a Kokkos AD material
   * @tparam T The property data type
   * @param name The property name or the parameter name containing the property name
   * @returns The reader of the property
   */
  template <typename T>
  ADMaterialPropertyReader getKokkosADMaterialProperty(const std::string & name);

  /// Test function values and gradients, for derived classes that need them in their hooks
  ///@{
  const VariableTestValue _test;
  const VariableTestGradient _grad_test;
  ///@}

private:
  /**
   * Dispatch the residual and Jacobian computation
   */
  void dispatch();

  /// Registration of the AD accessors created during construction
  ADSlotSetup _ad_setup;
  /// Slot table of the AD accessors
  ADSlotTable _ad_slots;
  /// Number of derivative slots declared by the derived class
  unsigned int _max_ad_slots = 0;
  /// Whether the residual is being computed
  bool _computing_residual = false;
  /// Whether the Jacobian is being computed
  bool _computing_jacobian = false;
};

inline ADVariableValue
ADKernel::adCoupledValue(const std::string & var_name, const unsigned int comp)
{
  ADVariableValue value(kokkosCoupledValue(var_name, comp));
  _ad_setup.add(value.slot(), value.variable(), false);
  return value;
}

inline ADVariableGradient
ADKernel::adCoupledGradient(const std::string & var_name, const unsigned int comp)
{
  ADVariableGradient gradient(kokkosCoupledGradient(var_name, comp));
  _ad_setup.add(gradient.slot(), gradient.variable(), true);
  return gradient;
}

inline ADVariableValue
ADKernel::adCoupledDot(const std::string & var_name, const unsigned int comp)
{
  ADVariableValue value(kokkosCoupledDot(var_name, comp));
  _ad_setup.add(value.slot(), value.variable(), false);
  return value;
}

inline ADVariableValue
ADKernel::adValue()
{
  ADVariableValue value{VariableValue(_var)};
  _ad_setup.add(value.slot(), value.variable(), false);
  return value;
}

inline ADVariableGradient
ADKernel::adGradient()
{
  ADVariableGradient gradient{VariableGradient(_var)};
  _ad_setup.add(gradient.slot(), gradient.variable(), true);
  return gradient;
}

inline ADVariableValue
ADKernel::adDot()
{
  ADVariableValue value{VariableValue(_var, Moose::SOLUTION_DOT_TAG)};
  _ad_setup.add(value.slot(), value.variable(), false);
  return value;
}

template <typename T>
ADMaterialPropertyReader
ADKernel::getKokkosADMaterialProperty(const std::string & name)
{
  static_assert(std::is_same_v<T, Real>, "Kokkos AD material properties are of type Real.");

  const auto prop_name = getMaterialPropertyName(name);
  if (const auto value = adConstantPropertyValue(prop_name))
    return ADMaterialPropertyReader(*value);

  ADMaterialPropertyReader reader(
      getKokkosMaterialPropertyByName<Real>(prop_name),
      getKokkosMaterialPropertyByName<Real, 1>(adDerivativesPropertyName(prop_name)));
  _ad_setup.addProperty(reader.slotMap(), reader.derivatives().adInputs());
  return reader;
}

template <typename Derived>
KOKKOS_FUNCTION void
ADKernel::operator()(ResidualLoop,
                     const TeamMember<ResidualLoop> & team,
                     const Derived & kernel) const
{
  auto elem = kokkosBlockElementID(team.league_rank());

  AssemblyDatum datum(elem,
                      libMesh::invalid_uint,
                      kokkosAssembly(),
                      kokkosSystems(),
                      _kokkos_var,
                      _kokkos_var.var(),
                      _configuration);

  assembleADTerms<Derived::max_ad_slots>(
      kernel, team, datum, _ad_slots, _computing_residual, _computing_jacobian);
}

template <typename Derived>
KOKKOS_FUNCTION void
ADKernel::operator()(DisplacedResidualLoop,
                     const TeamMember<DisplacedResidualLoop> & team,
                     const Derived & kernel) const
{
  auto elem = kokkosBlockElementID(team.league_rank());

  AssemblyDatum datum(elem,
                      libMesh::invalid_uint,
                      kokkosAssembly(),
                      kokkosSystems(),
                      _kokkos_var,
                      _kokkos_var.var(),
                      _configuration);

  assembleADTerms<Derived::max_ad_slots + AD_DISPLACEMENT_SLOTS>(
      kernel, team, datum, _ad_slots, _computing_residual, _computing_jacobian);
}

} // namespace Moose::Kokkos
