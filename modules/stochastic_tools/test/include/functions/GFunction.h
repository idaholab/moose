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

/* SOBOL test function, see Slaughter, Eq. 5.51 */
class GFunction : public GeneralVectorPostprocessor
{
public:
  static InputParameters validParams();
  GFunction(const InputParameters & parameters);
  virtual void execute() override;
  virtual void initialize() override {}
  virtual void finalize() override;

protected:
  Sampler & _sampler;
  const std::vector<Real> & _q_vector;
  const bool & _classify;
  const Real & _limiting_value;
  VectorPostprocessorValue & _values;
};
