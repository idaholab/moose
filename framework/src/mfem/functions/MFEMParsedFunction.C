//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMParsedFunction.h"
#include "MFEMParsedCoefficient.h"

registerMooseObject("MooseApp", MFEMParsedFunction);

InputParameters
MFEMParsedFunction::validParams()
{
  InputParameters params = MooseParsedFunction::validParams();
  params += FunctionParserUtils<false>::validParams();
  params.addClassDescription("Parses scalar function of position, time and scalar "
                             "problem coefficients (including scalar variables).");
  return params;
}

MFEMParsedFunction::MFEMParsedFunction(const InputParameters & parameters)
  : MooseParsedFunction(parameters),
    FunctionParserUtils(parameters),
    _mfem_problem(static_cast<MFEMProblem &>(_pfb_feproblem)),
    _sym_function(std::make_shared<SymFunction>()),
    _xyzt({"x", "y", "z", "t"})
{
  // variable symbols the function depends on (including position and time)
  std::string symbols = MooseUtils::stringJoin({_vars.begin(), _vars.end()}, ",");
  symbols += (symbols.empty() ? "" : ",") + MooseUtils::stringJoin(_xyzt, ",");

  // setup parsed function
  parsedFunctionSetup(_sym_function, _value, symbols, {}, {}, comm());

  // create MFEMParsedCoefficient
  _mfem_problem.getCoefficients().declareScalar<MFEMParsedCoefficient>(
      name(), _vars.size() + _xyzt.size(), _coefficients, _sym_function);
}

void
MFEMParsedFunction::initialSetup()
{
  for (const auto i : index_range(_vars))
    _coefficients.push_back(_mfem_problem.getCoefficients().getScalarCoefficient(_vals[i]));
}

#endif
