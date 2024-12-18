//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralPostprocessor.h"

class TransientBase;

/**
 * Gets the relative solution norm from the transient executioner
 */
class RelativeSolutionDifferenceNorm : public GeneralPostprocessor
{
public:
  static InputParameters validParams();

  /**
   * Class constructor
   * @param parameters The input parameters
   */
  RelativeSolutionDifferenceNorm(const InputParameters & parameters);

  ///@{
  /**
   * No action taken
   */
  virtual void initialize() override {}
  virtual void execute() override {}
  ///@}

  /**
   * Returns the relative solution norm taken from the transient executioner
   * @return A const reference to the value of the postprocessor
   */
  virtual Real getValue() const override;

protected:
  /// Transient executioner
  TransientBase * _trex;
};
