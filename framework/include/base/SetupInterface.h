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

#ifndef SETUPINTERFACE_H
#define SETUPINTERFACE_H

#include "InputParameters.h"
#include "ExecStore.h"
#include "MooseEnum.h"

// Forward declarations
class SetupInterface;

template<>
InputParameters validParams<SetupInterface>();

class SetupInterface
{
public:
  SetupInterface(InputParameters & params);
  virtual ~SetupInterface();

  /**
   * Gets called at the beginning of the simulation before this object is asked to do its job
   */
  virtual void initialSetup();

  /**
   * Gets called at the beginning of the timestep before this object is asked to do its job
   */
  virtual void timestepSetup();

  /**
   * Gets called just before the jacobian is computed and before this object is asked to do its job
   */
  virtual void jacobianSetup();

  /**
   * Gets called just before the residual is computed and before this object is asked to do its job
   */
  virtual void residualSetup();

  /**
   * Gets called when the subdomain changes (ie in a jacobian or residual loop) and before this object is asked to do its job
   */
  virtual void subdomainSetup();

  /**
   * Get the execution falg for the object
   */
  virtual ExecFlagType execFlag() const;

  /**
   * Returns the available options for the 'execute_on' input parameters
   * @return A MooseEnum with the avaiable 'execute_on' options, the default is 'residual'
   */
  static MooseEnum getExecuteOptions();

protected:
  /// execution flag (when is the object executed/evaluated)
  ExecFlagType _exec_flags;
};

#endif /* SETUPINTERFACE_H */
