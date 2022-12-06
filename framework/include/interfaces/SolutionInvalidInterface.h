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
#include "SolutionInvalidity.h"

// Forward declarations
class MooseApp;
class MooseObject;
class FEProblemBase;

/**
 * An interface to communicate the solutioninvalidity for one app to the materials
 */
class SolutionInvalidInterface
{
public:
  /**
   * A class to help communicating the solutioninvalidity for one app to the materials
   */
  SolutionInvalidInterface(MooseApp & moose_app);

  void setSolutionInvalid(SolutionID _solution_id);

  /**
   * Get the SolutionInvalidity
   */
  SolutionInvalidity & solutionInvalidity();

protected:
  SolutionID registerInvalidSection(const std::string & section_name) const;

  /// The MooseApp that owns the SolutionInvalidity
  MooseApp & _si_moose_app;
};
