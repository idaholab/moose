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
    mooseError("Kokkos",
               wrapper_name,
               " cannot be constructed with ",
               var.vector() ? "vector" : "scalar",
               " variables.");
}

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

class VectorVariablePhiValue
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
    auto & elem = datum.elem();
    auto side = datum.side();
    auto fe = datum.jfe();

    return side == libMesh::invalid_uint
               ? datum.assembly().getVectorPhi(elem.subdomain, elem.type, fe)(i, qp)
               : datum.assembly().getVectorPhiFace(elem.subdomain, elem.type, fe)(side)(i, qp);
  }
};

class VectorVariablePhiGradient
{
public:
  /**
   * Get the gradient of the current vector shape function
   * @param datum The AssemblyDatum object of the current thread
   * @param i The element-local DOF index
   * @param qp The local quadrature point index
   * @returns The gradient of the vector shape function
   */
  KOKKOS_FUNCTION Real33 operator()(AssemblyDatum & datum, unsigned int i, unsigned int qp) const
  {
    auto & elem = datum.elem();
    auto side = datum.side();
    auto fe = datum.jfe();

    auto grad =
        side == libMesh::invalid_uint
            ? datum.assembly().getVectorGradPhi(elem.subdomain, elem.type, fe)(i, qp)
            : datum.assembly().getVectorGradPhiFace(elem.subdomain, elem.type, fe)(side)(i, qp);

    return grad * datum.J(qp).transpose();
  }
};

class VectorVariablePhiCurl
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
    auto & elem = datum.elem();
    auto side = datum.side();
    auto fe = datum.jfe();

    auto grad =
        side == libMesh::invalid_uint
            ? datum.assembly().getVectorGradPhi(elem.subdomain, elem.type, fe)(i, qp)
            : datum.assembly().getVectorGradPhiFace(elem.subdomain, elem.type, fe)(side)(i, qp);

    return curlFromVectorGradient(grad * datum.J(qp).transpose(), datum.assembly().getDimension());
  }
};

class VectorVariableTestValue
{
public:
  /**
   * Get the current vector test function
   * @param datum The AssemblyDatum object of the current thread
   * @param i The element-local DOF index
   * @param qp The local quadrature point index
   * @returns The vector test function
   */
  KOKKOS_FUNCTION Real3 operator()(AssemblyDatum & datum, unsigned int i, unsigned int qp) const
  {
    auto & elem = datum.elem();
    auto side = datum.side();
    auto fe = datum.ife();

    return side == libMesh::invalid_uint
               ? datum.assembly().getVectorPhi(elem.subdomain, elem.type, fe)(i, qp)
               : datum.assembly().getVectorPhiFace(elem.subdomain, elem.type, fe)(side)(i, qp);
  }
};

class VectorVariableTestGradient
{
public:
  /**
   * Get the gradient of the current vector test function
   * @param datum The AssemblyDatum object of the current thread
   * @param i The element-local DOF index
   * @param qp The local quadrature point index
   * @returns The gradient of the vector test function
   */
  KOKKOS_FUNCTION Real33 operator()(AssemblyDatum & datum, unsigned int i, unsigned int qp) const
  {
    auto & elem = datum.elem();
    auto side = datum.side();
    auto fe = datum.ife();

    auto grad =
        side == libMesh::invalid_uint
            ? datum.assembly().getVectorGradPhi(elem.subdomain, elem.type, fe)(i, qp)
            : datum.assembly().getVectorGradPhiFace(elem.subdomain, elem.type, fe)(side)(i, qp);

    return grad * datum.J(qp).transpose();
  }
};

class VectorVariableTestCurl
{
public:
  /**
   * Get the curl of the current vector test function
   * @param datum The AssemblyDatum object of the current thread
   * @param i The element-local DOF index
   * @param qp The local quadrature point index
   * @returns The curl of the vector test function
   */
  KOKKOS_FUNCTION Real3 operator()(AssemblyDatum & datum, unsigned int i, unsigned int qp) const
  {
    auto & elem = datum.elem();
    auto side = datum.side();
    auto fe = datum.ife();

    auto grad =
        side == libMesh::invalid_uint
            ? datum.assembly().getVectorGradPhi(elem.subdomain, elem.type, fe)(i, qp)
            : datum.assembly().getVectorGradPhiFace(elem.subdomain, elem.type, fe)(side)(i, qp);

    return curlFromVectorGradient(grad * datum.J(qp).transpose(), datum.assembly().getDimension());
  }
};

///@}

/**
 * The Kokkos wrapper classes for MOOSE-like variable value access
 */
///@{
template <bool is_ad>
class VariableValueTempl
{
  using real_type = std::conditional_t<is_ad, ADReal, Real>;

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
  VariableValueTempl(Variable var, bool dof = false) : _var(var), _dof(dof)
  {
    checkVariable(_var, false, is_ad ? "ADVariableValue" : "VariableValue");
  }
  /**
   * Constructor
   * @param var The MOOSE variable
   * @param tag The vector tag name
   * @param dof Whether to get DOF values
   */
  VariableValueTempl(const MooseVariableFieldBase & var,
                     const TagName & tag = Moose::SOLUTION_TAG,
                     bool dof = false)
    : _var(var, tag), _dof(dof)
  {
    checkVariable(_var, false, is_ad ? "ADVariableValue" : "VariableValue");
  }
  /**
   * Constructor
   * @param vars The MOOSE variables
   * @param tag The vector tag name
   * @param dof Whether to get DOF values
   */
  ///@{
  VariableValueTempl(const std::vector<const MooseVariableFieldBase *> & vars,
                     const TagName & tag = Moose::SOLUTION_TAG,
                     bool dof = false)
    : _var(vars, tag), _dof(dof)
  {
    checkVariable(_var, false, is_ad ? "ADVariableValue" : "VariableValue");
  }
  VariableValueTempl(const std::vector<MooseVariableFieldBase *> & vars,
                     const TagName & tag = Moose::SOLUTION_TAG,
                     bool dof = false)
    : _var(vars, tag), _dof(dof)
  {
    checkVariable(_var, false, is_ad ? "ADVariableValue" : "VariableValue");
  }
  ///@}

  /**
   * Copy constructor for parallel dispatch
   */
  VariableValueTempl(const VariableValueTempl<is_ad> & object);
  /**
   * Copy assignment operator
   */
  VariableValueTempl<is_ad> & operator=(const VariableValueTempl<is_ad> & object);

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
  KOKKOS_FUNCTION auto operator()(Datum & datum, unsigned int idx, unsigned int comp = 0) const
  {
    return get(datum, idx, comp);
  }

  /**
   * Get the current variable value
   * @param datum The AssemblyDatum object of the current thread
   * @param idx The local quadrature point or DOF index
   * @param comp The variable component
   * @returns The variable value
   */
  KOKKOS_FUNCTION auto
  operator()(AssemblyDatum & datum, unsigned int idx, unsigned int comp = 0) const;

  /**
   * Get the Kokkos variable
   * @returns The Kokkos variable
   */
  KOKKOS_FUNCTION const Variable & variable() const { return _var; }

private:
  /**
   * Get the current variable value
   * @param datum The Datum object of the current thread
   * @param idx The local quadrature point or DOF index
   * @param comp The variable component
   * @param seed The derivative seed (only meaningful for AD)
   * @returns The variable value
   */
  KOKKOS_FUNCTION auto
  get(Datum & datum, unsigned int idx, unsigned int comp = 0, Real seed = 0) const;

  /**
   * Coupled Kokkos variable
   */
  Variable _var;
  /**
   * Derivative seed of each component for AD
   */
  Array<Real> _seed;
  /**
   * Flag whether DOF values are requested
   */
  bool _dof = false;
};

template <bool is_ad>
VariableValueTempl<is_ad>::VariableValueTempl(const VariableValueTempl<is_ad> & object)
  : _var(object._var), _seed(object._seed), _dof(object._dof)
{
  if constexpr (is_ad)
    if (_var.coupled())
    {
      if (!_seed.isAlloc())
        _seed.create(_var.components());

      for (unsigned int comp = 0; comp < _var.components(); ++comp)
        _seed[comp] =
            _var.dot() ? _var.mooseVar(comp)->sys().duDotDu(_var.var(comp)) : (_var.old() ? 0 : 1);

      _seed.copyToDevice();
    }
}

template <bool is_ad>
VariableValueTempl<is_ad> &
VariableValueTempl<is_ad>::operator=(const VariableValueTempl<is_ad> & object)
{
  _var = object._var;
  _dof = object._dof;

  return *this;
}

template <bool is_ad>
KOKKOS_FUNCTION auto
VariableValueTempl<is_ad>::operator()(AssemblyDatum & datum,
                                      unsigned int idx,
                                      unsigned int comp) const
{
  if constexpr (is_ad)
  {
    Real seed =
        datum.do_derivatives() && _var.coupled() && _var.sys(comp) == datum.sys() ? _seed[comp] : 0;

    return get(datum, idx, comp, seed);
  }
  else
    return get(datum, idx, comp);
}

template <bool is_ad>
KOKKOS_FUNCTION auto
VariableValueTempl<is_ad>::get(Datum & datum,
                               unsigned int idx,
                               unsigned int comp,
                               [[maybe_unused]] Real seed) const
{
  KOKKOS_ASSERT(_var.initialized());

  real_type value;

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

      if constexpr (is_ad)
        value = sys.getVectorDofADValue(dof, tag, seed);
      else
        value = sys.getVectorDofValue(dof, tag);
    }
    else
    {
      auto & elem = datum.elem();
      auto side = datum.side();

      if constexpr (is_ad)
        value = side == libMesh::invalid_uint
                    ? sys.getVectorQpADValue(elem, datum.qpOffset(), idx, var, tag, seed)
                    : sys.getVectorQpADValueFace(elem, side, idx, var, tag, seed);
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

template <bool is_ad>
class VariableGradientTempl
{
  using real3_type = std::conditional_t<is_ad, ADReal3, Real3>;

public:
  /**
   * Default constructor
   */
  VariableGradientTempl() = default;
  /**
   * Constructor
   * @param var The Kokkos variable
   */
  VariableGradientTempl(Variable var) : _var(var)
  {
    checkVariable(_var, false, is_ad ? "ADVariableGradient" : "VariableGradient");
  }
  /**
   * Constructor
   * @param var The MOOSE variable
   * @param tag The vector tag name
   */
  VariableGradientTempl(const MooseVariableFieldBase & var,
                        const TagName & tag = Moose::SOLUTION_TAG)
    : _var(var, tag)
  {
    checkVariable(_var, false, is_ad ? "ADVariableGradient" : "VariableGradient");
  }
  /**
   * Constructor
   * @param vars The MOOSE variables
   * @param tag The vector tag name
   */
  ///@{
  VariableGradientTempl(const std::vector<const MooseVariableFieldBase *> & vars,
                        const TagName & tag = Moose::SOLUTION_TAG)
    : _var(vars, tag)
  {
    checkVariable(_var, false, is_ad ? "ADVariableGradient" : "VariableGradient");
  }
  VariableGradientTempl(const std::vector<MooseVariableFieldBase *> & vars,
                        const TagName & tag = Moose::SOLUTION_TAG)
    : _var(vars, tag)
  {
    checkVariable(_var, false, is_ad ? "ADVariableGradient" : "VariableGradient");
  }
  ///@}

  /**
   * Copy constructor for parallel dispatch
   */
  VariableGradientTempl(const VariableGradientTempl<is_ad> & object);
  /**
   * Copy assignment operator
   */
  VariableGradientTempl<is_ad> & operator=(const VariableGradientTempl<is_ad> & object);

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
  KOKKOS_FUNCTION auto operator()(Datum & datum, unsigned int qp, unsigned int comp = 0) const
  {
    return get(datum, qp, comp);
  }

  /**
   * Get the current variable gradient
   * @param datum The AssemblyDatum object of the current thread
   * @param qp The local quadrature point index
   * @param comp The variable component
   * @returns The variable gradient
   */
  KOKKOS_FUNCTION auto
  operator()(AssemblyDatum & datum, unsigned int qp, unsigned int comp = 0) const;

  /**
   * Get the Kokkos variable
   * @returns The Kokkos variable
   */
  KOKKOS_FUNCTION const Variable & variable() const { return _var; }

private:
  /**
   * Get the current variable gradient
   * @param datum The Datum object of the current thread
   * @param qp The local quadrature point index
   * @param comp The variable component
   * @param seed The derivative seed (only meaningful for AD)
   * @returns The variable gradient
   */
  KOKKOS_FUNCTION auto
  get(Datum & datum, unsigned int qp, unsigned int comp = 0, Real seed = 0) const;

  /**
   * Coupled Kokkos variable
   */
  Variable _var;
  /**
   * Derivative seed of each component for AD
   */
  Array<Real> _seed;
};

template <bool is_ad>
VariableGradientTempl<is_ad>::VariableGradientTempl(const VariableGradientTempl<is_ad> & object)
  : _var(object._var), _seed(object._seed)
{
  if constexpr (is_ad)
    if (_var.coupled())
    {
      if (!_seed.isAlloc())
        _seed.create(_var.components());

      for (unsigned int comp = 0; comp < _var.components(); ++comp)
        _seed[comp] =
            _var.dot() ? _var.mooseVar(comp)->sys().duDotDu(_var.var(comp)) : (_var.old() ? 0 : 1);

      _seed.copyToDevice();
    }
}

template <bool is_ad>
VariableGradientTempl<is_ad> &
VariableGradientTempl<is_ad>::operator=(const VariableGradientTempl<is_ad> & object)
{
  _var = object._var;

  return *this;
}

template <bool is_ad>
KOKKOS_FUNCTION auto
VariableGradientTempl<is_ad>::operator()(AssemblyDatum & datum,
                                         unsigned int qp,
                                         unsigned int comp) const
{
  if constexpr (is_ad)
  {
    Real seed =
        datum.do_derivatives() && _var.coupled() && _var.sys(comp) == datum.sys() ? _seed[comp] : 0;

    return get(datum, qp, comp, seed);
  }
  else
    return get(datum, qp, comp);
}

template <bool is_ad>
KOKKOS_FUNCTION auto
VariableGradientTempl<is_ad>::get(Datum & datum,
                                  unsigned int qp,
                                  unsigned int comp,
                                  [[maybe_unused]] Real seed) const
{
  KOKKOS_ASSERT(_var.initialized());

  real3_type grad;

  if (_var.coupled())
  {
    KOKKOS_ASSERT(!datum.isNodal());

    auto & elem = datum.elem();
    auto side = datum.side();

    if constexpr (is_ad)
      grad =
          side == libMesh::invalid_uint
              ? datum.system(_var.sys(comp))
                    .getVectorQpADGrad(
                        elem, datum.J(qp), datum.qpOffset(), qp, _var.var(comp), _var.tag(), seed)
              : datum.system(_var.sys(comp))
                    .getVectorQpADGradFace(
                        elem, side, datum.J(qp), qp, _var.var(comp), _var.tag(), seed);
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

using VariableValue = VariableValueTempl<false>;
using ADVariableValue = VariableValueTempl<true>;
using VariableGradient = VariableGradientTempl<false>;
using ADVariableGradient = VariableGradientTempl<true>;

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
   * @param qp The local quadrature point index
   * @returns The vector variable value
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
VectorVariableValue::operator()(AssemblyDatum & datum, unsigned int qp, unsigned int comp) const
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
      KOKKOS_ASSERT(datum.isNodal());

      auto node = datum.node();
      auto dimension = datum.assembly().getDimension();

      for (unsigned int comp = 0; comp < dimension; ++comp)
        value(comp) = sys.getVectorDofValue(sys.getNodeLocalDofIndex(node, comp, var), tag);
    }
    else
    {
      KOKKOS_ASSERT(!datum.isNodal());

      auto & elem = datum.elem();
      auto side = datum.side();

      if (side == libMesh::invalid_uint)
        value = sys.getVectorQpVectorValue(elem, datum.qpOffset() + qp, var, tag);
      else
        value = sys.getVectorQpVectorValueFace(elem, side, qp, var, tag);
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
      grad = sys.getVectorQpVectorGrad(elem, datum.qpOffset() + qp, var, tag);
    else
      grad = sys.getVectorQpVectorGradFace(elem, side, datum.J(qp), qp, var, tag);
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

    if (side == libMesh::invalid_uint)
      curl = sys.getVectorQpVectorCurl(elem, datum.qpOffset() + qp, var, tag);
    else
    {
      auto fe = sys.getFETypeID(var);
      auto n_dofs = datum.assembly().getNumDofs(elem.type, fe);
      auto & grad_phi = datum.assembly().getVectorGradPhiFace(elem.subdomain, elem.type, fe)(side);
      auto jacobian = datum.J(qp);
      auto jacobian_transpose = jacobian.transpose();
      Real33 grad = 0;

      for (unsigned int i = 0; i < n_dofs; ++i)
        grad += sys.getVectorDofValue(sys.getElemLocalDofIndex(elem.id, i, var), tag) *
                (grad_phi(i, qp) * jacobian_transpose);

      curl = curlFromVectorGradient(grad, datum.assembly().getDimension());
    }
  }

  return curl;
}

template <>
struct ArrayDeepCopy<ADVariableValue>
{
  static constexpr bool value = true;
};

template <>
struct ArrayDeepCopy<ADVariableGradient>
{
  static constexpr bool value = true;
};
///@}

} // namespace Moose::Kokkos
