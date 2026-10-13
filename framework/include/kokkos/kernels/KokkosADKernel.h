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

namespace Moose::Kokkos
{

/**
 * The base class for a user to derive their own Kokkos kernels whose Jacobians are computed with
 * pointwise automatic differentiation (AD).
 *
 * The residual takes the form of a Kokkos kernel, defined through the precomputeQpResidualValue()
 * and precomputeQpResidualGrad() hooks, of which at least one must be defined:
 *
 * template <typename Derived>
 * KOKKOS_FUNCTION DenseADReal<max_ad_slots> precomputeQpResidualValue(const unsigned int qp,
 *                                                                     AssemblyDatum & datum) const;
 * template <typename Derived>
 * KOKKOS_FUNCTION DenseADReal3<max_ad_slots> precomputeQpResidualGrad(const unsigned int qp,
 *                                                                     AssemblyDatum & datum) const;
 *
 * The derivative directions of the dual numbers are the quadrature point quantities the kernel
 * reads through its AD accessors: one slot for each value and three for each gradient of a
 * nonlinear variable that the coupling matrix couples to the kernel variable. The derived class
 * declares an upper bound on that number as
 *
 *   static constexpr unsigned int max_ad_slots = ...;
 *
 * and creates its accessors with adCoupledValue(), adCoupledGradient(), adCoupledDot(), or their
 * counterparts on the kernel variable.
 */
class ADKernel : public KernelBase
{
public:
  static InputParameters validParams();

  /// AD kernels dispatch one team per element, whose threads share the quadrature point terms
  static constexpr bool uses_team_dispatch = true;

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
   * Get the slot table of the AD accessors
   */
  KOKKOS_FUNCTION const ADSlotTable & adSlotTable() const { return _ad_slots; }

protected:
  /**
   * Create AD accessors of a coupled variable or of the kernel variable
   * @tparam slots The max_ad_slots of the derived class
   */
  ///@{
  template <unsigned int slots>
  ADVariableValue<slots> adCoupledValue(const std::string & var_name, unsigned int comp = 0);
  template <unsigned int slots>
  ADVariableGradient<slots> adCoupledGradient(const std::string & var_name, unsigned int comp = 0);
  template <unsigned int slots>
  ADVariableValue<slots> adCoupledDot(const std::string & var_name, unsigned int comp = 0);
  template <unsigned int slots>
  ADVariableValue<slots> adValue();
  template <unsigned int slots>
  ADVariableGradient<slots> adGradient();
  template <unsigned int slots>
  ADVariableValue<slots> adDot();
  ///@}

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

template <unsigned int slots>
ADVariableValue<slots>
ADKernel::adCoupledValue(const std::string & var_name, const unsigned int comp)
{
  ADVariableValue<slots> value(kokkosCoupledValue(var_name, comp));
  _ad_setup.add(value.slot(), value.variable(), false);
  return value;
}

template <unsigned int slots>
ADVariableGradient<slots>
ADKernel::adCoupledGradient(const std::string & var_name, const unsigned int comp)
{
  ADVariableGradient<slots> gradient(kokkosCoupledGradient(var_name, comp));
  _ad_setup.add(gradient.slot(), gradient.variable(), true);
  return gradient;
}

template <unsigned int slots>
ADVariableValue<slots>
ADKernel::adCoupledDot(const std::string & var_name, const unsigned int comp)
{
  ADVariableValue<slots> value(kokkosCoupledDot(var_name, comp));
  _ad_setup.add(value.slot(), value.variable(), false);
  return value;
}

template <unsigned int slots>
ADVariableValue<slots>
ADKernel::adValue()
{
  ADVariableValue<slots> value{VariableValue(_var)};
  _ad_setup.add(value.slot(), value.variable(), false);
  return value;
}

template <unsigned int slots>
ADVariableGradient<slots>
ADKernel::adGradient()
{
  ADVariableGradient<slots> gradient{VariableGradient(_var)};
  _ad_setup.add(gradient.slot(), gradient.variable(), true);
  return gradient;
}

template <unsigned int slots>
ADVariableValue<slots>
ADKernel::adDot()
{
  ADVariableValue<slots> value{VariableValue(_var, Moose::SOLUTION_DOT_TAG)};
  _ad_setup.add(value.slot(), value.variable(), false);
  return value;
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

  assembleADTerms(kernel, team, datum, _ad_slots, _computing_residual, _computing_jacobian);
}

} // namespace Moose::Kokkos
