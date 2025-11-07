//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMParsedCoefficient.h"

MFEMParsedCoefficient::MFEMParsedCoefficient(
    const unsigned & arity,
    const std::vector<std::reference_wrapper<mfem::Coefficient>> & coefficients,
    const FunctionParserUtils<false>::SymFunctionPtr & sym_function)
  : _coefficients(coefficients), _sym_function(sym_function), _vals(arity), _transip(3)
{
}

mfem::real_t
MFEMParsedCoefficient::Eval(mfem::ElementTransformation & T, const mfem::IntegrationPoint & ip)
{
  for (unsigned i = 0; i < _coefficients.size(); i++)
    _vals[i] = _coefficients[i].get().Eval(T, ip);

  T.Transform(ip, _transip);

  for (int i = 0; i < 3; i++)
    _vals[_coefficients.size() + i] = i < _transip.Size() ? _transip(i) : 0.;

  _vals[_coefficients.size() + 3] = GetTime();

  return _sym_function->Eval(_vals.GetData());
}

#endif
