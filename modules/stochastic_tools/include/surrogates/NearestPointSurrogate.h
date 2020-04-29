//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "SurrogateModel.h"

class NearestPointSurrogate : public SurrogateModel
{
public:
  static InputParameters validParams();
  NearestPointSurrogate(const InputParameters & parameters);
  virtual Real evaluate(const std::vector<Real> & x) const override;

protected:
  /// Array containing sample points and the results
  const std::vector<std::vector<Real>> & _sample_points;
};
