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

#ifndef TRANSFER_H
#define TRANSFER_H

#include "ParallelUniqueId.h"
#include "MooseObject.h"
#include "InputParameters.h"
#include "SetupInterface.h"
#include "MooseEnum.h"

class Transfer;
class SubProblem;
class FEProblem;
class SystemBase;

template<>
InputParameters validParams<Transfer>();

/**
 * Base class for all Transfer objects.
 *
 * Transfers are objects that take values from one Application
 * or System and put them in another Application or System.
 */
class Transfer :
  public MooseObject,
  public SetupInterface
{
public:
  Transfer(const std::string & name, InputParameters parameters);
  virtual ~Transfer() {}

  /**
   * Execute the transfer.
   */
  virtual void execute() = 0;

  /**
   * @return When this Transfer will be executed.
   */
  virtual int executeOn() { return _execute_on; }

protected:
  SubProblem & _subproblem;
  FEProblem & _fe_problem;
  SystemBase & _sys;

  THREAD_ID _tid;

  MooseEnum _execute_on;
};

#endif /* TRANSFER_H */
