//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "RandomICBase.h"
#include "MooseRandom.h"

#include "libmesh/point.h"

defineLegacyParams(RandomICBase);

InputParameters
RandomICBase::validParams()
{
  InputParameters params = InitialCondition::validParams();
  params.addParam<unsigned int>("seed", 0, "Seed value for the random number generator");

  params.addClassDescription("Base class for generating a random field for a variable.");
  return params;
}

RandomICBase::RandomICBase(const InputParameters & parameters)
  : InitialCondition(parameters), _is_nodal(_var.isNodal())
{
  MooseRandom::seed(getParam<unsigned int>("seed"));

  _elem_random_data =
      std::make_unique<RandomData>(_fe_problem, false, EXEC_INITIAL, MooseRandom::randl());
  _node_random_data =
      std::make_unique<RandomData>(_fe_problem, true, EXEC_INITIAL, MooseRandom::randl());
}

void
RandomICBase::initialSetup()
{
  _elem_random_data->updateSeeds(EXEC_INITIAL);
  _node_random_data->updateSeeds(EXEC_INITIAL);
}

Real
RandomICBase::generateRandom()
{
  const auto get_value =
      [](const dof_id_type id, MooseRandom & generator, std::map<dof_id_type, Real> & map) {
        auto it_pair = map.lower_bound(id);

        // Do we need to generate a new number?
        if (it_pair == map.end() || it_pair->first != id)
          it_pair = map.emplace_hint(it_pair, id, generator.rand(id));

        return it_pair->second;
      };

  if (_current_node)
    return get_value(_current_node->id(), _node_random_data->getGenerator(), _node_numbers);
  if (_current_elem)
    return get_value(_current_elem->id(), _elem_random_data->getGenerator(), _elem_numbers);

  mooseError("We can't generate parallel consistent random numbers for this kind of variable "
             "yet. Please contact the MOOSE team for assistance");
}
