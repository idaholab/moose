//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralPostprocessor.h"

class AveragePointSeparation : public GeneralPostprocessor
{
public:
  static InputParameters validParams();

  AveragePointSeparation(const InputParameters & parameters);

  virtual void initialize() override {}
  virtual void execute() override;
  virtual void finalize() override {}
  virtual Real getValue() override;

protected:
  std::vector<VariableName> _displacements;

  /// The vector of displacements number we are operating on
  std::vector<int> _disp_num;

  /// A reference to the system containing the variable
  const System & _system;

  /// The vectors of start and end points
  const std::vector<Point> & _first_point;
  const std::vector<Point> & _last_point;

  /// The value of the average separation between the sets of points
  Real _value;
};
