//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADOneDIntegratedBC.h"

class ADGateValve1PhaseUserObject;

/**
 * Adds boundary fluxes for flow channels connected to a 1-phase gate valve
 */
class ADGateValve1PhaseBC : public ADOneDIntegratedBC
{
public:
  ADGateValve1PhaseBC(const InputParameters & params);

protected:
  virtual ADReal computeQpResidual() override;

  /**
   * Creates mapping of coupled variable index to local equation system index
   */
  std::map<unsigned int, unsigned int> getIndexMapping() const;

  /// Index of connected flow channel
  const unsigned int _connection_index;

  /// 1-phase gate valve user object
  const ADGateValve1PhaseUserObject & _gate_valve_uo;

  /// Cross-sectional area, elemental
  const ADVariableValue & _A_elem;
  /// Cross-sectional area, linear
  const ADVariableValue & _A_linear;

  /// rho*A coupled variable index
  const unsigned int _rhoA_jvar;
  /// rho*u*A coupled variable index
  const unsigned int _rhouA_jvar;
  /// rho*E*A coupled variable index
  const unsigned int _rhoEA_jvar;

  /// Map of coupled variable index to local equation system index
  const std::map<unsigned int, unsigned int> _jvar_map;
  /// Index within local system of the equation upon which this object acts
  const unsigned int _equation_index;

public:
  static InputParameters validParams();
};
