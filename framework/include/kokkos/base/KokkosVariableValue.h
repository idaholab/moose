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

#include "MooseVariableFieldBase.h"

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
///@}

/**
 * The Kokkos wrapper classes for MOOSE-like variable value access
 */
///@{
class VariableValue
{
public:
  /**
   * Default constructor
   */
  VariableValue() = default;
  /**
   * Constructor
   * @param var The Kokkos variable
   * @param dof Whether to get DOF values
   */
  VariableValue(Variable var, bool dof = false) : _var(var), _dof(dof) {}
  /**
   * Constructor
   * @param var The MOOSE variable
   * @param tag The vector tag name
   * @param dof Whether to get DOF values
   */
  VariableValue(const MooseVariableFieldBase & var,
                const TagName & tag = Moose::SOLUTION_TAG,
                bool dof = false)
    : _var(var, tag), _dof(dof)
  {
  }
  /**
   * Constructor
   * @param vars The MOOSE variables
   * @param tag The vector tag name
   * @param dof Whether to get DOF values
   */
  ///@{
  VariableValue(const std::vector<const MooseVariableFieldBase *> & vars,
                const TagName & tag = Moose::SOLUTION_TAG,
                bool dof = false)
    : _var(vars, tag), _dof(dof)
  {
  }
  VariableValue(const std::vector<MooseVariableFieldBase *> & vars,
                const TagName & tag = Moose::SOLUTION_TAG,
                bool dof = false)
    : _var(vars, tag), _dof(dof)
  {
  }
  ///@}

  /**
   * Get whether the variable was coupled
   * @returns Whether the variable was coupled
   */
  KOKKOS_FUNCTION operator bool() const { return _var.coupled(); }

  /**
   * Get the current variable value
   * @param datum The Datum object of the current thread
   * @param idx The local quadrature point index or DOF index
   * @param comp The variable component
   * @returns The variable value
   */
  ///@{
  KOKKOS_FUNCTION Real operator()(Datum & datum, unsigned int idx, unsigned int comp = 0) const;
  KOKKOS_FUNCTION Real operator()(AssemblyDatum & datum, unsigned int idx) const
  {
    return this->operator()(datum, idx, datum.comp());
  }
  ///@}

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

class VariableGradient
{
public:
  /**
   * Default constructor
   */
  VariableGradient() = default;
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
  VariableGradient(const MooseVariableFieldBase & var, const TagName & tag = Moose::SOLUTION_TAG)
    : _var(var, tag)
  {
  }
  /**
   * Constructor
   * @param vars The MOOSE variables
   * @param tag The vector tag name
   */
  ///@{
  VariableGradient(const std::vector<const MooseVariableFieldBase *> & vars,
                   const TagName & tag = Moose::SOLUTION_TAG)
    : _var(vars, tag)
  {
  }
  VariableGradient(const std::vector<MooseVariableFieldBase *> vars,
                   const TagName & tag = Moose::SOLUTION_TAG)
    : _var(vars, tag)
  {
  }
  ///@}

  /**
   * Get whether the variable was coupled
   * @returns Whether the variable was coupled
   */
  KOKKOS_FUNCTION operator bool() const { return _var.coupled(); }

  /**
   * Get the current variable gradient
   * @param datum The Datum object of the current thread
   * @param qp The local quadrature point index
   * @param comp The variable component
   * @returns The variable gradient
   */
  ///@{
  KOKKOS_FUNCTION Real3 operator()(Datum & datum, unsigned int qp, unsigned int comp = 0) const;
  KOKKOS_FUNCTION Real3 operator()(AssemblyDatum & datum, unsigned int qp) const
  {
    return this->operator()(datum, qp, datum.comp());
  }
  ///@}
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
///@}

KOKKOS_FUNCTION inline Real
VariableValue::operator()(Datum & datum, unsigned int idx, unsigned int comp) const
{
  KOKKOS_ASSERT(_var.initialized());

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

      return sys.getVectorDofValue(dof, tag);
    }
    else
    {
      auto & elem = datum.elem();
      auto side = datum.side();
      auto offset = datum.qpOffset();

      return side == libMesh::invalid_uint ? sys.getVectorQpValue(elem, offset + idx, var, tag)
                                           : sys.getVectorQpValueFace(elem, side, idx, var, tag);
    }
  }
  else
    return _var.value(comp);
}

KOKKOS_FUNCTION inline Real3
VariableGradient::operator()(Datum & datum, unsigned int qp, unsigned int comp) const
{
  KOKKOS_ASSERT(_var.initialized());

  if (_var.coupled())
  {
    KOKKOS_ASSERT(!datum.isNodal());

    auto & elem = datum.elem();
    auto side = datum.side();
    auto offset = datum.qpOffset();

    return side == libMesh::invalid_uint
               ? datum.system(_var.sys(comp))
                     .getVectorQpGrad(elem, offset + qp, _var.var(comp), _var.tag())
               : datum.system(_var.sys(comp))
                     .getVectorQpGradFace(elem, side, datum.J(qp), qp, _var.var(comp), _var.tag());
  }
  else
    return Real3(0);
}

} // namespace Moose::Kokkos
