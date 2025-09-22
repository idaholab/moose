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
    const bool & use_xyzt,
    const FunctionParserUtils<false>::SymFunctionPtr & func)
  : _coefficients(coefficients),
    _coef_names(coef_names),
    _use_xyzt(use_xyzt),
    _func(func),
    _vals(coef_names.size() + (use_xyzt ? 4 : 0)),
    _transip(3)
{
}

mfem::real_t
MFEMScalarParsedCoefficient::Eval(mfem::ElementTransformation & T,
                                  const mfem::IntegrationPoint & ip)
{
  for (unsigned i = 0; i < _coef_names.size(); i++)
    _vals[i] = _coefficients.getScalarCoefficient(_coef_names[i]).Eval(T, ip);

  if (_use_xyzt)
  {
    T.Transform(ip, _transip);

    for (int i = 0; i < 3; i++)
      _vals[_coef_names.size() + i] = i < _transip.Size() ? _transip(i) : 0.;

    _vals[_coef_names.size() + 3] = GetTime();
  }

  return _func->Eval(_vals.GetData());
}

#endif
