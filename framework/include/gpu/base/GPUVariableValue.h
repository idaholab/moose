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

class GPUVariablePhiValue : public GPUAssemblyHolder
{
public:
  GPUVariablePhiValue(GPUAssembly & assembly) : GPUAssemblyHolder(assembly) {}

  KOKKOS_FUNCTION Real operator()(ResidualDatum & datum, unsigned int i, unsigned int qp) const
  {
    auto & elem = datum.elem();
    auto side = datum.side();
    auto fe = datum.jfe();

    return side == libMesh::invalid_uint
               ? assembly().getPhi(elem.subdomain, elem.type, fe)(i, qp)
               : assembly().getPhiFace(elem.subdomain, elem.type, fe)(i, qp, side);
  }
};

class GPUVariablePhiGradient : public GPUAssemblyHolder
{
public:
  GPUVariablePhiGradient(GPUAssembly & assembly) : GPUAssemblyHolder(assembly) {}

  KOKKOS_FUNCTION Real3 operator()(ResidualDatum & datum, unsigned int i, unsigned int qp) const
  {
    auto & elem = datum.elem();
    auto side = datum.side();
    auto fe = datum.jfe();

    return datum.J(qp) *
           (side == libMesh::invalid_uint
                ? assembly().getGradPhi(elem.subdomain, elem.type, fe)(i, qp)
                : assembly().getGradPhiFace(elem.subdomain, elem.type, fe)(i, qp, side));
  }
};

class GPUVariableTestValue : public GPUAssemblyHolder
{
public:
  GPUVariableTestValue(GPUAssembly & assembly) : GPUAssemblyHolder(assembly) {}

  KOKKOS_FUNCTION Real operator()(ResidualDatum & datum, unsigned int i, unsigned int qp) const
  {
    auto & elem = datum.elem();
    auto side = datum.side();
    auto fe = datum.ife();

    return side == libMesh::invalid_uint
               ? assembly().getPhi(elem.subdomain, elem.type, fe)(i, qp)
               : assembly().getPhiFace(elem.subdomain, elem.type, fe)(i, qp, side);
  }
};

class GPUVariableTestGradient : public GPUAssemblyHolder
{
public:
  GPUVariableTestGradient(GPUAssembly & assembly) : GPUAssemblyHolder(assembly) {}

  KOKKOS_FUNCTION Real3 operator()(ResidualDatum & datum, unsigned int i, unsigned int qp) const
  {
    auto & elem = datum.elem();
    auto side = datum.side();
    auto fe = datum.ife();

    return datum.J(qp) *
           (side == libMesh::invalid_uint
                ? assembly().getGradPhi(elem.subdomain, elem.type, fe)(i, qp)
                : assembly().getGradPhiFace(elem.subdomain, elem.type, fe)(i, qp, side));
  }
};

class GPUVariableValue : public GPUSystemHolder
{
private:
  GPUVariable _var;

public:
  GPUVariableValue(GPUArray<GPUSystem> & systems, GPUVariable var)
    : GPUSystemHolder(systems), _var(var)
  {
  }
  GPUVariableValue(GPUArray<GPUSystem> & systems,
                   const MooseVariableBase & var,
                   const TagName & tag = Moose::SOLUTION_TAG)
    : GPUSystemHolder(systems), _var(var, tag)
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
                 ? system(_var.sys(comp))
                       .getVectorQpValue(elem, qp_offset + qp, _var.var(comp), _var.tag())
                 : system(_var.sys(comp))
                       .getVectorQpValueFace(elem, side, qp, _var.var(comp), _var.tag());
    }
    else
      return _var.value(comp);
  }
};

class GPUVariableNodalValue : public GPUSystemHolder
{
private:
  GPUVariable _var;

public:
  GPUVariableNodalValue(GPUArray<GPUSystem> & systems, GPUVariable var)
    : GPUSystemHolder(systems), _var(var)
  {
  }
  GPUVariableNodalValue(GPUArray<GPUSystem> & systems,
                        const MooseVariableBase & var,
                        const TagName & tag = Moose::SOLUTION_TAG)
    : GPUSystemHolder(systems), _var(var, tag)
  {
  }

  KOKKOS_FUNCTION operator bool() const { return _var.coupled(); }

  KOKKOS_FUNCTION Real operator()(dof_id_type node, unsigned int comp = 0) const
  {
    if (_var.coupled())
    {
      auto dof = system(_var.sys(comp)).getNodeLocalDofIndex(node, _var.var(comp));

      return system(_var.sys(comp)).getVectorDofValue(dof, _var.tag());
    }
    else
      return _var.value(comp);
  }
};

class GPUVariableGradient : public GPUSystemHolder
{
private:
  GPUVariable _var;

public:
  GPUVariableGradient(GPUArray<GPUSystem> & systems, GPUVariable var)
    : GPUSystemHolder(systems), _var(var)
  {
  }
  GPUVariableGradient(GPUArray<GPUSystem> & systems,
                      const MooseVariableBase & var,
                      const TagName & tag = Moose::SOLUTION_TAG)
    : GPUSystemHolder(systems), _var(var, tag)
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
                 ? system(_var.sys(comp))
                       .getVectorQpGrad(elem, qp_offset + qp, _var.var(comp), _var.tag())
                 : system(_var.sys(comp))
                       .getVectorQpGradFace(
                           elem, side, datum.J(qp), qp, _var.var(comp), _var.tag());
    }
    else
      return Real3(0);
  }
};
