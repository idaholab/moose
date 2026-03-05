//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "SideIntegralPostprocessor.h"

class ADBoundaryFluxBase;

/**
 * Retrieves an entry of a flux vector for a 1-phase boundary
 */
class ADFlowBoundaryFlux1Phase : public SideIntegralPostprocessor
{
public:
  ADFlowBoundaryFlux1Phase(const InputParameters & parameters);

  virtual Real computeQpIntegral() override;

protected:
  /// Gets the flux index corresponding to the requested equation
  unsigned int getEquationIndex() const;

  /// Cross-sectional area, linear
  const ADVariableValue & _A_linear;

  // solution variables
  const ADMaterialProperty<Real> & _rhoA;
  const ADMaterialProperty<Real> & _rhouA;
  const ADMaterialProperty<Real> & _rhoEA;
  const ADMaterialProperty<std::vector<Real>> & _passives_times_area;

  /// Boundary component name
  const std::string & _boundary_name;
  /// Boundary user object name
  const std::string _boundary_uo_name;
  /// Boundary user object
  const ADBoundaryFluxBase & _boundary_uo;
  /// Index within flux vector to query
  const unsigned int _equation_index;

public:
  static InputParameters validParams();
};
