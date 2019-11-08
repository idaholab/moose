//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "CoefDiffusion.h"

/**
 * A class for testing MooseObject::mooseConsole method
 */
class ConsoleMessageKernel : public CoefDiffusion
{
public:
  static InputParameters validParams();

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
