//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADLAROMANCEStressUpdateBase.h"

class SS316HLAROMANCEStressUpdateTest : public ADLAROMANCEStressUpdateBase
{
public:
  static InputParameters validParams();

  SS316HLAROMANCEStressUpdateTest(const InputParameters & parameters);

protected:
  virtual std::vector<std::vector<std::vector<ROMInputTransform>>> getTransform() override;
  virtual std::vector<std::vector<std::vector<Real>>> getTransformCoefs() override;
  virtual std::vector<std::vector<std::vector<Real>>> getInputLimits() override;
  virtual std::vector<std::vector<std::vector<Real>>> getCoefs() override;

  virtual Real romStrainCutoff() override { return 1.0e-10; }
};
