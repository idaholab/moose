//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GPUDatum.h"

#include "MooseVariableBase.h"

namespace Moose
{
namespace Kokkos
{

class VariablePhiValue : public AssemblyHolder
{
public:
  VariablePhiValue(Assembly & assembly) : AssemblyHolder(assembly) {}

  KOKKOS_FUNCTION Real operator()(ResidualDatum & datum, unsigned int i, unsigned int qp) const
  {
    auto & elem = datum.elem();
    auto side = datum.side();
    auto fe = datum.jfe();

    return side == libMesh::invalid_uint
               ? kokkosAssembly().getPhi(elem.subdomain, elem.type, fe)(i, qp)
               : kokkosAssembly().getPhiFace(elem.subdomain, elem.type, fe)(i, qp, side);
  }
};

class VariablePhiGradient : public AssemblyHolder
{
public:
  VariablePhiGradient(Assembly & assembly) : AssemblyHolder(assembly) {}

  KOKKOS_FUNCTION Real3 operator()(ResidualDatum & datum, unsigned int i, unsigned int qp) const
  {
    auto & elem = datum.elem();
    auto side = datum.side();
    auto fe = datum.jfe();

    return datum.J(qp) *
           (side == libMesh::invalid_uint
                ? kokkosAssembly().getGradPhi(elem.subdomain, elem.type, fe)(i, qp)
                : kokkosAssembly().getGradPhiFace(elem.subdomain, elem.type, fe)(i, qp, side));
  }
};

class VariableTestValue : public AssemblyHolder
{
public:
  VariableTestValue(Assembly & assembly) : AssemblyHolder(assembly) {}

  KOKKOS_FUNCTION Real operator()(ResidualDatum & datum, unsigned int i, unsigned int qp) const
  {
    auto & elem = datum.elem();
    auto side = datum.side();
    auto fe = datum.ife();

    return side == libMesh::invalid_uint
               ? kokkosAssembly().getPhi(elem.subdomain, elem.type, fe)(i, qp)
               : kokkosAssembly().getPhiFace(elem.subdomain, elem.type, fe)(i, qp, side);
  }
};

class VariableTestGradient : public AssemblyHolder
{
public:
  VariableTestGradient(Assembly & assembly) : AssemblyHolder(assembly) {}

  KOKKOS_FUNCTION Real3 operator()(ResidualDatum & datum, unsigned int i, unsigned int qp) const
  {
    auto & elem = datum.elem();
    auto side = datum.side();
    auto fe = datum.ife();

    return datum.J(qp) *
           (side == libMesh::invalid_uint
                ? kokkosAssembly().getGradPhi(elem.subdomain, elem.type, fe)(i, qp)
                : kokkosAssembly().getGradPhiFace(elem.subdomain, elem.type, fe)(i, qp, side));
  }
};

class VariableValue : public SystemHolder
{
private:
  Variable _var;

public:
  VariableValue(Array<System> & systems, Variable var) : SystemHolder(systems), _var(var) {}
  VariableValue(Array<System> & systems,
                const MooseVariableBase & var,
                const TagName & tag = Moose::SOLUTION_TAG)
    : SystemHolder(systems), _var(var, tag)
  {
  }

  KOKKOS_FUNCTION operator bool() const { return _var.coupled(); }

  KOKKOS_FUNCTION Real operator()(Datum & datum, unsigned int qp, unsigned int comp = 0) const
  {
    if (_var.coupled())
    {
      auto & elem = datum.elem();
      auto side = datum.side();
      auto qp_offset = datum.qpOffset();

      return side == libMesh::invalid_uint
                 ? kokkosSystem(_var.sys(comp))
                       .getVectorQpValue(elem, qp_offset + qp, _var.var(comp), _var.tag())
                 : kokkosSystem(_var.sys(comp))
                       .getVectorQpValueFace(elem, side, qp, _var.var(comp), _var.tag());
    }
    else
      return _var.value(comp);
  }
};

class VariableNodalValue : public SystemHolder
{
private:
  Variable _var;

public:
  VariableNodalValue(Array<System> & systems, Variable var) : SystemHolder(systems), _var(var) {}
  VariableNodalValue(Array<System> & systems,
                     const MooseVariableBase & var,
                     const TagName & tag = Moose::SOLUTION_TAG)
    : SystemHolder(systems), _var(var, tag)
  {
  }

  KOKKOS_FUNCTION operator bool() const { return _var.coupled(); }

  KOKKOS_FUNCTION Real operator()(dof_id_type node, unsigned int comp = 0) const
  {
    if (_var.coupled())
    {
      auto dof = kokkosSystem(_var.sys(comp)).getNodeLocalDofIndex(node, _var.var(comp));

      return kokkosSystem(_var.sys(comp)).getVectorDofValue(dof, _var.tag());
    }
    else
      return _var.value(comp);
  }
};

class VariableGradient : public SystemHolder
{
private:
  Variable _var;

public:
  VariableGradient(Array<System> & systems, Variable var) : SystemHolder(systems), _var(var) {}
  VariableGradient(Array<System> & systems,
                   const MooseVariableBase & var,
                   const TagName & tag = Moose::SOLUTION_TAG)
    : SystemHolder(systems), _var(var, tag)
  {
  }

  KOKKOS_FUNCTION operator bool() const { return _var.coupled(); }

  KOKKOS_FUNCTION Real3 operator()(Datum & datum, unsigned int qp, unsigned int comp = 0) const
  {
    if (_var.coupled())
    {
      auto & elem = datum.elem();
      auto side = datum.side();
      auto qp_offset = datum.qpOffset();

      return side == libMesh::invalid_uint
                 ? kokkosSystem(_var.sys(comp))
                       .getVectorQpGrad(elem, qp_offset + qp, _var.var(comp), _var.tag())
                 : kokkosSystem(_var.sys(comp))
                       .getVectorQpGradFace(
                           elem, side, datum.J(qp), qp, _var.var(comp), _var.tag());
    }
    else
      return Real3(0);
  }
};

} // namespace Kokkos
} // namespace Moose
