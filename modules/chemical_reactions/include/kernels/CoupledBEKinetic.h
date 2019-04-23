//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "TimeDerivative.h"

class CoupledBEKinetic;

template <>
InputParameters validParams<CoupledBEKinetic>();

/**
 * Derivative of mineral species concentration wrt time
 */
class CoupledBEKinetic : public TimeDerivative
{
public:
  CoupledBEKinetic(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;

private:
  /// Porosity
  const MaterialProperty<Real> & _porosity;
  /// Weight of the kinetic mineral concentration in the total primary species concentration
  const std::vector<Real> _weight;
  /// Coupled kinetic mineral concentrations
  std::vector<const VariableValue *> _vals;
  /// Coupled old values of kinetic mineral concentrations
  std::vector<const VariableValue *> _vals_old;
};

