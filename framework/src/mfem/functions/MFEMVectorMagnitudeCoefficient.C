//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMVectorMagnitudeCoefficient.h"

MFEMVectorMagnitudeCoefficient::MFEMVectorMagnitudeCoefficient(mfem::VectorCoefficient & vec_coef)
  : mfem::Coefficient(), _vec_coef(&vec_coef)
{
}

void
MFEMVectorMagnitudeCoefficient::SetTime(mfem::real_t t)
{
  if (_vec_coef)
    _vec_coef->SetTime(t);
  this->mfem::Coefficient::SetTime(t);
}

mfem::real_t
MFEMVectorMagnitudeCoefficient::Eval(mfem::ElementTransformation & T,
                                     const mfem::IntegrationPoint & ip)
{
  _vec_coef->Eval(_vec, T, ip);
  return _vec.Norml2();
}

#endif
