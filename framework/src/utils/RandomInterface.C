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

#include "Moose.h"
#include "RandomInterface.h"
#include "Assembly.h"
#include "RandomData.h"
#include "MooseRandom.h"

template<>
InputParameters validParams<RandomInterface>()
{
  InputParameters params = emptyInputParameters();
  params.addParam<unsigned int>("seed", 0, "The seed for the master random number generator");

  params.addParamNamesToGroup("seed", "Advanced");
  return params;
}

RandomInterface::RandomInterface(const std::string & name, InputParameters & parameters,
                                 FEProblem & problem, THREAD_ID tid, bool is_nodal) :
    _random_data(NULL),
    _generator(NULL),
    _ri_problem(problem),
    _ri_name(name),
    _master_seed(parameters.get<unsigned int>("seed")),
    _is_nodal(is_nodal),
    _reset_on(EXEC_RESIDUAL),
    _curr_node(problem.assembly(tid).node()),
    _curr_element(problem.assembly(tid).elem())
{
}

RandomInterface::~RandomInterface()
{
}

void
RandomInterface::setRandomResetFrequency(ExecFlagType exec_flag)
{
  _ri_problem.registerRandomInterface(*this, _ri_name, exec_flag);
}

void
RandomInterface::setRandomDataPointer(RandomData *random_data)
{
  _random_data = random_data;
  _generator = &_random_data->getGenerator();
}

unsigned int
RandomInterface::getSeed(unsigned int id)
{
  mooseAssert(_random_data, "RandomData object is NULL!");

  return _random_data->getSeed(id);
}

unsigned long
RandomInterface::getRandomLong()
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
RandomInterface::getRandomReal()
{
  mooseAssert(_generator, "Random Generator is NULL, did you call setRandomResetFrequency()?");

  dof_id_type id;
  if (_is_nodal)
    id = _curr_node->id();
  else
    id = _curr_element->id();

  return _generator->rand(id);
}
