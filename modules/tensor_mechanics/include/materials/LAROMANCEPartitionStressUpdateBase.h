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
  virtual void computePartitionWeights(std::vector<GenericReal<is_ad>> & weights,
                                       std::vector<GenericReal<is_ad>> & dweights_dstress) override;

  virtual void exportJSON() override;

  /**
   * Compute the partition weight on the location in input-space,
   * based on a calibrated Gaussian Process Regression model
   */
  virtual GenericReal<is_ad> computeSecondPartitionWeight();

  /// Width to determine the finite difference derivative for the partition weight
  const Real _finite_difference_width;

  /**
   * Compute the derivative of the partition weight of the second partition w.r.t. stress
   */
  virtual void
  computeDSecondPartitionWeightDStress(GenericReal<is_ad> & dsecond_partition_weight_dstress);

  ///@{ Method and container for the Gaussian Process Regression lower triangular covariance matrix
  virtual std::vector<std::vector<Real>> getClassificationLuu()
  {
    this->checkJSONKey("luu");
    return this->_json["luu"].template get<std::vector<std::vector<Real>>>();
  }
  std::vector<std::vector<Real>> _partition_Luu;
  ///@}

  ///@{ Method and container for the Gaussian Process Regression model training points
  virtual std::vector<std::vector<Real>> getClassificationXu()
  {
    this->checkJSONKey("xu");
    return this->_json["xu"].template get<std::vector<std::vector<Real>>>();
  }
  std::vector<std::vector<Real>> _partition_Xu;
  ///@}

  ///@{ Method and container for the inducing points of the Gaussian Process Regression model
  virtual DenseVector<Real> getClassificationVind()
  {
    this->checkJSONKey("vind");
    return DenseVector<Real>(this->_json["vind"].template get<std::vector<Real>>());
  }
  DenseVector<Real> _partition_Vind;
  ///@}

  ///@{ Method and container for the mean values of the training input
  virtual std::vector<Real> getClassificationMmean()
  {
    this->checkJSONKey("m_mean");
    return this->_json["m_mean"].template get<std::vector<Real>>();
  }
  std::vector<Real> _partition_Mmean;
  ///@}

  ///@{ Method and container for the scale factor of the training input points to normalize all input parameters to equivalent values
  virtual std::vector<Real> getClassificationMscale()
  {
    this->checkJSONKey("m_scale");
    return this->_json["m_scale"].template get<std::vector<Real>>();
  }
  std::vector<Real> _partition_Mscale;
  ///@}

  ///@{ Method and container for the calibrated Gaussian Regression Model hyperparameter "Ell", which controls the decay of the covariance as a function of distance
  virtual Real getClassificationEll()
  {
    this->checkJSONKey("ell");
    return this->_json["ell"].template get<Real>();
  }
  Real _partition_Ell;
  ///@}

  ///@{ Method and container for the calibrated Gaussian Regression Model hyperparameter "Eta", which is a scale factor that controls the amplitude of the mean
  virtual Real getClassificationEta()
  {
    this->checkJSONKey("eta");
    return this->_json["eta"].template get<Real>();
  }
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
  using LAROMANCEStressUpdateBaseTempl<is_ad>::_num_partitions;
  using LAROMANCEStressUpdateBaseTempl<is_ad>::sigmoid;
};

typedef LAROMANCEPartitionStressUpdateBaseTempl<false> LAROMANCEPartitionStressUpdateBase;
typedef LAROMANCEPartitionStressUpdateBaseTempl<true> ADLAROMANCEPartitionStressUpdateBase;
