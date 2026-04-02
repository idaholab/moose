//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosDatum.h"

#include "MooseVariableBase.h"

namespace Moose::Kokkos
{

/**
 * The Kokkos wrapper classes for MOOSE-like shape function access
 */
///@{
class VariablePhiValue
{
public:
  /**
   * Get the current shape function
   * @param datum The AssemblyDatum object of the current thread
   * @param i The element-local DOF index
   * @param qp The local quadrature point index
   * @returns The shape function
   */
  KOKKOS_FUNCTION Real operator()(AssemblyDatum & datum, unsigned int i, unsigned int qp) const
  {
    auto & elem = datum.elem();
    auto side = datum.side();
    auto fe = datum.jfe();

    return side == libMesh::invalid_uint
               ? datum.assembly().getPhi(elem.subdomain, elem.type, fe)(i, qp)
               : datum.assembly().getPhiFace(elem.subdomain, elem.type, fe)(side)(i, qp);
  }
};

class VariablePhiGradient
{
public:
  /**
   * Get the gradient of the current shape function
   * @param datum The AssemblyDatum object of the current thread
   * @param i The element-local DOF index
   * @param qp The local quadrature point index
   * @returns The gradient of the shape function
   */
  KOKKOS_FUNCTION Real3 operator()(AssemblyDatum & datum, unsigned int i, unsigned int qp) const
  {
    auto & elem = datum.elem();
    auto side = datum.side();
    auto fe = datum.jfe();

    return datum.J(qp) *
           (side == libMesh::invalid_uint
                ? datum.assembly().getGradPhi(elem.subdomain, elem.type, fe)(i, qp)
                : datum.assembly().getGradPhiFace(elem.subdomain, elem.type, fe)(side)(i, qp));
  }
};

class VariableTestValue
{
public:
  /**
   * Get the current test function
   * @param datum The AssemblyDatum object of the current thread
   * @param i The element-local DOF index
   * @param qp The local quadrature point index
   * @returns The test function
   */
  KOKKOS_FUNCTION Real operator()(AssemblyDatum & datum, unsigned int i, unsigned int qp) const
  {
    auto & elem = datum.elem();
    auto side = datum.side();
    auto fe = datum.ife();

    return side == libMesh::invalid_uint
               ? datum.assembly().getPhi(elem.subdomain, elem.type, fe)(i, qp)
               : datum.assembly().getPhiFace(elem.subdomain, elem.type, fe)(side)(i, qp);
  }
};

class VariableTestGradient
{
public:
  /**
   * Get the gradient of the current test function
   * @param datum The AssemblyDatum object of the current thread
   * @param i The element-local DOF index
   * @param qp The local quadrature point index
   * @returns The gradient of the test function
   */
  KOKKOS_FUNCTION Real3 operator()(AssemblyDatum & datum, unsigned int i, unsigned int qp) const
  {
    auto & elem = datum.elem();
    auto side = datum.side();
    auto fe = datum.ife();

    return datum.J(qp) *
           (side == libMesh::invalid_uint
                ? datum.assembly().getGradPhi(elem.subdomain, elem.type, fe)(i, qp)
                : datum.assembly().getGradPhiFace(elem.subdomain, elem.type, fe)(side)(i, qp));
  }
};

using ADVariablePhiValue = VariablePhiValue;
using ADVariablePhiGradient = VariablePhiGradient;
using ADVariableTestValue = VariableTestValue;
using ADVariableTestGradient = VariableTestGradient;

///@}

/**
 * The Kokkos wrapper classes for MOOSE-like variable value access
 */
///@{
template <bool is_ad>
class VariableValueTempl
{
  using data_type = std::conditional_t<is_ad, ADReal, Real>;

public:
  /**
   * Default constructor
   */
  VariableValueTempl() = default;
  /**
   * Constructor
   * @param var The Kokkos variable
   * @param dof Whether to get DOF values
   */
  VariableValueTempl(Variable var, bool dof = false) : _var(var), _dof(dof) {}
  /**
   * Constructor
   * @param var The MOOSE variable
   * @param tag The vector tag name
   * @param dof Whether to get DOF values
   */
  VariableValueTempl(const MooseVariableBase & var,
                     const TagName & tag = Moose::SOLUTION_TAG,
                     bool dof = false)
    : _var(var, tag), _dof(dof)
  {
  }

  /**
   * Get whether the variable was coupled
   * @returns Whether the variable was coupled
   */
  KOKKOS_FUNCTION operator bool() const { return _var.coupled(); }

  /**
   * Get the current variable value
   * @param datum The Datum object of the current thread
   * @param qp The local quadrature point index
   * @param comp The variable component
   * @param do_derivatives Whether to compute derivatives (only meaningful for AD)
   * @returns The variable value
   */
  KOKKOS_FUNCTION auto operator()(Datum & datum,
                                  unsigned int qp,
                                  unsigned int comp = 0,
                                  bool do_derivatives = false) const;

  /**
   * Get the current variable value
   * @param datum The AssemblyDatum object of the current thread
   * @param qp The local quadrature point index
   * @param comp The variable component
   * @returns The variable value
   */
  KOKKOS_FUNCTION auto
  operator()(AssemblyDatum & datum, unsigned int qp, unsigned int comp = 0) const
  {
    return operator()(datum, qp, comp, _var.coupled() ? _var.sys(comp) == datum.sys() : false);
  }

  /**
   * Get the Kokkos variable
   * @returns The Kokkos variable
   */
  KOKKOS_FUNCTION const Variable & variable() const { return _var; }

private:
  /**
   * Coupled Kokkos variable
   */
  Variable _var;
  /**
   * Flag whether DOF values are requested
   */
  bool _dof = false;
};

template <bool is_ad>
KOKKOS_FUNCTION auto
VariableValueTempl<is_ad>::operator()(Datum & datum,
                                      unsigned int idx,
                                      unsigned int comp,
                                      [[maybe_unused]] bool do_derivatives) const
{
  KOKKOS_ASSERT(_var.initialized());

  data_type value;

  if (_var.coupled())
  {
    auto & sys = datum.system(_var.sys(comp));
    auto var = _var.var(comp);
    auto tag = _var.tag();

    if (_dof)
    {
      unsigned int dof;

      if (datum.isNodal())
      {
        auto node = datum.node();
        dof = sys.getNodeLocalDofIndex(node, 0, var);
      }
      else
      {
        auto elem = datum.elem().id;
        dof = sys.getElemLocalDofIndex(elem, idx, var);
      }

      value = sys.getVectorDofValue(dof, tag);
    }
    else
    {
      auto & elem = datum.elem();
      auto side = datum.side();

      if constexpr (is_ad)
        value = side == libMesh::invalid_uint
                    ? sys.getVectorQpADValue(elem, datum.qpOffset() + idx, var, tag, do_derivatives)
                    : sys.getVectorQpADValueFace(elem, side, idx, var, tag, do_derivatives);
      else
        value = side == libMesh::invalid_uint
                    ? sys.getVectorQpValue(elem, datum.qpOffset() + idx, var, tag)
                    : sys.getVectorQpValueFace(elem, side, idx, var, tag);
    }
  }
  else
    value = _var.value(comp);

  return value;
}

using VariableValue = VariableValueTempl<false>;
using ADVariableValue = VariableValueTempl<true>;

template <bool is_ad>
class VariableGradientTempl
{
  using data_type = std::conditional_t<is_ad, ADReal3, Real3>;

public:
  /**
   * Default constructor
   */
  VariableGradientTempl() = default;
  /**
   * Constructor
   * @param var The Kokkos variable
   */
  VariableGradientTempl(Variable var) : _var(var) {}
  /**
   * Constructor
   * @param var The MOOSE variable
   * @param tag The vector tag name
   */
  VariableGradientTempl(const MooseVariableBase & var, const TagName & tag = Moose::SOLUTION_TAG)
    : _var(var, tag)
  {
  }

  /**
   * Get whether the variable was coupled
   * @returns Whether the variable was coupled
   */
  KOKKOS_FUNCTION operator bool() const { return _var.coupled(); }

  /**
   * Get the current variable gradient
   * @param datum The Datum object of the current thread
   * @param idx The local quadrature point or DOF index
   * @param comp The variable component
   * @param do_derivatives Whether to compute derivatives (only meaningful for AD)
   * @returns The variable gradient
   */
  KOKKOS_FUNCTION auto operator()(Datum & datum,
                                  unsigned int idx,
                                  unsigned int comp = 0,
                                  bool do_derivatives = false) const;

  /**
   * Get the current variable gradient
   * @param datum The AssemblyDatum object of the current thread
   * @param idx The local quadrature point or DOF index
   * @param comp The variable component
   * @returns The variable gradient
   */
  KOKKOS_FUNCTION auto
  operator()(AssemblyDatum & datum, unsigned int idx, unsigned int comp = 0) const
  {
    return operator()(datum, idx, comp, _var.coupled() ? _var.sys(comp) == datum.sys() : false);
  }

  /**
   * Get the Kokkos variable
   * @returns The Kokkos variable
   */
  KOKKOS_FUNCTION const Variable & variable() const { return _var; }

private:
  /**
   * Coupled Kokkos variable
   */
  Variable _var;
};

template <bool is_ad>
KOKKOS_FUNCTION auto
VariableGradientTempl<is_ad>::operator()(Datum & datum,
                                         unsigned int qp,
                                         unsigned int comp,
                                         [[maybe_unused]] bool do_derivatives) const
{
  KOKKOS_ASSERT(_var.initialized());

  data_type grad;

  if (_var.coupled())
  {
    KOKKOS_ASSERT(!datum.isNodal());

    auto & elem = datum.elem();
    auto side = datum.side();

    if constexpr (is_ad)
      grad = side == libMesh::invalid_uint
                 ? datum.system(_var.sys(comp))
                       .getVectorQpADGrad(
                           elem, datum.J(qp), qp, _var.var(comp), _var.tag(), do_derivatives)
                 : datum.system(_var.sys(comp))
                       .getVectorQpADGradFace(
                           elem, side, datum.J(qp), qp, _var.var(comp), _var.tag(), do_derivatives);
    else
      grad =
          side == libMesh::invalid_uint
              ? datum.system(_var.sys(comp))
                    .getVectorQpGrad(elem, datum.qpOffset() + qp, _var.var(comp), _var.tag())
              : datum.system(_var.sys(comp))
                    .getVectorQpGradFace(elem, side, datum.J(qp), qp, _var.var(comp), _var.tag());
  }

  return grad;
}

using VariableGradient = VariableGradientTempl<false>;
using ADVariableGradient = VariableGradientTempl<true>;
///@}

} // namespace Moose::Kokkos
