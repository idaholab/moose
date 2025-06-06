//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "InitialCondition.h"

/**
 * Initial condition for specific total enthalpy
 *
 * This computes the specific total enthalpy:
 * \f[
 *   H = E + \frac{p}{\rho}
 * \f]
 */
class SpecificTotalEnthalpyIC : public InitialCondition
{
public:
  SpecificTotalEnthalpyIC(const InputParameters & parameters);

protected:
  virtual Real value(const Point & /*p*/);

  const VariableValue & _rhoA;
  const VariableValue & _rhoEA;
  const VariableValue & _pressure;
  const VariableValue & _area;
  const VariableValue & _alpha;

public:
  static InputParameters validParams();
};
