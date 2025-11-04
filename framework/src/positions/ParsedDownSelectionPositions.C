//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ParsedDownSelectionPositions.h"
#include "libmesh/parallel.h"
#include "libmesh/parallel_algebra.h"
#include "libmesh/vector_value.h"

registerMooseObject("MooseApp", ParsedDownSelectionPositions);

InputParameters
ParsedDownSelectionPositions::validParams()
{
  InputParameters params = Positions::validParams();
  params += NonADFunctorInterface::validParams();
  // Might as well offer block restriction as an additional down-selection criterion
  params += BlockRestrictable::validParams();
  params += FunctionParserUtils<false>::validParams();

  params.addRequiredParam<std::vector<PositionsName>>(
      "input_positions",
      "Positions object(s) that will be down-selected by this object. The order of the "
      "down-selected positions is kept the same");

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

  // We need to preserve the order of the positions from the input_positions objects
  params.set<bool>("auto_sort") = false;
  // We already perform a custom synchronization, because the functors cannot be evaluated on every
  // domain
  params.set<bool>("auto_broadcast") = false;

  // Keep as up-to-date as possible given the generality of functors
  params.set<ExecFlagEnum>("execute_on") = {EXEC_LINEAR, EXEC_TIMESTEP_BEGIN};

  params.addClassDescription("Examines input positions object(s) to find all positions matching a "
                             "user-specifed parsed expression criterion. The order of the "
                             "positions in the input is kept.");
  return params;
}

ParsedDownSelectionPositions::ParsedDownSelectionPositions(const InputParameters & parameters)
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
      paramError("functor_names",
                 "x, y, z, and t cannot be used in 'functor_names'. Use functor_symbols to "
                 "disambiguate by using a different symbol in the expression.");

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

  // Create parsed function
  _func_F = std::make_shared<SymFunction>();
  parsedFunctionSetup(_func_F, _expression, variables, {}, {}, comm());

  // reserve storage for parameter passing buffer
  _func_params.resize(_n_functors + 4);

  // keep pointers to the functors
  for (const auto & name : _functor_names)
    _functors.push_back(&getFunctor<Real>(name));
}

void
ParsedDownSelectionPositions::initialize()
{
  if (!_initialized)
  {
    // Retrieve the input positions
    const auto & base_names = getParam<std::vector<PositionsName>>("input_positions");
    for (const auto & base_name : base_names)
      if (_fe_problem.hasUserObject(base_name))
        _positions_ptrs.push_back(&_fe_problem.getPositionsObject(base_name));

    // Check execute-ons
    for (const auto pos_ptr : _positions_ptrs)
      if (!pos_ptr->getExecuteOnEnum().contains(_fe_problem.getCurrentExecuteOnFlag()))
        mooseInfo("Positions '",
                  pos_ptr->name(),
                  "' is not executing on ",
                  Moose::stringify(_fe_problem.getCurrentExecuteOnFlag()),
                  ". This could mean this position is not updated when down-selecting.");
  }

  clearPositions();
  const bool initial = _fe_problem.getCurrentExecuteOnFlag() == EXEC_INITIAL;

  // Pre-allocate for performance
  unsigned int n_points = 0;
  for (const auto pos_ptr : _positions_ptrs)
    n_points += pos_ptr->getNumPositions(initial);
  _positions.reserve(n_points);
  mooseAssert(comm().verify(n_points), "Input positions should be synchronized");

  // Rather than synchronize all ranks at every point, we will figure out whether to keep (2),
  // discard (1) or error (0, due to no ranks having made the decision) for each position
  std::vector<short> keep_positions(n_points, PositionSelection::Error);

  const auto state = determineState();
  auto pl = _fe_problem.mesh().getMesh().sub_point_locator();
  pl->enable_out_of_mesh_mode();

  // Loop over the positions, find them in the mesh to form the adequate functor arguments
  // Note that every positions object is assumed replicated over every rank already
  unsigned int i_pos, counter = 0;
  for (const auto & pos_ptr : _positions_ptrs)
    for (const auto & pos : pos_ptr->getPositions(initial))
    {
      counter++;
      i_pos = counter - 1;
      // Get all possible elements the position may be in
      std::set<const Elem *> candidate_elements;
      (*pl)(pos, candidate_elements);

      for (const auto elem : candidate_elements)
      {
        // Check block restriction
        // Dont exclude a point we already chose to keep. This 'inclusivity' means that a position
        // at a node can be included if at least one element it borders is in the block restriction
        if (!hasBlocks(elem->subdomain_id()) && (keep_positions[i_pos] != PositionSelection::Keep))
        {
          keep_positions[i_pos] = PositionSelection::Discard;
          continue;
        }

        // We can't guarantee we have enough algebraic ghosting for variable functors. Might as well
        // skip, another process will take care of it
        if (elem->processor_id() != processor_id())
          continue;

        // Form a functor argument
        const Moose::ElemPointArg elem_arg = {elem, pos, false};

        // Fill arguments to the parsed expression
        // Functors
        for (const auto i : index_range(_functors))
          _func_params[i] = (*_functors[i])(elem_arg, state);

        // Positions and time
        for (const auto j : make_range(Moose::dim))
          _func_params[_n_functors + j] = pos(j);
        _func_params[_n_functors + 3] = _t;

        // Evaluate parsed expression
        const auto value = evaluate(_func_F);

        // Keep points matching the criterion
        if (value > 0)
          keep_positions[i_pos] = PositionSelection::Keep;
        // Dont exclude a point we already chose to keep. This 'inclusivity'
        // means that a position at an interface between elements can get included if the functor
        // evaluates greater than 0 for any of the elements used in forming ElemPointArgs
        else if (keep_positions[i_pos] != PositionSelection::Keep)
          keep_positions[i_pos] = PositionSelection::Discard;
      }
    }

  // Synchronize which positions to keep across all ranks
  comm().max(keep_positions);
  i_pos = 0;
  for (const auto & pos_ptr : _positions_ptrs)
    for (const auto & pos : pos_ptr->getPositions(initial))
    {
      if (keep_positions[i_pos] == PositionSelection::Keep)
        _positions.push_back(pos);
      else if (keep_positions[i_pos] == PositionSelection::Error)
        mooseError(
            "No process has made a decision on whether position '",
            pos,
            "' from Positions object '" + pos_ptr->name() +
                "' should be discarded or kept during down-selection. This usually means this "
                "position is outside the mesh!");
      i_pos++;
    }

  _initialized = true;
}
