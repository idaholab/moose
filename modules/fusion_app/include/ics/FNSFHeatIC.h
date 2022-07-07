//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "InitialCondition.h"

class FNSFHeatIC : public InitialCondition
{
public:
  static InputParameters validParams();

  FNSFHeatIC(const InputParameters & parameters);

protected:
  virtual Real value(const Point & p) override;

  std::vector<Real> _inner_xi_grid;
  std::vector<Real> _outer_xi_grid;
  std::vector<Real> _depth_grid;
  std::vector<Real> _heat;
};
