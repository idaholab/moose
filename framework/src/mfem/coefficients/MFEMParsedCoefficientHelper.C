#include "MFEMParsedCoefficientHelper.h"

InputParameters
MFEMParsedCoefficientHelper::validParams()
{
  InputParameters params = MFEMCoefficient::validParams();
  params += FunctionParserUtils<false>::validParams();
  params.addClassDescription(
      "Class to create mfem::Coefficients which evaluate to a provided parsed function.");
  return params;
}

MFEMParsedCoefficientHelper::MFEMParsedCoefficientHelper(const InputParameters & parameters,
                                                         VariableNameMappingMode map_mode)
  : MFEMCoefficient(parameters),
    hephaestus::CoupledCoefficient(hephaestus::InputParameters(
        std::map<std::string, std::any>({{"CoupledVariableName", std::string("dummy_variable")}}))),
    FunctionParserUtils<false>(parameters),
    _symbol_names(0),
    _coefficient_names(0)
{
}

void
MFEMParsedCoefficientHelper::functionParse(const std::string & function_expression)
{
  const std::vector<std::string> empty_string_vector;
  functionParse(function_expression, empty_string_vector, empty_string_vector);
}

void
MFEMParsedCoefficientHelper::functionParse(const std::string & function_expression,
                                           const std::vector<std::string> & constant_names,
                                           const std::vector<std::string> & constant_expressions)
{
  const std::vector<std::string> empty_string_vector;
  const std::vector<Real> empty_real_vector;
  functionParse(function_expression, constant_names, constant_expressions, empty_string_vector);
}

void
MFEMParsedCoefficientHelper::functionParse(const std::string & function_expression,
                                           const std::vector<std::string> & constant_names,
                                           const std::vector<std::string> & constant_expressions,
                                           const std::vector<std::string> & mfem_coefficient_names)
{
  const std::vector<std::string> empty_string_vector;
  functionParse(function_expression,
                constant_names,
                constant_expressions,
                mfem_coefficient_names,
                empty_string_vector);
}

void
MFEMParsedCoefficientHelper::functionParse(const std::string & function_expression,
                                           const std::vector<std::string> & constant_names,
                                           const std::vector<std::string> & constant_expressions,
                                           const std::vector<std::string> & mfem_coefficient_names,
                                           const std::vector<std::string> & mfem_gridfunction_names)
{
  // build base function object
  _func_F = std::make_shared<SymFunction>();

  // set FParser internal feature flags
  setParserFeatureFlags(_func_F);

  // initialize constants
  addFParserConstants(_func_F, constant_names, constant_expressions);

  // Store all MFEM gridfunction names
  unsigned int nmfem_gfs = mfem_gridfunction_names.size();
  _gridfunctions.resize(nmfem_gfs);
  for (const auto & gfname : mfem_gridfunction_names)
  {
    _gridfunction_names.push_back(gfname);
    _symbol_names.push_back(gfname);
  }

  // Store all MFEM coefficient names
  unsigned int nmfem_coefs = mfem_coefficient_names.size();
  _coefficients.resize(nmfem_coefs);
  for (const auto & coefname : mfem_coefficient_names)
  {
    _coefficient_names.push_back(coefname);
    _symbol_names.push_back(coefname);
  }

  // build 'variables' argument for fparser
  std::string variables = Moose::stringify(_symbol_names);

  // build the base function
  if (_func_F->Parse(function_expression, variables) >= 0)
    mooseError("Invalid function\n",
               function_expression,
               '\n',
               variables,
               "\nin MFEMParsedCoefficientHelper.\n",
               _func_F->ErrorMsg());

  // create parameter passing buffer
  _func_params.resize(nmfem_coefs + nmfem_gfs);

  // optimise function
  functionsOptimize();
}

void
MFEMParsedCoefficientHelper::functionsOptimize()
{
  // base function
  if (!_disable_fpoptimizer)
    _func_F->Optimize();
  if (_enable_jit && !_func_F->JITCompile())
    mooseInfo("Failed to JIT compile expression, falling back to byte code interpretation.");
}

void
MFEMParsedCoefficientHelper::Init(const hephaestus::GridFunctions & variables,
                                  hephaestus::Coefficients & coefficients)
{
  auto nmfem_gfs = _gridfunction_names.size();
  for (MooseIndex(_gridfunction_names) i = 0; i < nmfem_gfs; ++i)
  {
    if (variables.Has(_gridfunction_names[i]))
    {
      _gridfunctions[i] = variables.Get(_gridfunction_names[i]);
    }
    else
    {
      mooseError("Invalid gridfunction\n",
                 _gridfunction_names[i],
                 "\n",
                 "not found in variables when\n"
                 "creating MFEMParsedCoefficient\n",
                 "in MFEMParsedCoefficientHelper.\n");
    }
  }

  auto nmfem_coefs = _coefficient_names.size();
  for (MooseIndex(_coefficient_names) i = 0; i < nmfem_coefs; ++i)
  {
    _coefficients[i] = coefficients._scalars.Get(_coefficient_names[i]);
  }
}

double
MFEMParsedCoefficientHelper::Eval(mfem::ElementTransformation & trans,
                                  const mfem::IntegrationPoint & ip)
{
  // insert coefficient values
  auto nmfem_coefs = _coefficient_names.size();
  for (MooseIndex(_coefficient_names) i = 0; i < nmfem_coefs; ++i)
    _func_params[i] = _coefficients[i]->Eval(trans, ip);

  // insert gridfunction values
  auto nmfem_gfs = _gridfunction_names.size();
  for (MooseIndex(_gridfunction_names) i = 0; i < nmfem_gfs; ++i)
    _func_params[i + nmfem_coefs] = _gridfunctions[i]->GetValue(trans, ip);

  // set function value
  return evaluate(_func_F, _name);
}
