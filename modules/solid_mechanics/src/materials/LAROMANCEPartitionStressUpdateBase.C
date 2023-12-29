//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LAROMANCEPartitionStressUpdateBase.h"

#include <libmesh/dense_matrix.h> // libMesh::cholesky_solve

registerMooseObjectAliased("TensorMechanicsApp",
                           LAROMANCEPartitionStressUpdateBase,
                           "LAROMANCEPartitionStressUpdate");
registerMooseObjectAliased("TensorMechanicsApp",
                           ADLAROMANCEPartitionStressUpdateBase,
                           "ADLAROMANCEPartitionStressUpdate");

template <bool is_ad>
InputParameters
LAROMANCEPartitionStressUpdateBaseTempl<is_ad>::validParams()
{
  InputParameters params = LAROMANCEStressUpdateBaseTempl<is_ad>::validParams();
  params.addClassDescription("LAROMANCE base class for partitioned reduced order models");
  params.addRangeCheckedParam<Real>(
      "finite_difference_width",
      1.0e-2,
      "finite_difference_width > 0",
      "Factor multiplied against the input stress to compute the finite "
      "difference derivative of the parition weights.");

  return params;
}

template <bool is_ad>
LAROMANCEPartitionStressUpdateBaseTempl<is_ad>::LAROMANCEPartitionStressUpdateBaseTempl(
    const InputParameters & parameters)
  : LAROMANCEStressUpdateBaseTempl<is_ad>(parameters),
    _finite_difference_width(this->template getParam<Real>("finite_difference_width"))
{
}

template <bool is_ad>
void
LAROMANCEPartitionStressUpdateBaseTempl<is_ad>::initialSetup()
{
  LAROMANCEStressUpdateBaseTempl<is_ad>::initialSetup();
  _partition_Mmean = getClassificationMmean();
  _partition_Mscale = getClassificationMscale();
  _partition_Xu = getClassificationXu();
  _partition_Ell = getClassificationEll();
  _partition_Eta = getClassificationEta();
  _partition_Luu = getClassificationLuu();
  _partition_Vind = getClassificationVind();
  _partition_difference.resize(_partition_Xu[0].size(),
                               std::vector<GenericReal<is_ad>>(_partition_Xu.size()));
  _partition_distance.resize(_partition_difference.size());
  _partition_covariance.resize(_partition_distance.size());
  _partition_b.resize(_partition_covariance.size());
  _partition_A.resize(_partition_Luu[0].size(), _partition_Luu.size());
}

template <bool is_ad>
void
LAROMANCEPartitionStressUpdateBaseTempl<is_ad>::exportJSON()
{
  LAROMANCEStressUpdateBaseTempl<is_ad>::exportJSON();

  this->_json["m_mean"] = getClassificationMmean();
  this->_json["m_scale"] = getClassificationMscale();
  this->_json["xu"] = getClassificationXu();
  this->_json["ell"] = getClassificationEll();
  this->_json["eta"] = getClassificationEta();
  this->_json["luu"] = getClassificationLuu();
  this->_json["vind"] = getClassificationVind().get_values();
}

template <bool is_ad>
void
LAROMANCEPartitionStressUpdateBaseTempl<is_ad>::computePartitionWeights(
    std::vector<GenericReal<is_ad>> & weights, std::vector<GenericReal<is_ad>> & dweights_dstress)
{
  std::vector<bool> in_partition(_num_partitions, false);
  unsigned int num_partitions_active = 0;
  for (unsigned int p = 0; p < _num_partitions; ++p)
  {
    for (unsigned int t = 0; t < this->_num_tiles[p]; ++t)
      if (!in_partition[p] && this->checkInTile(p, t))
        in_partition[p] = true;

    num_partitions_active += in_partition[p];
  }
  mooseAssert(num_partitions_active <= _num_partitions,
              "Number of paritions active must be less than total number of paritions");
  if (!num_partitions_active)
    mooseException("Number of active partitions (",
                   num_partitions_active,
                   ") out of total number of partitions (",
                   _num_partitions,
                   ") is zero. This may be because you are trying to sample outside the total "
                   "applicability of the model");

  if (_num_partitions == 1) // If only one parition present, all weights are one
  {
    weights[0] = 1.0;
    dweights_dstress[0] = 0.0;
  }
  else if (_num_partitions == 2)
  {
    if (num_partitions_active == 1) // If only present in one partition, math is easy
    {
      std::fill(dweights_dstress.begin(), dweights_dstress.end(), 0.0);

      if (in_partition[0])
        weights[1] = 0.0;
      else if (in_partition[1])
        weights[1] = 1.0;
      else
        mooseError("Internal error: Parition weight calculation incorrect when only one "
                   "partition is applicable");
    }
    else if (num_partitions_active == 2) // If present in two partitions, compute weights
    {
      weights[1] = computeSecondPartitionWeight();
      computeDSecondPartitionWeightDStress(dweights_dstress[1]);
      dweights_dstress[0] = -dweights_dstress[1];
    }
    weights[0] = 1.0 - weights[1];
  }
  else
    mooseError("Internal error: number of partitions can only be 1 or 2");

  if (_num_partitions == 2)
    _second_partition_weight[_qp] = this->_partition_weights[1];
  else
    _second_partition_weight[_qp] = 0.0;

  // Given the weight, compute the sigmoid transformed value to smoothly transition
  for (unsigned int p = 0; p < _num_partitions; ++p)
  {
    weights[p] = (1.0 - sigmoid(0.0, 1.0, weights[p]));
    dweights_dstress[p] *= -sigmoid(0.0, 1.0, weights[p], true);
  }
}

template <bool is_ad>
GenericReal<is_ad>
LAROMANCEPartitionStressUpdateBaseTempl<is_ad>::computeSecondPartitionWeight()
{
  // extract and scale only the relevant inputs on which the GPR was trained (ignore
  // _old_strain_input_index)
  std::vector<GenericReal<is_ad>> scaled_input_values(_num_inputs - 1);
  unsigned int inc = 0;
  for (const auto i : index_range(_input_values))
  {
    if (i != _old_strain_input_index)
    {
      scaled_input_values[inc] =
          (_input_values[i] - _partition_Mmean[inc]) / _partition_Mscale[inc];
      inc++;
    }
  }

  // compute the distance of the new point to ALL training points and sum the distance over all
  // input variables first compute difference between points.
  // for-loop to compute difference
  for (const auto i : index_range(_partition_Xu))
    for (const auto j : index_range(_partition_Xu[0]))
      _partition_difference[j][i] =
          Utility::pow<2>((_partition_Xu[i][j] - scaled_input_values[i]) / _partition_Ell);

  // sum difference over input dimension and take square root
  std::fill(_partition_distance.begin(), _partition_distance.end(), 0.0);
  for (const auto i : index_range(_partition_difference))
  {
    for (const auto j : index_range(_partition_difference[0]))
      _partition_distance[i] += _partition_difference[i][j];
    _partition_distance[i] = std::sqrt(_partition_distance[i]);
  }

  // compute the covariance wrt ALL training points: the larger the distance, the smaller the
  // covariance
  for (const auto i : index_range(_partition_covariance))
    _partition_covariance[i] =
        Utility::pow<2>(_partition_Eta) * std::exp(-0.5 * _partition_distance[i]);

  // solve the system of equations: convert covariance to libMesh::DenseVector
  for (const auto i : index_range(_partition_covariance))
    _partition_b(i) = _partition_covariance[i];

  // convert Luu to libMesh::DenseMatrix, first fill values of A with Luu-values
  for (const auto i : index_range(_partition_Luu[0]))
    for (const auto j : index_range(_partition_Luu))
      _partition_A(i, j) = _partition_Luu[j][i];

  // solve the linear system of equations (lu_solve)
  DenseVector<GenericReal<is_ad>> ma(_partition_b.size());
  _partition_A.lu_solve(_partition_b, ma);

  // induce full array through sparse grid and summarize into mu
  GenericReal<is_ad> partition_weight = 0.0;
  for (const auto i : index_range(ma))
    partition_weight += ma(i) * _partition_Vind(i);

  // scale mu between 0 and 1: the weight of partition 2
  partition_weight = -partition_weight + 1.0;
  partition_weight = std::min(partition_weight, 1.0);
  partition_weight = std::max(partition_weight, 0.0);
  return partition_weight;
}

template <bool is_ad>
void
LAROMANCEPartitionStressUpdateBaseTempl<is_ad>::computeDSecondPartitionWeightDStress(
    GenericReal<is_ad> & dsecond_partition_weight_dstress)
{
  const auto fd_tol =
      _input_values[_stress_input_index] * _finite_difference_width; // Finite difference width
  _input_values[_stress_input_index] += fd_tol;
  dsecond_partition_weight_dstress =
      (computeSecondPartitionWeight() - _second_partition_weight[_qp]) / fd_tol;
  _input_values[_stress_input_index] -= fd_tol;
}

template class LAROMANCEPartitionStressUpdateBaseTempl<false>;
template class LAROMANCEPartitionStressUpdateBaseTempl<true>;
