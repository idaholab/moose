//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMScalarParsedCoeff.h"

MFEMScalarParsedCoeff::MFEMScalarParsedCoeff(
    const Moose::MFEM::GridFunctions & gridfunctions,
    const std::vector<VariableName> & var_names,
    bool use_xyzt,
    const FunctionParserUtils<false>::SymFunctionPtr & func)
  : _gridfunctions(gridfunctions), _var_names(var_names), _use_xyzt(use_xyzt), _func(func)
{
}

mfem::real_t
MFEMScalarParsedCoeff::Eval(mfem::ElementTransformation & T, const mfem::IntegrationPoint & ip)
{
  std::vector<mfem::real_t> vals(_var_names.size() + (_use_xyzt ? 4 : 0));

  for (unsigned i = 0; i < _var_names.size(); i++)
    vals[i] = _gridfunctions.GetRef(_var_names[i]).GetValue(T, ip);

  if (_use_xyzt)
  {
    mfem::Vector transip(3);
    T.Transform(ip, transip);

    for (unsigned i = 0; i < 3; i++)
      inpVals[_inputs.size() + i] = transip(i);

    inpVals[_inputs.size() + 3] = GetTime();
  }

  return _func->Eval(inpVals.data());
}

#endif
