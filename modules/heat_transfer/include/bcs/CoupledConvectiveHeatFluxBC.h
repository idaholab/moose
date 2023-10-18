//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "IntegratedBC.h"

/**
 * Boundary condition for convective heat flux where temperature and heat transfer coefficient are
 * given by auxiliary variables.  Typically used in multi-app coupling scenario. It is possible to
 * couple in a vector variable where each entry corresponds to a "phase".
 */
class CoupledConvectiveHeatFluxBC : public IntegratedBC
{
public:
  static InputParameters validParams();

  CoupledConvectiveHeatFluxBC(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();

  /// The number of components
  const unsigned int _n_components;
  /// Far-field temperature fields for each component
  const std::vector<const VariableValue *> _T_infinity;
  /// Convective heat transfer coefficient
  const std::vector<const VariableValue *> _htc;
  /// Volume fraction of individual phase
  const std::vector<const VariableValue *> _alpha;
  /// Scale factor
  const VariableValue & _scale_factor;
};
