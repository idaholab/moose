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

#include "CoefficientManager.h"
#include "FunctionParserUtils.h"

/**
 * Scalar coefficient that, given a set of scalar (possibly, but not necessarily, gridfunction)
 * coefficients u, v, w, ..., and a scalar, parsed function f, returns f(u, v, w, ..., x, y, z, t)
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
