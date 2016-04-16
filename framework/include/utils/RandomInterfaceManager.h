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

#ifndef RANDOMINTERFACEMANAGER_H
#define RANDOMINTERFACEMANAGER_H

#include "GlobalManager.h"
#include "MooseTypes.h"

#include <string>
#include <map>

class RandomInterface;
class RandomData;

/**
 * Manage the RandomInterface features in FEProblem
 */
class RandomInterfaceManager : public GlobalManager
{
public:
  RandomInterfaceManager(FEProblem & fe_problem);

  void registerRandomInterface(RandomInterface & random_interface, const std::string & name);

protected:
  ///@{ Setup methods called by FEProblem
  virtual void initialSetup();
  virtual void timestepSetup();
  virtual void jacobianSetup();
  virtual void residualSetup();
  ///@}

  typedef std::map<std::string, MooseSharedPointer<RandomData> > RandomDataStore;
  RandomDataStore _random_data_objects;
};

#endif //RANDOMINTERFACEMANAGER_H
