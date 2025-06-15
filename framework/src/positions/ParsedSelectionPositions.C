//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ParsedSelectionPositions.h"
#include "libmesh/parallel.h"
#include "libmesh/parallel_algebra.h"
#include "libmesh/vector_value.h"

registerMooseObject("MooseApp", ParsedSelectionPositions);

InputParameters
ParsedSelectionPositions::validParams()
{
  InputParameters params = Positions::validParams();
  params += NonADFunctorInterface::validParams();
  params += BlockRestrictable::validParams();
  params += FunctionParserUtils<false>::validParams();

  // Parsed expression parameters
  params.addRequiredCustomTypeParam<std::string>(
      "expression",
      "FunctionExpression",
      "Parsed function expression to compute the selection criterion. Note that x,y,z and t can be "
      "used in the expression without being declared as functors.");
  params.addParam<std::vector<MooseFunctorName>>(
      "functor_names", {}, "Functors to use in the parsed expression");
  params.addParam<std::vector<std::string>>(
      "functor_symbols",
      {},
      "Symbolic name to use for each functor in 'functor_names' in the parsed expression. If not "
      "provided, then the actual functor names will be used in the parsed expression.");

  // Use the default behaviors of a Positions object
  // Sort positions as usual
  params.set<bool>("auto_sort") = true;
  // Need to synchronize positions across ranks
  params.set<bool>("auto_broadcast") = true;

  // Keep as up-to-date as possible given the generality of functors
  params.set<ExecFlagEnum>("execute_on") = {EXEC_LINEAR, EXEC_TIMESTEP_BEGIN};

  params.addClassDescription("Examines elements in the geometry to find all positions matching a "
                             "user-specifed parsed expression criterion.");
  return params;
}

ParsedSelectionPositions::ParsedSelectionPositions(const InputParameters & parameters)
  : Positions(parameters),
    NonADFunctorInterface(this),
    BlockRestrictable(this),
    FunctionParserUtils(parameters),
    _expression(getParam<std::string>("expression")),
    _xyzt({"x", "y", "z", "t"}),
    _functor_names(getParam<std::vector<MooseFunctorName>>("functor_names")),
    _n_functors(_functor_names.size()),
    _functor_symbols(getParam<std::vector<std::string>>("functor_symbols"))
{
  if (getParam<ExecFlagEnum>("execute_on").contains(EXEC_NONE))
    paramError("execute_on",
               "NONE execution flag not supported. Most functors (functions, variables, spatial "
               "user objects for example) are not initialized at construction.");

  // sanity checks
  if (!_functor_symbols.empty() && (_functor_symbols.size() != _n_functors))
    paramError("functor_symbols", "functor_symbols must be the same length as functor_names.");

  // Make sure functors are not x, y, z, or t
  for (const auto & name : _functor_symbols)
    if (std::find(_xyzt.begin(), _xyzt.end(), name) != _xyzt.end())
      paramError("functor_symbols", "x, y, z, and t cannot be used in 'functor_symbols'.");
  for (const auto & name : _functor_names)
    if (std::find(_xyzt.begin(), _xyzt.end(), name) != _xyzt.end())
      paramError(
          "functor_names",
          "x, y, z, and t cannot be used in 'functor_names'. Use functor_symbols to disambiguate");

  // build variables argument
  std::string variables;

  // adding functors to the expression
  if (_functor_symbols.size())
    for (const auto & symbol : _functor_symbols)
      variables += (variables.empty() ? "" : ",") + symbol;
  else
    for (const auto & name : _functor_names)
      variables += (variables.empty() ? "" : ",") + name;

  // xyz and t are likely useful here
  for (auto & v : _xyzt)
    variables += (variables.empty() ? "" : ",") + v;

  // base function object
  _func_F = std::make_shared<SymFunction>();

  // set FParser internal feature flags
  setParserFeatureFlags(_func_F);

  // parse function
  if (_func_F->Parse(_expression, variables) >= 0)
    mooseError("Invalid function\n", _expression, "\nError:\n", _func_F->ErrorMsg());

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

  // reserve storage for parameter passing buffer
  _func_params.resize(_n_functors + 4);

  // keep pointers to the functors
  for (const auto & name : _functor_names)
    _functors.push_back(&getFunctor<Real>(name));
}

void
ParsedSelectionPositions::initialize()
{
  clearPositions();
  const auto n_points = _fe_problem.mesh().nElem();
  _positions.reserve(n_points);

  const auto state = determineState();

  // Loop over the local mesh, keep track of all extrema
  for (const auto & elem :
       _fe_problem.mesh().getMesh().active_local_subdomain_set_element_ptr_range(blockIDs()))
  {
    const Moose::ElemArg elem_arg = {elem, false};

    // Fill arguments to the parsed expression
    // Functors
    for (const auto i : index_range(_functors))
      _func_params[i] = (*_functors[i])(elem_arg, state);

    // Positions and time
    const auto & centroid = elem->true_centroid();
    for (const auto j : make_range(LIBMESH_DIM))
      _func_params[_n_functors + j] = centroid(j);
    _func_params[_n_functors + 3] = _t;

    // Evaluate parsed expression
    const auto value = evaluate(_func_F);

    // Keep points matching the criterion
    if (value > 0)
      _positions.push_back(centroid);
  }
  _initialized = true;
}
