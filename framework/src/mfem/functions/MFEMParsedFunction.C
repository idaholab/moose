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
#include "MFEMScalarParsedCoefficient.h"
#include "MFEMProblem.h"

registerMooseObject("MooseApp", MFEMParsedFunction);

InputParameters
MFEMParsedFunction::validParams()
{
  InputParameters params = MFEMGeneralUserObject::validParams();
  params += FunctionParserUtils<false>::validParams();
  params.registerBase("Function");
  params.addClassDescription("Parses function expression of position, time and problem variables.");
  params.addRequiredCustomTypeParam<std::string>(
      "expression", "FunctionExpression", "Parsed function expression to compute");
  params.addRequiredParam<std::vector<VariableName>>("var_names",
                                                     "The names of the function variables");
  params.addParam<bool>(
      "use_xyzt",
      false,
      "Make coordinate (x,y,z) and time (t) variables available in the function expression.");
  return params;
}

MFEMParsedFunction::MFEMParsedFunction(const InputParameters & parameters)
  : MFEMGeneralUserObject(parameters),
    FunctionParserUtils(parameters),
    _function(getParam<std::string>("expression")),
    _var_names(getParam<std::vector<VariableName>>("var_names")),
    _use_xyzt(getParam<bool>("use_xyzt")),
    _xyzt({"x", "y", "z", "t"})
{
  // coupled field variables
  std::string variables = MooseUtils::stringJoin({_var_names.begin(), _var_names.end()}, ",");

  // positions and time
  if (_use_xyzt)
    variables += (variables.empty() ? "" : ",") + MooseUtils::stringJoin(_xyzt, ",");

  // base function object
  _func_F = std::make_shared<SymFunction>();

  // set FParser internal feature flags
  setParserFeatureFlags(_func_F);

  // parse function
  if (_func_F->Parse(_function, variables) >= 0)
    mooseError("Invalid function\n", _function, "\nError:\n", _func_F->ErrorMsg());

  // optimize
  if (!_disable_fpoptimizer)
    _func_F->Optimize();

  // just-in-time compile
  if (_enable_jit)
  {
    // let rank 0 do the JIT compilation first
    if (_communicator.rank() != 0)
      _communicator.barrier();

    _func_F->JITCompile();

    // wait for ranks > 0 to catch up
    if (_communicator.rank() == 0)
      _communicator.barrier();
  }

  // declares MFEMScalarParsedCoefficient
  getMFEMProblem().getCoefficients().declareScalar<MFEMScalarParsedCoefficient>(
      name(), getMFEMProblem().getProblemData().gridfunctions, _var_names, _use_xyzt, _func_F);
}

MFEMParsedFunction::~MFEMParsedFunction() {}

#endif
