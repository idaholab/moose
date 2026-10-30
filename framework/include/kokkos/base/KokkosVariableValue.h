//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosDatum.h"

#include "MooseError.h"
#include "MooseVariableFieldBase.h"
#include "SystemBase.h"

namespace Moose::Kokkos
{

inline void
checkVariable(const Variable & var, bool expect_vector, const std::string & wrapper_name)
{
  if (!var.initialized())
    mooseError("Attempted to construct Kokkos ", wrapper_name, " with an uninitialized variable.");

  if (var.vector() != expect_vector)
    mooseError("Kokkos ",
               wrapper_name,
               " cannot be constructed with ",
               var.vector() ? "vector" : "scalar",
               " variables.");
}

/**
 * The Kokkos wrapper classes for MOOSE-like shape function access
 */
///@{
template <bool is_test>
class VariableShapeValue
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
    if (!is_test && datum.trialOverride())
      return datum.trialOverride()->phi;

    auto & elem = datum.elem();
    auto side = datum.side();
    auto fe = is_test ? datum.ife() : datum.jfe();

    return side == libMesh::invalid_uint
               ? datum.assembly().getPhi(elem.subdomain, elem.type, fe)(i, qp)
               : datum.assembly().getPhiFace(elem.subdomain, elem.type, fe)(side)(i, qp);
  }
};

template <bool is_test>
class VariableShapeGradient
{
public:
  /**
   * Get the gradient of the current shape function in reference space
   * @param datum The AssemblyDatum object of the current thread
   * @param i The element-local DOF index
   * @param qp The local quadrature point index
   * @returns The reference-space gradient of the shape function
   */
  KOKKOS_FUNCTION const Real3 &
  reference(AssemblyDatum & datum, unsigned int i, unsigned int qp) const;

  /**
   * Get the gradient of the current shape function
   * @param datum The AssemblyDatum object of the current thread
   * @param i The element-local DOF index
   * @param qp The local quadrature point index
   * @returns The gradient of the shape function
   */
  KOKKOS_FUNCTION Real3 operator()(AssemblyDatum & datum, unsigned int i, unsigned int qp) const
  {
    if (!is_test && datum.trialOverride())
      return datum.trialOverride()->grad_phi;

    return datum.dxi_dx(qp) * reference(datum, i, qp);
  }
};

template <bool is_test>
KOKKOS_FUNCTION const Real3 &
VariableShapeGradient<is_test>::reference(AssemblyDatum & datum,
                                          unsigned int i,
                                          unsigned int qp) const
{
  auto & elem = datum.elem();
  auto side = datum.side();
  auto fe = is_test ? datum.ife() : datum.jfe();

  return side == libMesh::invalid_uint
             ? datum.assembly().getGradPhi(elem.subdomain, elem.type, fe)(i, qp)
             : datum.assembly().getGradPhiFace(elem.subdomain, elem.type, fe)(side)(i, qp);
}

using VariablePhiValue = VariableShapeValue<false>;
using VariablePhiGradient = VariableShapeGradient<false>;
using VariableTestValue = VariableShapeValue<true>;
using VariableTestGradient = VariableShapeGradient<true>;

template <bool is_test>
class VectorVariableShapeValue
{
public:
  /**
   * Get the current vector shape function
   * @param datum The AssemblyDatum object of the current thread
   * @param i The element-local DOF index
   * @param qp The local quadrature point index
   * @returns The vector shape function
   */
  KOKKOS_FUNCTION Real3 operator()(AssemblyDatum & datum, unsigned int i, unsigned int qp) const
  {
    if (!is_test && datum.trialOverride())
      return datum.trialOverride()->vector_phi;

    auto & elem = datum.elem();
    auto side = datum.side();
    auto fe = is_test ? datum.ife() : datum.jfe();

    return side == libMesh::invalid_uint
               ? datum.assembly().getVectorPhi(elem.subdomain, elem.type, fe)(i, qp)
               : datum.assembly().getVectorPhiFace(elem.subdomain, elem.type, fe)(side)(i, qp);
  }
};

template <bool is_test>
class VectorVariableShapeGradient
{
public:
  /**
   * Get the gradient of the current vector shape function in reference space
   * @param datum The AssemblyDatum object of the current thread
   * @param i The element-local DOF index
   * @param qp The local quadrature point index
   * @returns The reference-space gradient of the vector shape function
   */
  KOKKOS_FUNCTION const Real33 &
  reference(AssemblyDatum & datum, unsigned int i, unsigned int qp) const;

  /**
   * Get the gradient of the current vector shape function
   * @param datum The AssemblyDatum object of the current thread
   * @param i The element-local DOF index
   * @param qp The local quadrature point index
   * @returns The gradient of the vector shape function
   */
  KOKKOS_FUNCTION Real33 operator()(AssemblyDatum & datum, unsigned int i, unsigned int qp) const
  {
    if (!is_test && datum.trialOverride())
      return datum.trialOverride()->vector_grad_phi;

    return reference(datum, i, qp) * datum.dxi_dx(qp).transpose();
  }
};

template <bool is_test>
KOKKOS_FUNCTION const Real33 &
VectorVariableShapeGradient<is_test>::reference(AssemblyDatum & datum,
                                                unsigned int i,
                                                unsigned int qp) const
{
  auto & elem = datum.elem();
  auto side = datum.side();
  auto fe = is_test ? datum.ife() : datum.jfe();

  return side == libMesh::invalid_uint
             ? datum.assembly().getVectorGradPhi(elem.subdomain, elem.type, fe)(i, qp)
             : datum.assembly().getVectorGradPhiFace(elem.subdomain, elem.type, fe)(side)(i, qp);
}

template <bool is_test>
class VectorVariableShapeCurl
{
public:
  /**
   * Get the curl of the current vector shape function
   * @param datum The AssemblyDatum object of the current thread
   * @param i The element-local DOF index
   * @param qp The local quadrature point index
   * @returns The curl of the vector shape function
   */
  KOKKOS_FUNCTION Real3 operator()(AssemblyDatum & datum, unsigned int i, unsigned int qp) const
  {
    if (!is_test && datum.trialOverride())
      return curlFromVectorGradient(datum.trialOverride()->vector_grad_phi,
                                    datum.assembly().getDimension());

    auto & elem = datum.elem();
    auto side = datum.side();
    auto fe = is_test ? datum.ife() : datum.jfe();

    auto grad =
        side == libMesh::invalid_uint
            ? datum.assembly().getVectorGradPhi(elem.subdomain, elem.type, fe)(i, qp)
            : datum.assembly().getVectorGradPhiFace(elem.subdomain, elem.type, fe)(side)(i, qp);

    return curlFromVectorGradient(grad * datum.dxi_dx(qp).transpose(),
                                  datum.assembly().getDimension());
  }
};

using VectorVariablePhiValue = VectorVariableShapeValue<false>;
using VectorVariablePhiGradient = VectorVariableShapeGradient<false>;
using VectorVariablePhiCurl = VectorVariableShapeCurl<false>;
using VectorVariableTestValue = VectorVariableShapeValue<true>;
using VectorVariableTestGradient = VectorVariableShapeGradient<true>;
using VectorVariableTestCurl = VectorVariableShapeCurl<true>;
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
  VariableValue(Variable var, bool dof = false) : _var(var), _dof(dof || _var.scalar())
  {
    checkVariable(_var, false, "VariableValue");
  }
  /**
   * Constructor
   * @param var The MOOSE variable
   * @param tag The vector tag name
   * @param dof Whether to get DOF values
   */
  VariableValue(const MooseVariableBase & var,
                const TagName & tag = Moose::SOLUTION_TAG,
                bool dof = false)
    : _var(var, tag), _dof(dof || _var.scalar())
  {
    checkVariable(_var, false, "VariableValue");
  }

  /**
   * Get whether the variable was coupled
   * @returns Whether the variable was coupled
   */
  KOKKOS_FUNCTION operator bool() const { return _var.coupled(); }

  /**
   * Get the current variable value
   * @param datum The Datum object of the current thread
   * @param idx The local quadrature point or DOF index
   * @param comp The variable component
   * @returns The variable value
   */
  KOKKOS_FUNCTION Real operator()(Datum & datum, unsigned int idx, unsigned int comp = 0) const;

  /**
   * Get the Kokkos variable
   * @returns The Kokkos variable
   */
  KOKKOS_FUNCTION const Variable & variable() const { return _var; }

  /**
   * Get whether DOF values are requested
   */
  KOKKOS_FUNCTION bool dof() const { return _dof; }

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

KOKKOS_FUNCTION inline Real
VariableValue::operator()(Datum & datum, unsigned int idx, unsigned int comp) const
{
  KOKKOS_ASSERT(_var.initialized());

  if (!_var.coupled())
    return _var.value(_var.scalar() ? idx : comp);

  auto & sys = datum.system(_var.sys(comp));
  auto var = _var.var(comp);
  auto tag = _var.tag();

  if (_dof)
  {
    unsigned int dof;

    if (_var.scalar())
      dof = sys.getScalarLocalDofIndex(idx, var);
    else if (datum.isNodal())
      dof = sys.getNodeLocalDofIndex(datum.node(), 0, var);
    else
      dof = sys.getElemLocalDofIndex(datum.elem().id, idx, var);

    return sys.getVectorDofValue(dof, tag);
  }

  auto & elem = datum.elem();
  auto side = datum.side();

  return side == libMesh::invalid_uint
             ? sys.getVectorQpValue(elem, datum.qpOffset() + idx, var, tag)
             : sys.getVectorQpValueFace(elem, side, idx, var, tag);
}

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
  VariableGradient(Variable var) : _var(var) { checkVariable(_var, false, "VariableGradient"); }
  /**
   * Constructor
   * @param var The MOOSE variable
   * @param tag The vector tag name
   */
  VariableGradient(const MooseVariableFieldBase & var, const TagName & tag = Moose::SOLUTION_TAG)
    : _var(var, tag)
  {
    checkVariable(_var, false, "VariableGradient");
  }

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
  KOKKOS_FUNCTION Real3 operator()(Datum & datum, unsigned int qp, unsigned int comp = 0) const;
  /**
   * Get the current variable gradient with respect to the reference position
   * @param datum The Datum object of the current thread
   * @param qp The local quadrature point index
   * @param comp The variable component
   * @returns The variable gradient on the reference configuration
   */
  KOKKOS_FUNCTION Real3 reference(Datum & datum, unsigned int qp, unsigned int comp = 0) const;

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

KOKKOS_FUNCTION inline Real3
VariableGradient::reference(Datum & datum, unsigned int qp, unsigned int comp) const
{
  KOKKOS_ASSERT(_var.initialized());

  if (!_var.coupled())
    return Real3(0);

  KOKKOS_ASSERT(!datum.isNodal());

  auto & elem = datum.elem();
  auto side = datum.side();
  auto & sys = datum.system(_var.sys(comp));

  return side == libMesh::invalid_uint
             ? sys.getVectorQpGrad(elem, datum.qpOffset() + qp, _var.var(comp), _var.tag())
             : sys.getVectorQpGradFace(
                   elem, side, datum.reference_dxi_dx(qp), qp, _var.var(comp), _var.tag());
}

KOKKOS_FUNCTION inline Real3
VariableGradient::operator()(Datum & datum, unsigned int qp, unsigned int comp) const
{
  KOKKOS_ASSERT(_var.initialized());

  if (!_var.coupled())
    return Real3(0);

  KOKKOS_ASSERT(!datum.isNodal());

  auto & elem = datum.elem();
  auto side = datum.side();
  auto & sys = datum.system(_var.sys(comp));

  // The cached element gradients are taken on the reference configuration, while the face
  // gradients use the map of the configuration
  return side == libMesh::invalid_uint
             ? datum.pushForward(
                   sys.getVectorQpGrad(elem, datum.qpOffset() + qp, _var.var(comp), _var.tag()), qp)
             : sys.getVectorQpGradFace(
                   elem, side, datum.dxi_dx(qp), qp, _var.var(comp), _var.tag());
}

class VectorVariableValue
{
public:
  /**
   * Default constructor
   */
  VectorVariableValue() = default;
  /**
   * Constructor
   * @param var The Kokkos variable
   * @param dof Whether to get DOF values
   */
  VectorVariableValue(Variable var, bool dof = false) : _var(var), _dof(dof)
  {
    checkVariable(_var, true, "VectorVariableValue");
  }
  /**
   * Constructor
   * @param var The MOOSE variable
   * @param tag The vector tag name
   * @param dof Whether to get DOF values
   */
  VectorVariableValue(const MooseVariableFieldBase & var,
                      const TagName & tag = Moose::SOLUTION_TAG,
                      bool dof = false)
    : _var(var, tag), _dof(dof)
  {
    checkVariable(_var, true, "VectorVariableValue");
  }

  /**
   * Get whether the variable was coupled
   * @returns Whether the variable was coupled
   */
  KOKKOS_FUNCTION operator bool() const { return _var.coupled(); }

  /**
   * Get the current vector variable value
   * @param datum The AssemblyDatum object of the current thread
   * @param idx The local quadrature point index or DOF index
   * @returns The vector variable value
   */
  KOKKOS_FUNCTION Real3 operator()(AssemblyDatum & datum,
                                   unsigned int idx,
                                   unsigned int comp = 0) const;

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

class VectorVariableGradient
{
public:
  /**
   * Default constructor
   */
  VectorVariableGradient() = default;
  /**
   * Constructor
   * @param var The Kokkos variable
   */
  VectorVariableGradient(Variable var) : _var(var)
  {
    checkVariable(_var, true, "VectorVariableGradient");
  }
  /**
   * Constructor
   * @param var The MOOSE variable
   * @param tag The vector tag name
   */
  VectorVariableGradient(const MooseVariableFieldBase & var,
                         const TagName & tag = Moose::SOLUTION_TAG)
    : _var(var, tag)
  {
    checkVariable(_var, true, "VectorVariableGradient");
  }

  /**
   * Get whether the variable was coupled
   * @returns Whether the variable was coupled
   */
  KOKKOS_FUNCTION operator bool() const { return _var.coupled(); }

  /**
   * Get the current vector variable gradient
   * @param datum The AssemblyDatum object of the current thread
   * @param qp The local quadrature point index
   * @returns The vector variable gradient
   */
  KOKKOS_FUNCTION Real33 operator()(AssemblyDatum & datum,
                                    unsigned int qp,
                                    unsigned int comp = 0) const;

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

class VectorVariableCurl
{
public:
  /**
   * Default constructor
   */
  VectorVariableCurl() = default;
  /**
   * Constructor
   * @param var The Kokkos variable
   */
  VectorVariableCurl(Variable var) : _var(var) { checkVariable(_var, true, "VectorVariableCurl"); }
  /**
   * Constructor
   * @param var The MOOSE variable
   * @param tag The vector tag name
   */
  VectorVariableCurl(const MooseVariableFieldBase & var, const TagName & tag = Moose::SOLUTION_TAG)
    : _var(var, tag)
  {
    checkVariable(_var, true, "VectorVariableCurl");
  }

  /**
   * Get whether the variable was coupled
   * @returns Whether the variable was coupled
   */
  KOKKOS_FUNCTION operator bool() const { return _var.coupled(); }

  /**
   * Get the current vector variable curl
   * @param datum The AssemblyDatum object of the current thread
   * @param qp The local quadrature point index
   * @returns The vector variable curl
   */
  KOKKOS_FUNCTION Real3 operator()(AssemblyDatum & datum,
                                   unsigned int qp,
                                   unsigned int comp = 0) const;

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

KOKKOS_FUNCTION inline Real3
VectorVariableValue::operator()(AssemblyDatum & datum, unsigned int idx, unsigned int comp) const
{
  KOKKOS_ASSERT(_var.initialized());

  Real3 value = 0;

  if (_var.coupled())
  {
    auto & sys = datum.system(_var.sys(comp));
    auto var = _var.var(comp);
    auto tag = _var.tag();

    if (_dof)
    {
      auto dimension = datum.assembly().getDimension();

      if (datum.isNodal())
      {
        auto node = datum.node();

        for (unsigned int c = 0; c < dimension; ++c)
          value(c) = sys.getVectorDofValue(sys.getNodeLocalDofIndex(node, c, var), tag);
      }
      else
      {
        auto elem = datum.elem().id;
        auto offset = idx * dimension;

        for (unsigned int c = 0; c < dimension; ++c)
          value(c) = sys.getVectorDofValue(sys.getElemLocalDofIndex(elem, offset + c, var), tag);
      }
    }
    else
    {
      KOKKOS_ASSERT(!datum.isNodal());

      auto & elem = datum.elem();
      auto side = datum.side();

      if (side == libMesh::invalid_uint)
        value = sys.getVectorQpVectorValue(elem, datum.qpOffset() + idx, var, tag);
      else
        value = sys.getVectorQpVectorValueFace(elem, side, idx, var, tag);
    }
  }
  else
    value = _var.vectorValue(comp);

  return value;
}

KOKKOS_FUNCTION inline Real33
VectorVariableGradient::operator()(AssemblyDatum & datum, unsigned int qp, unsigned int comp) const
{
  KOKKOS_ASSERT(_var.initialized());

  Real33 grad = 0;

  if (_var.coupled())
  {
    KOKKOS_ASSERT(!datum.isNodal());

    auto & elem = datum.elem();
    auto side = datum.side();
    auto & sys = datum.system(_var.sys(comp));
    auto var = _var.var(comp);
    auto tag = _var.tag();

    if (side == libMesh::invalid_uint)
      grad =
          datum.pushForward(sys.getVectorQpVectorGrad(elem, datum.qpOffset() + qp, var, tag), qp);
    else
      grad = sys.getVectorQpVectorGradFace(elem, side, datum.dxi_dx(qp), qp, var, tag);
  }

  return grad;
}

KOKKOS_FUNCTION inline Real3
VectorVariableCurl::operator()(AssemblyDatum & datum, unsigned int qp, unsigned int comp) const
{
  KOKKOS_ASSERT(_var.initialized());

  Real3 curl = 0;

  if (_var.coupled())
  {
    KOKKOS_ASSERT(!datum.isNodal());

    auto & elem = datum.elem();
    auto side = datum.side();
    auto & sys = datum.system(_var.sys(comp));
    auto var = _var.var(comp);
    auto tag = _var.tag();

    // The cached element curls are taken on the reference configuration, so the curl on the
    // current configuration is formed from the shape function gradients
    if (side == libMesh::invalid_uint && !datum.configuration().displaced())
      curl = sys.getVectorQpVectorCurl(elem, datum.qpOffset() + qp, var, tag);
    else
    {
      auto fe = sys.getFETypeID(var);
      auto n_dofs = datum.assembly().getNumDofs(elem.type, fe);
      auto jacobian = datum.dxi_dx(qp);
      auto jacobian_transpose = jacobian.transpose();
      Real33 grad = 0;

      for (unsigned int i = 0; i < n_dofs; ++i)
      {
        const auto & grad_phi =
            side == libMesh::invalid_uint
                ? datum.assembly().getVectorGradPhi(elem.subdomain, elem.type, fe)(i, qp)
                : datum.assembly().getVectorGradPhiFace(elem.subdomain, elem.type, fe)(side)(i, qp);

        grad += sys.getVectorDofValue(sys.getElemLocalDofIndex(elem.id, i, var), tag) *
                (grad_phi * jacobian_transpose);
      }

      curl = curlFromVectorGradient(grad, datum.assembly().getDimension());
    }
  }

  return curl;
}

///@}

} // namespace Moose::Kokkos
