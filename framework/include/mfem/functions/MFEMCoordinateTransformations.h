//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once
#include "Function.h"

class MFEMCoordinateTransformations : public Function
{
public:
  static InputParameters validParams();
  MFEMCoordinateTransformations(const InputParameters & parameters);
  using Function::value;
  virtual Real value(Real /* t */, const Point & /* p */) const override { return 0.0; }

  const MooseEnum & coordType() const { return _coord_type; }
  Real invREps() const { return _inv_r_eps; }
  std::string coefficientName(const std::string & base) const { return name() + "_" + base; }

protected:
  const MooseEnum _coord_type;
  const Real _inv_r_eps;
};
