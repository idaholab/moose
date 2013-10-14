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

#ifndef RECOVERBASEACTION_H
#define RECOVERBASEACTION_H

#include "Action.h"

class RecoverBaseAction;
class Output;

template<>
InputParameters validParams<RecoverBaseAction>();


class RecoverBaseAction : public Action
{
public:
  RecoverBaseAction(const std::string & name, InputParameters params);

  virtual void act();
  void RecoverBaseObject(Output & output, InputParameters & params);

protected:
  /**
   * Small helper to grab the newest restart file.
   *
   * Returns the full path to the restart file base to use.
   */
  std::string newestRestartFileWithBase(std::string base_name);
};

#endif // RECOVERBASEACTION_H
