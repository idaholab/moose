//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMScalarParsedCoefficient.h"

MFEMScalarParsedCoefficient::MFEMScalarParsedCoefficient(
    Moose::MFEM::CoefficientManager & coefficients,
    const std::vector<MFEMScalarCoefficientName> & coef_names,
    bool use_xyzt,
    const FunctionParserUtils<false>::SymFunctionPtr & func)
  : _coefficients(coefficients), _coef_names(coef_names), _use_xyzt(use_xyzt), _func(func)
{
}

mfem::real_t
MFEMScalarParsedCoefficient::Eval(mfem::ElementTransformation & T,
                                  const mfem::IntegrationPoint & ip)
{
  std::vector<mfem::real_t> vals(_coef_names.size() + (_use_xyzt ? 4 : 0));

  for (unsigned i = 0; i < _coef_names.size(); i++)
    vals[i] = _coefficients.getScalarCoefficient(_coef_names[i]).Eval(T, ip);

  if (_use_xyzt)
  {
    mfem::Vector transip;
    T.Transform(ip, transip);

    for (int i = 0; i < transip.Size(); i++)
      vals[_coef_names.size() + i] = transip(i);

    for (int i = transip.Size(); i < 3; i++)
      vals[_coef_names.size() + i] = 0;

    vals[_coef_names.size() + 3] = GetTime();
  }

  return _func->Eval(vals.data());
}

#endif
