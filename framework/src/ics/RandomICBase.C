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

namespace
{
/**
 * Method for retrieving or generating and caching a value in a map.
 */
inline Real
valueHelper(dof_id_type id, MooseRandom & generator, std::map<dof_id_type, Real> & map)
{
  auto it_pair = map.lower_bound(id);

  // Do we need to generate a new number?
  if (it_pair == map.end() || it_pair->first != id)
    it_pair = map.emplace_hint(it_pair, id, generator.rand(id));

  return it_pair->second;
}
}

InputParameters
RandomICBase::validParams()
{
  InputParameters params = InitialCondition::validParams();
  params.addParam<unsigned int>("seed", 0, "Seed value for the random number generator");
  params.addParam<bool>(
      "legacy_generator",
      false,
      "Determines whether or not the legacy generator (deprecated) should be used.");

  params.addClassDescription("Base class for generating a random field for a variable.");
  return params;
}

RandomICBase::RandomICBase(const InputParameters & parameters)
  : InitialCondition(parameters),
    _is_nodal(_var.isNodal()),
    _use_legacy(getParam<bool>("legacy_generator")),
    _elem_random_generator(nullptr),
    _node_random_generator(nullptr)
{
  unsigned int processor_seed = getParam<unsigned int>("seed");
  MooseRandom::seed(processor_seed);

  if (_use_legacy)
  {
    auto proc_id = processor_id();
    if (proc_id > 0)
    {
      for (processor_id_type i = 0; i < proc_id; ++i)
        processor_seed = MooseRandom::randl();
      MooseRandom::seed(processor_seed);
    }
  }
  else
  {
    _elem_random_data =
        std::make_unique<RandomData>(_fe_problem, false, EXEC_INITIAL, MooseRandom::randl());
    _node_random_data =
        std::make_unique<RandomData>(_fe_problem, true, EXEC_INITIAL, MooseRandom::randl());

    _elem_random_generator = &_elem_random_data->getGenerator();
    _node_random_generator = &_node_random_data->getGenerator();
  }
}

void
RandomICBase::initialSetup()
{
  if (!_use_legacy)
  {
    _elem_random_data->updateSeeds(EXEC_INITIAL);
    _node_random_data->updateSeeds(EXEC_INITIAL);
  }
}

Real
RandomICBase::generateRandom()
{
  Real rand_num;

  if (_use_legacy)
  {
    mooseDeprecated("legacy_generator is deprecated. Please set \"legacy_generator = false\". This "
                    "capability will be removed after 11/01/2018");

    // Random number between 0 and 1
    rand_num = MooseRandom::rand();
  }
  else
  {
    if (_current_node)
      rand_num = valueHelper(_current_node->id(), *_node_random_generator, _node_numbers);
    else if (_current_elem)
      rand_num = valueHelper(_current_elem->id(), *_elem_random_generator, _elem_numbers);
    else
      mooseError("We can't generate parallel consistent random numbers for this kind of variable "
                 "yet. Please contact the MOOSE team for assistance");
  }

  return rand_num;
}
