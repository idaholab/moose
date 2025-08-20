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
#include "MFEMGeneralUserObject.h"

/**
 * Declares parsed functions based on names and values prescribed by input parameters.
 */
class MFEMParsedFunction : public MFEMGeneralUserObject, public FunctionParserUtils<false>
{
public:
  static InputParameters validParams();

  MFEMParsedFunction(const InputParameters & parameters);
  virtual ~MFEMParsedFunction();

protected:
  /// function expression
  std::string _function;
  /// function variables
  const std::vector<VariableName> & _var_names;
  /// import coordinates and time
  const bool _use_xyzt;
  /// coordinate and time variable names
  const std::vector<std::string> _xyzt;
  /// function parser object for the resudual and on-diagonal Jacobian
  SymFunctionPtr _func_F;
};

#endif
