//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "LAROMANCEStressUpdateBase.h"

template <bool is_ad>
class SS316HLAROMANCEStressUpdateTestTempl : public LAROMANCEStressUpdateBaseTempl<is_ad>
{
public:
  static InputParameters validParams();

  SS316HLAROMANCEStressUpdateTestTempl(const InputParameters & parameters);

protected:
  virtual std::vector<
      std::vector<std::vector<std::vector<typename LAROMANCEStressUpdateBaseTempl<is_ad>::ROMInputTransform>>>>
  getTransform() override;
  virtual std::vector<std::vector<std::vector<std::vector<Real>>>> getTransformCoefs() override;
  virtual std::vector<std::vector<std::vector<std::vector<Real>>>> getInputLimits() override;
  virtual std::vector<std::vector<std::vector<std::vector<Real>>>> getCoefs() override;
  virtual std::vector<Real> getStrainCutoff() override { return {1.0e-10}; }
};

typedef SS316HLAROMANCEStressUpdateTestTempl<false> SS316HLAROMANCEStressUpdateTest;
typedef SS316HLAROMANCEStressUpdateTestTempl<true> ADSS316HLAROMANCEStressUpdateTest;
