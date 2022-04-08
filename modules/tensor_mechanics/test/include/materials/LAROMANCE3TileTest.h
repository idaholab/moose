//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "LAROMANCEPartitionStressUpdateBase.h"

template <bool is_ad>
class LAROMANCE3TileTestTempl : public LAROMANCEPartitionStressUpdateBaseTempl<is_ad>
{
public:
  static InputParameters validParams();

  LAROMANCE3TileTestTempl(const InputParameters & parameters);

protected:
  /**
   * Inputs ordering is
   * input[0]: cell_old
   * input[1]: wall_old
   * input[2]: trial stress,
   * input[3]: effective strain old,
   * input[4]: temperature
   **/
  virtual std::vector<
      std::vector<std::vector<std::vector<typename LAROMANCEPartitionStressUpdateBaseTempl<is_ad>::ROMInputTransform>>>>
  getTransform() override;
  virtual std::vector<std::vector<std::vector<std::vector<Real>>>> getTransformCoefs() override;
  virtual std::vector<std::vector<std::vector<std::vector<Real>>>>
  getNormalizationLimits() override;
  virtual std::vector<std::vector<std::vector<std::vector<Real>>>> getInputLimits() override;
  virtual std::vector<std::vector<std::vector<std::vector<Real>>>> getCoefs() override;
  virtual std::vector<std::vector<unsigned int>> getTilings() override;
  virtual std::vector<Real> getStrainCutoff() override { return {1.0e-17, 1.0e-17}; }
  virtual std::vector<std::vector<Real>> getClassificationLuu() override;
  virtual std::vector<std::vector<Real>> getClassificationXu() override;
  virtual DenseVector<Real> getClassificationVind() override;
  virtual std::vector<Real> getClassificationMmean() override;
  virtual std::vector<Real> getClassificationMscale() override;
  virtual Real getClassificationEll() override;
  virtual Real getClassificationEta() override;
};

typedef LAROMANCE3TileTestTempl<false> LAROMANCE3TileTest;
typedef LAROMANCE3TileTestTempl<true> ADLAROMANCE3TileTest;
