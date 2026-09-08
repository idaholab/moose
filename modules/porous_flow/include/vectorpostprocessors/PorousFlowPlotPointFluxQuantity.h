//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralVectorPostprocessor.h"

class PorousFlowPointFluxQuantity;

/**
 * Extracts the per-point flux, and the coordinates of each point, from a
 * PorousFlowPointFluxQuantity userobject
 */
class PorousFlowPlotPointFluxQuantity : public GeneralVectorPostprocessor
{
public:
  static InputParameters validParams();

  PorousFlowPlotPointFluxQuantity(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override;

protected:
  /// The PorousFlowPointFluxQuantity userobject
  const PorousFlowPointFluxQuantity & _point_fluxes;

  /// Dirac point ID (0, 1, 2, ...) of each row
  VectorPostprocessorValue & _point_id;

  ///@{ Coordinates of each point
  VectorPostprocessorValue & _x;
  VectorPostprocessorValue & _y;
  VectorPostprocessorValue & _z;
  ///@}

  /// Flux recorded at each point
  VectorPostprocessorValue & _flux;
};
