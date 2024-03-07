//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DistributedPositions.h"

registerMooseObject("MooseApp", DistributedPositions);

InputParameters
DistributedPositions::validParams()
{
  InputParameters params = Positions::validParams();

  params.addRequiredParam<std::vector<PositionsName>>(
      "positions",
      "Positions object. The last Positions is distributed over all the Positions prior in the "
      "vector parameter.");

  // Use position ordering from the distribution
  params.set<bool>("auto_sort") = false;
  // If the base position is broadcast already we do not need to
  params.set<bool>("auto_broadcast") = false;
  // Keep as up-to-date as possible given that the incoming position to distribute could be changing
  params.set<ExecFlagEnum>("execute_on") = {EXEC_LINEAR, EXEC_TIMESTEP_BEGIN};

  params.addClassDescription(
      "Distribute positions, using translations, over one or more positions");
  return params;
}

DistributedPositions::DistributedPositions(const InputParameters & parameters)
  : Positions(parameters)
{
  const auto & base_names = getParam<std::vector<PositionsName>>("positions");
  for (const auto & base_name : base_names)
    if (_fe_problem.hasUserObject(base_name))
      _positions_objs.push_back(&_fe_problem.getPositionsObject(base_name));
    else
      mooseError("Positions ",
                 base_name,
                 " has not been created yet. If it exists, re-order Positions in the input "
                 "file or implement automated construction ordering");

  // Obtain the positions from the nested positions objects, then transform them
  initialize();
  // Sort if needed (user-specified)
  finalize();
}

void
DistributedPositions::initialize()
{
  clearPositions();
  const bool initial = _fe_problem.getCurrentExecuteOnFlag() == EXEC_INITIAL;

  // Check that everything is initialized
  for (const auto * const pos_obj : _positions_objs)
    if (!pos_obj->initialized(initial))
      mooseError("Positions '", pos_obj->name(), "' is not initialized.");

  // Size new positions vector
  unsigned int n_positions = 1;
  for (const auto * const pos_obj : _positions_objs)
  {
    const auto n_pos_obj = pos_obj->getNumPositions(initial);
    if (n_pos_obj == 0)
      paramError("positions", "Positions " + pos_obj->name() + " has 0 positions.");
    n_positions *= pos_obj->getNumPositions(initial);
  }
  _positions.resize(n_positions);

  // Fill _positions by distributing using translations
  _positions[0] = Point(0, 0, 0);
  unsigned int current_index = 1;
  for (const auto pos_i : index_range(_positions_objs))
  {
    const auto * const current_positions = _positions_objs[_positions_objs.size() - 1 - pos_i];
    for (const auto i : make_range(current_index))
    {
      unsigned int j = 0;
      for (const auto & translation : current_positions->getPositions(initial))
        _positions[i + (j++) * current_index] += translation;
    }
    current_index *= current_positions->getNumPositions(initial);
  }

  _initialized = true;
}
