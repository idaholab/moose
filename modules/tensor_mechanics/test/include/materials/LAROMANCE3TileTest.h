/************************************************************************************/
/*                        © 2021 Triad National Security, LLC                       */
/*                                ALL RIGHTS RESERVED                               */
/*                                                                                  */
/* This software was produced under U.S. Government contract 89233218CNA000001 for  */
/* Los Alamos National Laboratory (LANL), which is operated by Triad National       */
/* Security, LLC for the U.S. Department of Energy/National Nuclear Security        */
/* Administration. The U.S. Government has rights to use, reproduce, and distribute */
/* this software. NEITHER THE GOVERNMENT NOR TRIAD NATIONAL SECURITY, LLC MAKES ANY */
/* WARRANTY, EXPRESS OR IMPLIED, OR ASSUMES ANY LIABILITY FOR THE USE OF THIS       */
/* SOFTWARE. If software is modified to produce derivative works, such modified     */
/* software should be clearly marked, so as not to confuse it with the version      */
/* available from LANL.                                                             */
/************************************************************************************/
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
