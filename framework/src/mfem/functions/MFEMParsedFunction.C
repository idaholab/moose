#ifdef MOOSE_MFEM_ENABLED

#include "MFEMParsedFunction.h"
#include "MFEMScalarParsedCoeff.h"
#include "InputParameters.h"
#include "MFEMProblem.h"
#include <vector>
#include <memory>

registerMooseObject("MooseApp", MFEMParsedFunction);

InputParameters
MFEMParsedFunction::validParams()
{
  InputParameters params = MFEMGeneralUserObject::validParams();
  params += FunctionParserUtils<false>::validParams();
  params.registerBase("Function");
  params.addClassDescription("Parses function expression based on names and values "
                             "prescribed by input parameters.");
  params.addRequiredCustomTypeParam<std::string>(
      "function", "FunctionExpression", "Parsed function expression to compute");
  params.deprecateParam("function", "expression", "02/07/2024");
  params.addRequiredParam<std::vector<std::string>>("var_names",
                                                    "The names of the function variable names");
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
    _var_names(getParam<std::vector<std::string>>("var_names")),
    _use_xyzt(getParam<bool>("use_xyzt")),
    _xyzt({"x", "y", "z", "t"}),
    _problem_data(getMFEMProblem().getProblemData())
{

  // build variables argument
  std::string variables;

  // coupled field variables
  for (const auto i : index_range(_var_names))
    variables += (i == 0 ? "" : ",") + _var_names[i];

  // positions and time
  if (_use_xyzt)
    for (auto & v : _xyzt)
      variables += (variables.empty() ? "" : ",") + v;

  // base function object
  _func_F = std::make_shared<SymFunction>();

  // set FParser internal feature flags
  setParserFeatureFlags(_func_F);

  // parse function
  if (_func_F->Parse(_function, variables) >= 0)
    mooseError(
        "Invalid function\n", _function, "\nin ParsedAux ", name(), ".\n", _func_F->ErrorMsg());

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

  // declares MFEMScalarParsedCoeff
  getMFEMProblem().getCoefficients().declareScalar<MFEMScalarParsedCoeff>(
      name(), getMFEMProblem().getProblemData().gridfunctions, _var_names, _use_xyzt, _func_F);
}

MFEMParsedFunction::~MFEMParsedFunction() {}

#endif
