//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADIntegratedBC.h"

class MeshAlignment2D2D;

/**
 * Base class for BC for components derived from HeatStructure2DCouplerBase
 */
class HeatStructure2DCouplerBCBase : public ADIntegratedBC
{
public:
  HeatStructure2DCouplerBCBase(const InputParameters & parameters);

protected:
  /**
   * Computes the coupled neighbor temperature
   */
  ADReal computeCoupledTemperature() const;

  /// Variable number of the variable to transfer
  const unsigned int _coupled_variable_number;
  /// Mesh alignment object
  const MeshAlignment2D2D & _mesh_alignment;

  /// Nonlinear system
  const SystemBase & _nl_sys;
  /// Solution vector
  const NumericVector<Number> * const & _serialized_solution;

public:
  static InputParameters validParams();
};
