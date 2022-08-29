//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "NodalPostprocessorBasePD.h"

/**
 * Postprocessor class to compute a volume integral of the specified variable
 * Note that specializations of this integral are possible by deriving from this
 * class and overriding computeNodalIntegral()
 */
class NodalIntegralPostprocessorBasePD : public NodalPostprocessorBasePD
{
public:
  static InputParameters validParams();

  NodalIntegralPostprocessorBasePD(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override;
  virtual void threadJoin(const UserObject & uo) override;
  virtual Real getValue() override;
  virtual void finalize() override;

protected:
  /**
   * Function to evaluate the given function at each material point
   * @return The computed nodal value
   */
  virtual Real computeNodalValue() = 0;

  /// nodal area integral result
  Real _integral_value;
};
