//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosNodalBCBase.h"
#include "KokkosADResidualObject.h"

namespace Moose::Kokkos
{

/**
 * The base class for a user to derive their own Kokkos nodal boundary conditions whose Jacobians
 * are computed with pointwise automatic differentiation (AD).
 *
 * The derived class defines
 *
 * template <typename Derived>
 * KOKKOS_FUNCTION DenseADReal<max_ad_slots>
 * computeQpResidual(const unsigned int qp, ADAssemblyDatum<max_ad_slots> & datum) const;
 *
 * whose derivative directions are the nodal values the BC reads through its AD accessors, one slot
 * per nonlinear variable that the coupling matrix couples to the BC variable, and declares an upper
 * bound on their number as
 *
 *   static constexpr unsigned int max_ad_slots = ...;
 */
class ADNodalBC : public NodalBCBase
{
public:
  static InputParameters validParams();

  ADNodalBC(const InputParameters & parameters);

  virtual void initialSetup() override;
  virtual void computeResidual() override;
  virtual void computeJacobian() override;
  virtual void computeResidualAndJacobian() override;

  /**
   * The parallel computation entry function called by Kokkos for each boundary node
   */
  template <typename Derived>
  KOKKOS_FUNCTION void operator()(ResidualLoop, const ThreadID tid, const Derived & bc) const;

protected:
  /**
   * Create an AD accessor of the nodal value of a coupled variable or of the BC variable
   */
  ///@{
  ADVariableValue adCoupledNodalValue(const std::string & var_name, unsigned int comp = 0);
  ADVariableValue adNodalValue();
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
  /// Whether the residual is being computed
  bool _computing_residual = false;
  /// Whether the Jacobian is being computed
  bool _computing_jacobian = false;
};

inline ADVariableValue
ADNodalBC::adCoupledNodalValue(const std::string & var_name, const unsigned int comp)
{
  ADVariableValue value(kokkosCoupledNodalValue(var_name, comp));
  _ad_setup.add(value.slot(), value.variable(), false);
  return value;
}

inline ADVariableValue
ADNodalBC::adNodalValue()
{
  ADVariableValue value{VariableValue(_var, Moose::SOLUTION_TAG, true)};
  _ad_setup.add(value.slot(), value.variable(), false);
  return value;
}

template <typename Derived>
KOKKOS_FUNCTION void
ADNodalBC::operator()(ResidualLoop, const ThreadID tid, const Derived & bc) const
{
  auto node = kokkosBoundaryNodeID(tid);
  auto & sys = kokkosSystem(_kokkos_var.sys());

  if (!sys.isNodalDefined(node, _kokkos_var.var()))
    return;

  ADAssemblyDatum<Derived::max_ad_slots> datum(
      AssemblyDatum(
          node, kokkosAssembly(), kokkosSystems(), _kokkos_var, _kokkos_var.var(), _configuration),
      _ad_slots);

  const auto local_re = bc.template computeQpResidual<Derived>(0, datum);

  if (_computing_residual)
    accumulateTaggedNodalResidual(false, denseADValue(local_re), node);

  if (_computing_jacobian)
  {
    // The BC replaces the row of the residual variable: the first column sets the row and the rest
    // add to it. A residual without derivatives still sets an empty row.
    if (_ad_slots.numBlocks() == 0)
      accumulateTaggedNodalMatrix(false, 0, node, _kokkos_var.var());

    for (unsigned int b = 0; b < _ad_slots.numBlocks(); ++b)
    {
      const auto & block = _ad_slots.block(b);
      accumulateTaggedNodalMatrix(
          b != 0, denseADDerivative(local_re, block.first), node, block.var);
    }
  }
}

} // namespace Moose::Kokkos
