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

#include "RandomInterfaceManager.h"
#include "RandomInterface.h"
#include "RandomData.h"

RandomInterfaceManager::RandomInterfaceManager(FEProblem & fe_problem) :
    GlobalManager(fe_problem)
{
}

void
RandomInterfaceManager::registerRandomInterface(RandomInterface & random_interface, const std::string & name)
{
  ;
  if (_random_data_objects.find(name) == _random_data_objects.end())
  {
    MooseSharedPointer<RandomData> random_data(new RandomData(_fe_problem, random_interface));
    random_interface.setRandomDataPointer(random_data);

    _random_data_objects[name] = random_data;
  }
  else
    random_interface.setRandomDataPointer(_random_data_objects[name]);
}

void
RandomInterfaceManager::initialSetup()
{
  for (RandomDataStore::iterator it = _random_data_objects.begin();
       it != _random_data_objects.end();
       ++it)
    it->second->updateSeeds(EXEC_INITIAL);
}

void
RandomInterfaceManager::timestepSetup()
{
  for (RandomDataStore::iterator it = _random_data_objects.begin();
       it != _random_data_objects.end();
       ++it)
    it->second->updateSeeds(EXEC_TIMESTEP_BEGIN);
}

void
RandomInterfaceManager::jacobianSetup()
{
  for (RandomDataStore::iterator it = _random_data_objects.begin();
       it != _random_data_objects.end();
       ++it)
    it->second->updateSeeds(EXEC_NONLINEAR);
}

void
RandomInterfaceManager::residualSetup()
{
  for (RandomDataStore::iterator it = _random_data_objects.begin();
       it != _random_data_objects.end();
       ++it)
    it->second->updateSeeds(EXEC_LINEAR);
}
