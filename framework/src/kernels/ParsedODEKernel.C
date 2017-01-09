/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "ParsedODEKernel.h"
#include "libmesh/fparser_ad.hh"

template<>
InputParameters validParams<ParsedODEKernel>()
{
  InputParameters params = validParams<ODEKernel>();
  params += validParams<FunctionParserUtils>();
  params.addClassDescription("Parsed ODE function kernel.");

  params.addRequiredParam<std::string>("function", "function expression");
  params.addCoupledVar("args", "additional coupled variables");
  params.addParam<std::vector<std::string> >("constant_names", "Vector of constants used in the parsed function (use this for kB etc.)");
  params.addParam<std::vector<std::string> >("constant_expressions", "Vector of values for the constants in constant_names (can be an FParser expression)");

  return params;
}

ParsedODEKernel::ParsedODEKernel(const InputParameters & parameters) :
    ODEKernel(parameters),
    FunctionParserUtils(parameters),
    _function(getParam<std::string>("function")),
    _nargs(coupledScalarComponents("args")),
    _args(_nargs),
    _arg_names(_nargs),
    _func_dFdarg(_nargs),
    _number_of_nl_variables(_sys.nVariables()),
    _arg_index(_number_of_nl_variables, -1)
{
  // build variables argument (start with variable the kernel is operating on)
  std::string variables = _var.name();

  // add additional coupled variables
  for (unsigned int i = 0; i < _nargs; ++i)
  {
    _arg_names[i] = getScalarVar("args", i)->name();
    variables += "," + _arg_names[i];
    _args[i] = &coupledScalarValue("args", i);

    // populate number -> arg index lookup table skipping aux variables
    unsigned int number = coupledScalar("args", i);
    if (number < _number_of_nl_variables)
      _arg_index[number] = i;
  }

  // base function object
  _func_F = ADFunctionPtr(new ADFunction());

  // set FParser interneal feature flags
  setParserFeatureFlags(_func_F);

  // add the constant expressions
  addFParserConstants(_func_F,
                      getParam<std::vector<std::string> >("constant_names"),
                      getParam<std::vector<std::string> >("constant_expressions"));

  // parse function
  if (_func_F->Parse(_function, variables) >= 0)
    mooseError("Invalid function\n" << _function << "\nin ParsedODEKernel " << name() << ".\n" << _func_F->ErrorMsg());

  // on-diagonal derivative
  _func_dFdu = ADFunctionPtr(new ADFunction(*_func_F));

  if (_func_dFdu->AutoDiff(_var.name()) != -1)
    mooseError("Failed to take first derivative w.r.t. " << _var.name());

  // off-diagonal derivatives
  for (unsigned int i = 0; i < _nargs; ++i)
  {
    _func_dFdarg[i] = ADFunctionPtr(new ADFunction(*_func_F));

    if (_func_dFdarg[i]->AutoDiff(_arg_names[i]) != -1)
      mooseError("Failed to take first derivative w.r.t. " << _arg_names[i]);
  }

  // optimize
  if (!_disable_fpoptimizer)
  {
    _func_F->Optimize();
    _func_dFdu->Optimize();
    for (unsigned int i = 0; i < _nargs; ++i)
      _func_dFdarg[i]->Optimize();
  }

  // just-in-time compile
  if (_enable_jit)
  {
    _func_F->JITCompile();
    _func_dFdu->JITCompile();
    for (unsigned int i = 0; i < _nargs; ++i)
      _func_dFdarg[i]->JITCompile();
  }

  // reserve storage for parameter passing buffer
  _func_params.resize(_nargs + 1);
}

void
ParsedODEKernel::updateParams()
{
  _func_params[0] = _u[_i];

  for (unsigned int j = 0; j < _nargs; ++j)
    _func_params[j + 1] = (*_args[j])[_i];
}

Real
ParsedODEKernel::computeQpResidual()
{
  updateParams();
  return evaluate(_func_F);
}

Real
ParsedODEKernel::computeQpJacobian()
{
  updateParams();
  return evaluate(_func_dFdu);
}

Real
ParsedODEKernel::computeQpOffDiagJacobian(unsigned int jvar)
{
  int i = _arg_index[jvar];
  if (i < 0)
    return 0.0;

  updateParams();
  return evaluate(_func_dFdarg[i]);
}
