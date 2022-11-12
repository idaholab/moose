//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralUserObject.h"
#include "GeochemistryKineticRateCalculator.h"

/**
 * User object that defines a kinetic rate
 */
class GeochemistryKineticRate : public GeneralUserObject
{
public:
  static InputParameters validParams();

  GeochemistryKineticRate(const InputParameters & parameters);

  virtual void initialize() override final;
  virtual void execute() override final;
  virtual void finalize() override final;

  /// provides a reference to the rate description held by this object
  const KineticRateUserDescription & getRateDescription() const;

private:
  const std::vector<std::string> _promoting_names;
  const std::vector<Real> _monod_ind;
  const std::vector<Real> _half_sat;
  const KineticRateUserDescription _rate_description;
};
