//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMQuadratureFunctionCoefficient.h"
#include "libmesh/int_range.h"

MFEMQuadratureFunctionCoefficient::MFEMQuadratureFunctionCoefficient(mfem::Coefficient & source,
                                                                     mfem::QuadratureFunction & qf,
                                                                     bool constant)
  : mfem::QuadratureFunctionCoefficient(qf), _source(source), _qf(qf), _constant(constant),
    _dirty(true)
{
}

void
MFEMQuadratureFunctionCoefficient::SetTime(mfem::real_t t)
{
  mfem::Coefficient::SetTime(t);
  if (!_constant)
    _dirty = true;
}

mfem::real_t
MFEMQuadratureFunctionCoefficient::Eval(mfem::ElementTransformation & T,
                                        const mfem::IntegrationPoint & ip)
{
  if (_dirty)
    refresh();
  return mfem::QuadratureFunctionCoefficient::Eval(T, ip);
}

void
MFEMQuadratureFunctionCoefficient::Project(mfem::QuadratureFunction & qf)
{
  if (_dirty)
    refresh();
  mfem::QuadratureFunctionCoefficient::Project(qf);
}

void
MFEMQuadratureFunctionCoefficient::refresh()
{
  // Equivalent to _source.Project(_qf), except performed with a caller-owned element
  // transformation: the mesh-owned shared transformation used by mfem::Coefficient::Project
  // may belong to an in-flight assembly loop that is evaluating this coefficient, and
  // projecting through it would corrupt that loop's state.
  const mfem::QuadratureSpaceBase & qspace = *_qf.GetSpace();
  const mfem::Mesh & mesh = *qspace.GetMesh();
  mfem::IsoparametricTransformation T;
  mfem::Vector values;
  for (const auto iel : libMesh::make_range(qspace.GetNE()))
  {
    _qf.GetValues(iel, values);
    const mfem::IntegrationRule & ir = qspace.GetIntRule(iel);
    mesh.GetElementTransformation(iel, &T);
    for (const auto iq : libMesh::make_range(ir.Size()))
    {
      const mfem::IntegrationPoint & ip = ir[iq];
      T.SetIntPoint(&ip);
      values[iq] = _source.Eval(T, ip);
    }
  }
  _dirty = false;
}

#endif
