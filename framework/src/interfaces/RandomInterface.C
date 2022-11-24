//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "Moose.h"
#include "RandomInterface.h"
#include "RandomData.h"
#include "MooseRandom.h"
#include "FEProblemBase.h"
#include "Assembly.h"

InputParameters
RandomInterface::validParams()
{

  InputParameters params = emptyInputParameters();
  params.addParam<unsigned int>("seed", 0, "The seed for the master random number generator");

  params.addParamNamesToGroup("seed", "Advanced");
  return params;
}

RandomInterface::RandomInterface(const InputParameters & parameters,
                                 FEProblemBase & problem,
                                 THREAD_ID tid,
                                 bool is_nodal)
  : _random_data(nullptr),
    _generator(nullptr),
    _ri_problem(problem),
    _ri_name(parameters.get<std::string>("_object_name")),
    _master_seed(parameters.get<unsigned int>("seed")),
    _is_nodal(is_nodal),
    _reset_on(EXEC_LINEAR),
    _curr_node(problem.assembly(tid, 0).node()),
    _curr_element(problem.assembly(tid, 0).elem())
{
}

RandomInterface::~RandomInterface() {}

void
RandomInterface::setRandomResetFrequency(ExecFlagType exec_flag)
{
  _reset_on = exec_flag;
  _ri_problem.registerRandomInterface(*this, _ri_name);
}

void
RandomInterface::setRandomDataPointer(RandomData * random_data)
{
  _random_data = random_data;
  _generator = &_random_data->getGenerator();
}

unsigned int
RandomInterface::getSeed(std::size_t id)
{
  mooseAssert(_random_data, "RandomData object is NULL!");

  return _random_data->getSeed(id);
}

unsigned long
RandomInterface::getRandomLong() const
{
  mooseAssert(_generator, "Random Generator is NULL, did you call setRandomResetFrequency()?");

  dof_id_type id;
  if (_is_nodal)
    id = _curr_node->id();
  else
    id = _curr_element->id();

  return _generator->randl(id);
}

Real
RandomInterface::getRandomReal() const
{
  mooseAssert(_generator, "Random Generator is NULL, did you call setRandomResetFrequency()?");

  dof_id_type id;
  if (_is_nodal)
    id = _curr_node->id();
  else
    id = _curr_element->id();

  return _generator->rand(id);
}
