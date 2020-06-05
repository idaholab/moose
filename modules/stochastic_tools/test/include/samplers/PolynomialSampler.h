//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Sampler.h"

/**
 * Test class to fuzzy data that follows a polynomial function
 */
class PolynomialSampler : public Sampler
{
public:
  static InputParameters validParams();
  PolynomialSampler(const InputParameters & parameters);
  virtual void computeSampleRow(dof_id_type i, std::vector<Real> & data) override;

protected:
  virtual void advanceGenerators(dof_id_type count) override;

  const Real & _x_min;
  const Real & _x_max;
  const Real & _noise;
  const std::vector<Real> & _coefficients;
};
