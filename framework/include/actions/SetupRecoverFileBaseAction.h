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

#ifndef SETUPRECOVERFILEBASEACTION_H
#define SETUPRECOVERFILEBASEACTION_H

// MOOSE includes
#include "Action.h"

// Forward declerations
class SetupRecoverFileBaseAction;

template<>
InputParameters validParams<SetupRecoverFileBaseAction>();

/**
 *
 */
class SetupRecoverFileBaseAction : public Action
{
public:

  /**
   * Class constructor
   * @param name Name of the action
   * @param params Input parameters for this action
   */
  SetupRecoverFileBaseAction(const std::string & name, InputParameters params);

  /**
   * Class destructor
   */
  virtual ~SetupRecoverFileBaseAction();

  /**
   * Sets the recovery file base.
   */
  virtual void act();

private:

  /**
   * Extract all possible checkpoint file names
   * @param files A Set of checkpoint filenames to populate
   */
  void getCheckpointFiles(std::set<std::string> & files);

  /**
   * Extract the file base to utilize for recovery, uses the newest of the files in the supplied set
   * @param The most current checkpoing file base
   */
  std::string getRecoveryFileBase(const std::set<std::string> checkpoint_files);
};

#endif //SETUPRECOVERFILEBASEACTION_H
