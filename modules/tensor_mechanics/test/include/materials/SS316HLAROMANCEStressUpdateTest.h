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

template <ComputeStage>
class SS316HLAROMANCEStressUpdateTest;

declareADValidParams(SS316HLAROMANCEStressUpdateTest);

template <ComputeStage compute_stage>
class SS316HLAROMANCEStressUpdateTest : public ADLAROMANCEStressUpdateBase<compute_stage>
{
public:
  SS316HLAROMANCEStressUpdateTest(const InputParameters & parameters);

protected:
  /// Returns vector of the functions to use for the conversion of input variables.
  virtual std::vector<std::vector<ROMInputTransform>> getTransform() override;

  /// Returns factors for the functions for the conversion functions given in getTransform
  virtual std::vector<std::vector<Real>> getTransformCoefs() override;

  /* Returns human-readable limits for the inputs. Inputs ordering is
   * 0: mobile_old
   * 1: immobile_old
   * 2: trial stress,
   * 3: effective strain old,
   * 4: temperature
   * 5: environmental factor (optional)
   */
  virtual std::vector<std::vector<Real>> getInputLimits() override;

  /// Material specific coefficients multiplied by the Legendre polynomials for each of the input variables
  virtual std::vector<std::vector<Real>> getCoefs() override;

  usingADLAROMANCEStressUpdateBase;
};
