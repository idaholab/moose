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
#ifndef INITIALCONDITIONSTORAGE_H
#define INITIALCONDITIONSTORAGE_H

#include "MooseObjectStorageBase.h"
#include "MooseTypes.h"

class InitialCondition;

/**
 * Warehouse for storing initial conditions
 */
class InitialConditionStorage : public MooseObjectStorageBase<InitialCondition>
{
public:
  InitialConditionStorage();

  /**
   * Initial setup
   */
  void initialSetup(THREAD_ID tid);

  /**
   * Add object to the warehouse.
   */
  void addObject(MooseSharedPointer<InitialCondition> object, THREAD_ID tid);


protected:

  ///@{
  /// Variable name to block/boundary IDs for error checking
  std::map<std::string, std::set<BoundaryID> > _boundary_ics;
  std::map<std::string, std::set<SubdomainID> > _block_ics;
  ///@}
};

#endif /* INITIALCONDITIONSTORAGE_H */
