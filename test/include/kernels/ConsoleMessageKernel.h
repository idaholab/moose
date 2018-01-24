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

#ifndef CONSOLEMESSAGEKERNEL_H
#define CONSOLEMESSAGEKERNEL_H

// MOOSE includes
#include "CoefDiffusion.h"

// Forward declarations
class ConsoleMessageKernel;

template <>
InputParameters validParams<ConsoleMessageKernel>();

/**
 * A class for testing MooseObject::mooseConsole method
 */
class ConsoleMessageKernel : public CoefDiffusion
{
public:
  /**
   * Class constructor
   * @param parameters Input parameters
   */
  ConsoleMessageKernel(const InputParameters & parameters);

  /**
   * Class destructor
   */
  virtual ~ConsoleMessageKernel();

  /**
   * Prints a message on initial setup
   */
  void initialSetup();

  /*
   * Prints a message at beginning of timestep
   */
  void timestepSetup();

  /**
   * Prints from a const method
   */
  void constMethod() const;
};

#endif // CONSOLEMESSAGEKERNEL_H
