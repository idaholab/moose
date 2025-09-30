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

namespace Moose
{
namespace Kokkos
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
   * @param datum The ResidualDatum object of the current thread
   * @param i The element-local DOF index
   * @param qp The local quadrature point index
   * @returns The shape function
   */
  KOKKOS_FUNCTION Real operator()(ResidualDatum & datum, unsigned int i, unsigned int qp) const
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
   * @param datum The ResidualDatum object of the current thread
   * @param i The element-local DOF index
   * @param qp The local quadrature point index
   * @returns The gradient of the shape function
   */
  KOKKOS_FUNCTION Real3 operator()(ResidualDatum & datum, unsigned int i, unsigned int qp) const
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
   * @param datum The ResidualDatum object of the current thread
   * @param i The element-local DOF index
   * @param qp The local quadrature point index
   * @returns The test function
   */
  KOKKOS_FUNCTION Real operator()(ResidualDatum & datum, unsigned int i, unsigned int qp) const
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
   * @param datum The ResidualDatum object of the current thread
   * @param i The element-local DOF index
   * @param qp The local quadrature point index
   * @returns The gradient of the test function
   */
  KOKKOS_FUNCTION Real3 operator()(ResidualDatum & datum, unsigned int i, unsigned int qp) const
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
///@}

/**
 * The Kokkos wrapper classes for MOOSE-like variable value access
 */
///@{
class VariableValue
{
public:
  /**
   * Constructor
   * @param var The Kokkos variable
   * @param nodal Whether to get nodal values
   */
  VariableValue(Variable var, bool nodal = false) : _var(var), _nodal(nodal)
  {
    if (nodal && !var.nodal())
      mooseError("Cannot get nodal values of a non-nodal variable.");
  }
  /**
   * Constructor
   * @param var The MOOSE variable
   * @param tag The vector tag name
   * @param nodal Whether to get nodal values
   */
  VariableValue(const MooseVariableBase & var,
                const TagName & tag = Moose::SOLUTION_TAG,
                bool nodal = false)
    : _var(var, tag), _nodal(nodal)
  {
    if (nodal && !var.isNodal())
      mooseError("Cannot get nodal values of a non-nodal variable.");
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
   * @returns The variable value
   */
  KOKKOS_FUNCTION Real operator()(Datum & datum, unsigned int qp, unsigned int comp = 0) const
  {
    if (_var.coupled())
    {
      if (_nodal)
      {
        KOKKOS_ASSERT(datum.isNodal());

        auto node = datum.node();
        auto dof = datum.system(_var.sys(comp)).getNodeLocalDofIndex(node, _var.var(comp));

        return datum.system(_var.sys(comp)).getVectorDofValue(dof, _var.tag());
      }
      else
      {
        KOKKOS_ASSERT(!datum.isNodal());

        auto & elem = datum.elem();
        auto side = datum.side();
        auto qp_offset = datum.qpOffset();

        return side == libMesh::invalid_uint
                   ? datum.system(_var.sys(comp))
                         .getVectorQpValue(elem, qp_offset + qp, _var.var(comp), _var.tag())
                   : datum.system(_var.sys(comp))
                         .getVectorQpValueFace(elem, side, qp, _var.var(comp), _var.tag());
      }
    }
    else
      return _var.value(comp);
  }

private:
  /**
   * Coupled Kokkos variable
   */
  Variable _var;
  /**
   * Flag whether nodal values are requested
   */
  const bool _nodal;
};

class VariableGradient
{
public:
  /**
   * Constructor
   * @param var The Kokkos variable
   */
  VariableGradient(Variable var) : _var(var) {}
  /**
   * Constructor
   * @param var The MOOSE variable
   * @param tag The vector tag name
   */
  VariableGradient(const MooseVariableBase & var, const TagName & tag = Moose::SOLUTION_TAG)
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
   * @param qp The local quadrature-point index
   * @param comp The variable component
   * @returns The variable gradient
   */
  KOKKOS_FUNCTION Real3 operator()(Datum & datum, unsigned int qp, unsigned int comp = 0) const
  {
    if (_var.coupled())
    {
      auto & elem = datum.elem();
      auto side = datum.side();
      auto qp_offset = datum.qpOffset();

      return side == libMesh::invalid_uint
                 ? datum.system(_var.sys(comp))
                       .getVectorQpGrad(elem, qp_offset + qp, _var.var(comp), _var.tag())
                 : datum.system(_var.sys(comp))
                       .getVectorQpGradFace(
                           elem, side, datum.J(qp), qp, _var.var(comp), _var.tag());
    }
    else
      return Real3(0);
  }

private:
  /**
   * Coupled Kokkos variable
   */
  Variable _var;
};
///@}

} // namespace Kokkos
} // namespace Moose
