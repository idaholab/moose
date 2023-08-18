//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ParsedFunctorMaterial.h"

registerMooseObject("MooseApp", ParsedFunctorMaterial);
registerMooseObject("MooseApp", ADParsedFunctorMaterial);

template <bool is_ad>
InputParameters
ParsedFunctorMaterialTempl<is_ad>::validParams()
{
  InputParameters params = FunctorMaterial::validParams();
  params += FunctionParserUtils<is_ad>::validParams();

  params.addClassDescription(
      "Computes a functor material from a parsed expression of other functors.");

  params.addRequiredCustomTypeParam<std::string>(
      "expression", "FunctionExpression", "Expression to parse for the new functor material");
  params.addParam<std::vector<std::string>>("functor_names",
                                            "Functors to use in the parsed expression");
  params.addParam<std::vector<std::string>>(
      "functor_symbols",
      "Symbolic name to use for each functor in 'functor_names' in the parsed expression. If not "
      "provided, then the actual functor names must be used in the parsed expression.");
  params.addRequiredParam<std::string>("property_name",
                                       "Name to give the new functor material property");

  return params;
}

template <bool is_ad>
ParsedFunctorMaterialTempl<is_ad>::ParsedFunctorMaterialTempl(const InputParameters & parameters)
  : FunctorMaterial(parameters),
    FunctionParserUtils<is_ad>(parameters),
    _expression(getParam<std::string>("expression")),
    _functor_names(getParam<std::vector<std::string>>("functor_names")),
    _n_functors(_functor_names.size()),
    _functor_symbols(getParam<std::vector<std::string>>("functor_symbols")),
    _property_name(getParam<std::string>("property_name"))
{
  // Check/modify 'functor_symbols'
  if (_functor_symbols.size() != _n_functors)
  {
    if (_functor_symbols.size() == 0)
      _functor_symbols = _functor_names;
    else
      paramError("functor_symbols",
                 "The number of entries must be equal to either zero or the number of entries in "
                 "'functor_names'.");
  }

  // Get the functors
  _functors.resize(_n_functors);
  for (const auto i : index_range(_functor_names))
    _functors[i] = &getFunctor<GenericReal<is_ad>>(_functor_names[i]);

  // Build the parsed function
  buildParsedFunction();

  // Add the functor material property
  addFunctorProperty<GenericReal<is_ad>>(
      _property_name,
      [this](const auto & r, const auto & t) -> GenericReal<is_ad>
      {
        // Store the functor values
        for (const auto i : index_range(_functors))
          _func_params[i] = (*_functors[i])(r, t);

        // Store the space and time values
        // TODO: find some way to extract x,y,z from r. getPoint(r) does not exist yet.
        // const auto r_point = getPoint(r);
        _func_params[_n_functors] = 0;
#if LIBMESH_DIM > 1
        _func_params[_n_functors + 1] = 0;
#endif
#if LIBMESH_DIM > 2
        _func_params[_n_functors + 2] = 0;
#endif
        _func_params[_n_functors + LIBMESH_DIM] = getTime(t);

        // Evaluate the parsed function
        return evaluate(_parsed_function, _name);
      });
}

template <bool is_ad>
void
ParsedFunctorMaterialTempl<is_ad>::buildParsedFunction()
{
  _parsed_function = std::make_shared<SymFunction>();

  setParserFeatureFlags(_parsed_function);

  // Add constants
  _parsed_function->AddConstant("pi", std::acos(Real(-1)));
  _parsed_function->AddConstant("e", std::exp(Real(1)));

  // Collect the symbols corresponding to the _func_params values
  std::vector<std::string> symbols(_functor_symbols);
  std::string symbols_str = Moose::stringify(symbols);
  symbols_str += ",x";
#if LIBMESH_DIM > 1
  symbols_str += ",y";
#endif
#if LIBMESH_DIM > 2
  symbols_str += ",z";
#endif
  symbols_str += ",t";

  // Parse the expression
  if (_parsed_function->Parse(_expression, symbols_str) >= 0)
    mooseError("The expression\n'",
               _expression,
               "'\nwith symbols\n'",
               symbols_str,
               "'\ncould not be parsed:\n",
               _parsed_function->ErrorMsg());

  // Resize the values vector
  _func_params.resize(_n_functors + LIBMESH_DIM + 1);

  // TODO: Do the optimize stuff
}

// TODO: Make this some common utility function: duplicated from Function::getTime
template <bool is_ad>
Real
ParsedFunctorMaterialTempl<is_ad>::getTime(const Moose::StateArg & state) const
{
  if (state.iteration_type != Moose::SolutionIterationType::Time)
    // If we are any iteration type other than time (e.g. nonlinear), then temporally we are still
    // in the present time
    return miProblem().time();

  switch (state.state)
  {
    case 0:
      return miProblem().time();

    case 1:
      return miProblem().timeOld();

    default:
      mooseError("Unhandled state ", state.state, " in ParsedFunctorMaterial::getTime");
  }
}

template class ParsedFunctorMaterialTempl<false>;
template class ParsedFunctorMaterialTempl<true>;
