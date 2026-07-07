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
  _source.Project(_qf);
  _dirty = false;
}

#endif
