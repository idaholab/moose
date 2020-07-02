//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// Moose Includes
#include "DiracKernel.h"
#include "PenetrationLocator.h"

// Forward Declarations

class GapHeatPointSourceMaster : public DiracKernel
{
public:
  static InputParameters validParams();

  GapHeatPointSourceMaster(const InputParameters & parameters);

  virtual void addPoints();
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();

protected:
  PenetrationLocator & _penetration_locator;
  std::map<Point, PenetrationInfo *> point_to_info;
  NumericVector<Number> & _secondary_flux;

  //  std::vector<Real> _localized_secondary_flux;
};
