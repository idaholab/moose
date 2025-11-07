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

#include "MooseParsedFunction.h"
#include "FunctionParserUtils.h"
#include "MFEMProblem.h"

/**
 * Scalar, parsed function of position, time, and any number of scalar problem coefficients,
 * including any scalar variables, postprocessors, material properties or functions
 */
class MFEMParsedFunction : public MooseParsedFunction, public FunctionParserUtils<false>
{
public:
  static InputParameters validParams();

  MFEMParsedFunction(const InputParameters & parameters);
  virtual ~MFEMParsedFunction() = default;

  void initialSetup() override;

protected:
  /// reference to the MFEMProblem instance
  MFEMProblem & _mfem_problem;
  /// function parser object
  SymFunctionPtr _sym_function;
  /// coordinate and time variable names
  const std::vector<std::string> _xyzt;
  /// vector of references to the scalar coefficients used in the function
  std::vector<std::reference_wrapper<mfem::Coefficient>> _coefficients;
};

#endif
