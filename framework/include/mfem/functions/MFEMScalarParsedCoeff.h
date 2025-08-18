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

#include "MFEMContainers.h"
#include "FunctionParserUtils.h"
#include <vector>
#include <functional>

/**
 * Define a coefficient that, given a set of grid functions u, v, w, ...,
 * and a function func, returns func(x, y, z, t, u, v, w, ...)
 */
class MFEMScalarParsedCoeff : public mfem::Coefficient
{
private:
  const Moose::MFEM::GridFunctions & _gFuncs;
  const std::vector<std::string> & _inputs;
  bool _use_xyzt;
  const FunctionParserUtils<false>::SymFunctionPtr & _func;

public:
  MFEMScalarParsedCoeff(const Moose::MFEM::GridFunctions & gFuncs,
                        const std::vector<std::string> & inputs,
                        bool use_xyzt,
                        const FunctionParserUtils<false>::SymFunctionPtr & func);

  mfem::real_t Eval(mfem::ElementTransformation & T, const mfem::IntegrationPoint & ip) override;
};

#endif
