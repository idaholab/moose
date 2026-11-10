//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosIntegratedBCBase.h"
#include "KokkosADMaterialProperty.h"

namespace Moose::Kokkos
{

/**
 * The base class for a user to derive their own Kokkos integrated boundary conditions whose
 * Jacobians are computed with pointwise automatic differentiation (AD).
 *
 * The residual takes the form of a Kokkos integrated BC, defined through the
 * precomputeQpResidualValue() and precomputeQpResidualGrad() hooks, of which at least one must be
 * defined:
 *
 * template <typename Derived, unsigned int slots>
 * KOKKOS_FUNCTION DenseADReal<slots> precomputeQpResidualValue(const unsigned int qp,
 *                                                              ADAssemblyDatum<slots> & datum)
 * const; template <typename Derived, unsigned int slots> KOKKOS_FUNCTION DenseADReal3<slots>
 * precomputeQpResidualGrad(const unsigned int qp, ADAssemblyDatum<slots> & datum) const;
 *
 * The derivative directions of the dual numbers are the quadrature point quantities the BC
 * reads through its AD accessors: one slot for each value and three for each gradient of a
 * nonlinear variable that the coupling matrix couples to the BC variable. The derived class
 * declares an upper bound on that number as
 *
 *   static constexpr unsigned int max_ad_slots = ...;
 *
 * and creates its accessors with adCoupledValue(), adCoupledGradient(), adCoupledDot(), or their
 * counterparts on the BC variable. A pointwise AD material property, read with
 * getKokkosADMaterialProperty(), carries the derivatives with respect to the inputs of its
 * declaring material: those coupled to the BC variable share the slots of the accessors reading
 * the same quantities and count toward max_ad_slots, and a dependence on the displacements selects
 * the instantiation carrying the displacement slots even on the reference configuration. A residual
 * computed without the Jacobian instantiates the hooks with zero slots, where DenseADReal<0> is
 * Real.
 */
class ADIntegratedBC : public IntegratedBCBase
{
public:
  static InputParameters validParams();

  /// AD integrated BCs dispatch teams of boundary sides, whose threads share the quadrature
  /// point terms of each side
  static constexpr bool uses_team_dispatch = true;

  /**
   * Kokkos function tag of the residual and Jacobian of an object posed on the current
   * configuration
   */
  struct DisplacedResidualLoop
  {
  };
  /**
   * Kokkos function tag of the residual alone, whose hooks are evaluated with plain reals
   */
  struct ResidualOnlyLoop
  {
  };

  ADIntegratedBC(const InputParameters & parameters);

  virtual void initialSetup() override;
  virtual void computeResidual() override;
  virtual void computeJacobian() override;
  virtual void computeResidualAndJacobian() override;

  /**
   * The team operation called by Kokkos for each team of boundary sides
   */
  template <typename Derived, bool displaced>
  KOKKOS_FUNCTION void
  operator()(ConfigurationLoop<ResidualLoop, displaced>,
             const TeamMember<ConfigurationLoop<ResidualLoop, displaced>> & team,
             const Derived & bc) const;
  /**
   * The team operation called by Kokkos for each team of boundary sides of an object posed on the
   * current configuration, whose dual numbers carry the displacement slots
   */
  template <typename Derived, bool displaced>
  KOKKOS_FUNCTION void
  operator()(ConfigurationLoop<DisplacedResidualLoop, displaced>,
             const TeamMember<ConfigurationLoop<DisplacedResidualLoop, displaced>> & team,
             const Derived & bc) const;
  /**
   * The team operation called by Kokkos for each team of boundary sides when only the residual is
   * computed
   */
  template <typename Derived, bool displaced>
  KOKKOS_FUNCTION void
  operator()(ConfigurationLoop<ResidualOnlyLoop, displaced>,
             const TeamMember<ConfigurationLoop<ResidualOnlyLoop, displaced>> & team,
             const Derived & bc) const;

  /**
   * Get the slot table of the AD accessors
   */
  KOKKOS_FUNCTION const ADSlotTable & adSlotTable() const { return _ad_slots; }

protected:
  /**
   * Create AD accessors of a coupled variable or of the BC variable
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
ADIntegratedBC::adCoupledValue(const std::string & var_name, const unsigned int comp)
{
  ADVariableValue value(kokkosCoupledValue(var_name, comp));
  value.setRegistration(_ad_setup.add(value.variable(), false));
  return value;
}

inline ADVariableGradient
ADIntegratedBC::adCoupledGradient(const std::string & var_name, const unsigned int comp)
{
  ADVariableGradient gradient(kokkosCoupledGradient(var_name, comp));
  gradient.setRegistration(_ad_setup.add(gradient.variable(), true));
  return gradient;
}

inline ADVariableValue
ADIntegratedBC::adCoupledDot(const std::string & var_name, const unsigned int comp)
{
  ADVariableValue value(kokkosCoupledDot(var_name, comp));
  value.setRegistration(_ad_setup.add(value.variable(), false));
  return value;
}

inline ADVariableValue
ADIntegratedBC::adValue()
{
  ADVariableValue value{VariableValue(_var)};
  value.setRegistration(_ad_setup.add(value.variable(), false));
  return value;
}

inline ADVariableGradient
ADIntegratedBC::adGradient()
{
  ADVariableGradient gradient{VariableGradient(_var)};
  gradient.setRegistration(_ad_setup.add(gradient.variable(), true));
  return gradient;
}

inline ADVariableValue
ADIntegratedBC::adDot()
{
  ADVariableValue value{VariableValue(_var, Moose::SOLUTION_DOT_TAG)};
  value.setRegistration(_ad_setup.add(value.variable(), false));
  return value;
}

template <typename T>
ADMaterialPropertyReader
ADIntegratedBC::getKokkosADMaterialProperty(const std::string & name)
{
  static_assert(std::is_same_v<T, Real>, "Kokkos AD material properties are of type Real.");

  const auto prop_name = getMaterialPropertyName(name);
  if (const auto value = adConstantPropertyValue(prop_name))
    return ADMaterialPropertyReader(*value);

  ADMaterialPropertyReader reader(
      getKokkosMaterialPropertyByName<Real>(prop_name),
      getKokkosMaterialPropertyByName<Real, 1>(adDerivativesPropertyName(prop_name)));
  reader.setRegistration(_ad_setup.addProperty(reader.derivatives().adInputs()));
  return reader;
}

template <typename Derived, bool displaced>
KOKKOS_FUNCTION void
ADIntegratedBC::operator()(ConfigurationLoop<ResidualLoop, displaced>,
                           const TeamMember<ConfigurationLoop<ResidualLoop, displaced>> & team,
                           const Derived & bc) const
{
  const ADTeamGroup group(team, kokkosAssembly().getMaxQpsPerSide(), numKokkosBoundarySides());
  const auto [elem, side] = kokkosBoundaryElementSideID(group.item());

  AssemblyDatum datum(elem,
                      side,
                      kokkosAssembly(),
                      kokkosSystems(),
                      _kokkos_var,
                      _kokkos_var.var(),
                      _configuration,
                      std::bool_constant<displaced>{});

  assembleADTerms<Derived::max_ad_slots, false>(
      bc, team, group, datum, _ad_slots, _computing_residual, _computing_jacobian);
}

template <typename Derived, bool displaced>
KOKKOS_FUNCTION void
ADIntegratedBC::operator()(
    ConfigurationLoop<DisplacedResidualLoop, displaced>,
    const TeamMember<ConfigurationLoop<DisplacedResidualLoop, displaced>> & team,
    const Derived & bc) const
{
  const ADTeamGroup group(team, kokkosAssembly().getMaxQpsPerSide(), numKokkosBoundarySides());
  const auto [elem, side] = kokkosBoundaryElementSideID(group.item());

  AssemblyDatum datum(elem,
                      side,
                      kokkosAssembly(),
                      kokkosSystems(),
                      _kokkos_var,
                      _kokkos_var.var(),
                      _configuration,
                      std::bool_constant<displaced>{});

  assembleADTerms<ad_displaced_slots<Derived>, true>(
      bc, team, group, datum, _ad_slots, _computing_residual, _computing_jacobian);
}

template <typename Derived, bool displaced>
KOKKOS_FUNCTION void
ADIntegratedBC::operator()(ConfigurationLoop<ResidualOnlyLoop, displaced>,
                           const TeamMember<ConfigurationLoop<ResidualOnlyLoop, displaced>> & team,
                           const Derived & bc) const
{
  const ADTeamGroup group(team, kokkosAssembly().getMaxQpsPerSide(), numKokkosBoundarySides());
  const auto [elem, side] = kokkosBoundaryElementSideID(group.item());

  AssemblyDatum datum(elem,
                      side,
                      kokkosAssembly(),
                      kokkosSystems(),
                      _kokkos_var,
                      _kokkos_var.var(),
                      _configuration,
                      std::bool_constant<displaced>{});

  assembleADTerms<0, false>(bc, team, group, datum, _ad_slots, true, false);
}

} // namespace Moose::Kokkos
