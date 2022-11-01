//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Distribution.h"

/**
 * A class used to generate a Student's t distribution
 */
class StudentT : public Distribution
{
public:
  static InputParameters validParams();

  StudentT(const InputParameters & parameters);

  virtual Real pdf(const Real & x) const override;
  virtual Real cdf(const Real & x) const override;
  virtual Real quantile(const Real & p) const override;

  static Real pdf(const Real & x, const unsigned int & dof);
  static Real cdf(const Real & x, const unsigned int & dof);
  static Real quantile(const Real & p, const unsigned int & dof);

protected:
  /// Degrees of freedom
  const unsigned int & _dof;
};
