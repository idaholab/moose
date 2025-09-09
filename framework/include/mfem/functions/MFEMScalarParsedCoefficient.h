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
 * Define a coefficient that, given a set of (possibly gridfunction) coefficients
 * u, v, w, ..., and a function func, returns func(x, y, z, t, u, v, w, ...)
 */
class MFEMScalarParsedCoefficient : public mfem::Coefficient
{
private:
  Moose::MFEM::CoefficientManager & _coefficients;
  const std::vector<MFEMScalarCoefficientName> & _coef_names;
  bool _use_xyzt;
  const FunctionParserUtils<false>::SymFunctionPtr & _func;

public:
  MFEMScalarParsedCoefficient(Moose::MFEM::CoefficientManager & coefficients,
                              const std::vector<MFEMScalarCoefficientName> & coef_names,
                              bool use_xyzt,
                              const FunctionParserUtils<false>::SymFunctionPtr & func);

  mfem::real_t Eval(mfem::ElementTransformation & T, const mfem::IntegrationPoint & ip) override;
};

#endif
