//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#pragma once

#include "FunctionParserUtils.h"

/**
 * Define a coefficient that, given a set of (possibly gridfunction) coefficients
 * u, v, w, ..., and a function func, returns func(x, y, z, t, u, v, w, ...)
 */
class MFEMParsedCoefficient : public mfem::Coefficient
{
private:
  const std::vector<std::reference_wrapper<mfem::Coefficient>> & _coefficients;
  const FunctionParserUtils<false>::SymFunctionPtr & _sym_function;
  mfem::Array<mfem::real_t> _vals;
  mfem::Vector _transip;

public:
  MFEMParsedCoefficient(const unsigned & arity,
                        const std::vector<std::reference_wrapper<mfem::Coefficient>> & coefs,
                        const FunctionParserUtils<false>::SymFunctionPtr & sym_function);

  mfem::real_t Eval(mfem::ElementTransformation & T, const mfem::IntegrationPoint & ip) override;
};

#endif
