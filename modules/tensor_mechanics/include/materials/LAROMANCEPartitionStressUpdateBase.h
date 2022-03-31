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
class LAROMANCEPartitionStressUpdateBaseTempl : public LAROMANCEStressUpdateBaseTempl<is_ad>
{
public:
  static InputParameters validParams();

  LAROMANCEPartitionStressUpdateBaseTempl(const InputParameters & parameters);

protected:
  virtual void initialSetup() override;
  virtual GenericReal<is_ad> computeSecondPartitionWeight() override;
  virtual void computeDSecondPartitionWeightDStress(
      GenericReal<is_ad> & dsecond_partition_weight_dstress) override;

  ///@{ Method and container for the Gaussian Process Regression lower triangular covariance matrix
  virtual std::vector<std::vector<Real>> getClassificationLuu() = 0;
  std::vector<std::vector<Real>> _partition_Luu;
  ///@}

  ///@{ Method and container for the Gaussian Process Regression model training points
  virtual std::vector<std::vector<Real>> getClassificationXu() = 0;
  std::vector<std::vector<Real>> _partition_Xu;
  ///@}

  ///@{ Method and container for the inducing points of the Gaussian Process Regression model
  virtual std::vector<Real> getClassificationVind() = 0;
  std::vector<Real> _partition_Vind;
  ///@}

  ///@{ Method and container for the mean values of the training input
  virtual std::vector<Real> getClassificationMmean() = 0;
  std::vector<Real> _partition_Mmean;
  ///@}

  ///@{ Method and container for the scale factor of the training input points to normalize all input parameters to equivalent values
  virtual std::vector<Real> getClassificationMscale() = 0;
  std::vector<Real> _partition_Mscale;
  ///@}

  ///@{ Method and container for the calibrated Gaussian Regression Model hyperparameter "Ell", which controls the decay of the covariance as a function of distance
  virtual Real getClassificationEll() = 0;
  Real _partition_Ell;
  ///@}

  ///@{ Method and container for the calibrated Gaussian Regression Model hyperparameter "Eta", which is a scale factor that controls the amplitude of the mean
  virtual Real getClassificationEta() = 0;
  Real _partition_Eta;
  ///@}

  ///@{ Containers for parition math
  std::vector<std::vector<GenericReal<is_ad>>> _partition_difference;
  std::vector<GenericReal<is_ad>> _partition_distance;
  std::vector<GenericReal<is_ad>> _partition_covariance;
  DenseVector<GenericReal<is_ad>> _partition_b;
  DenseMatrix<GenericReal<is_ad>> _partition_A;
  ///@}

  using LAROMANCEStressUpdateBaseTempl<is_ad>::_input_values;
  using LAROMANCEStressUpdateBaseTempl<is_ad>::_second_partition_weight;
  using LAROMANCEStressUpdateBaseTempl<is_ad>::_stress_input_index;
  using LAROMANCEStressUpdateBaseTempl<is_ad>::_qp;
  using LAROMANCEStressUpdateBaseTempl<is_ad>::_old_strain_input_index;
  using LAROMANCEStressUpdateBaseTempl<is_ad>::_num_inputs;
};

typedef LAROMANCEPartitionStressUpdateBaseTempl<false> LAROMANCEPartitionStressUpdateBase;
typedef LAROMANCEPartitionStressUpdateBaseTempl<true> ADLAROMANCEPartitionStressUpdateBase;
