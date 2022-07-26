//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "HeatStructure2DCouplerBase.h"

/**
 * Couples boundaries of two 2D cylindrical heat structures via radiation
 */
class HeatStructure2DRadiationCouplerRZ : public HeatStructure2DCouplerBase
{
public:
  HeatStructure2DRadiationCouplerRZ(const InputParameters & parameters);

  virtual void addMooseObjects() override;

protected:
  virtual void init() override;
  virtual void check() const override;

  /// Emissivities for the primary and secondary sides
  const std::vector<Real> _emissivities;
  /// View factors for the primary and secondary sides
  std::vector<Real> _view_factors;
  /// Perimeters for the primary and secondary sides
  std::vector<Real> _perimeters;

public:
  static InputParameters validParams();
};
