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
#include "Moose.h"

// Forward declarations
class MooseApp;
class MooseObject;
class FEProblemBase;

/**
 * An interface to communicate an invalid solution state to FEProblemBase
 */
class SolutionInvalidInterface
{
public:
  /**
   * A class for providing a helper object for communicating to FEProblemBase
   */
  SolutionInvalidInterface(MooseObject * moose_object);
  void setSolutionInvalid(bool solution_invalid);

private:
  /// A reference to the FEProblem
  FEProblemBase & _si_fe_problem;
};
