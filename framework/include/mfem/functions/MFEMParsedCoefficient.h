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

/**
 * Define a coefficient that, given a set of grid functions u, v, w, ...,
 * and a function func, returns func(x, y, z, t, u, v, w, ...)
 */
class MFEMParsedCoefficient : public mfem::Coefficient
{
private:
  const Moose::MFEM::GridFunctions & _gridfunctions;
  const std::vector<VariableName> & _var_names;
  bool _use_xyzt;
  const FunctionParserUtils<false>::SymFunctionPtr & _func;

public:
  MFEMParsedCoefficient(const Moose::MFEM::GridFunctions & gridfunctions,
                              const std::vector<VariableName> & var_names,
                              bool use_xyzt,
                              const FunctionParserUtils<false>::SymFunctionPtr & func);

  mfem::real_t Eval(mfem::ElementTransformation & T, const mfem::IntegrationPoint & ip) override;
};

#endif
