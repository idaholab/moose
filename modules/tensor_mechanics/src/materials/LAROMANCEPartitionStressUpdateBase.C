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

template <bool is_ad>
InputParameters
LAROMANCEPartitionStressUpdateBaseTempl<is_ad>::validParams()
{
  InputParameters params = LAROMANCEStressUpdateBaseTempl<is_ad>::validParams();
  params.addClassDescription("LAROMANCE base class for partitioned reduced order models");

  return params;
}

template <bool is_ad>
LAROMANCEPartitionStressUpdateBaseTempl<is_ad>::LAROMANCEPartitionStressUpdateBaseTempl(
    const InputParameters & parameters)
  : LAROMANCEStressUpdateBaseTempl<is_ad>(parameters)
{
}

template <bool is_ad>
GenericReal<is_ad>
LAROMANCEPartitionStressUpdateBaseTempl<is_ad>::computeSecondPartitionWeight()
{
  // extract and scale only the relevant inputs on which the GPR was trained (ignore
  // _old_strain_input_index)
  std::vector<GenericReal<is_ad>> scaled_input_values(_input_values.size() - 1);
  unsigned int inc = 0;
  for (unsigned int i = 0; i < _input_values.size(); i++)
  {
    if (i != _old_strain_input_index)
    {
      scaled_input_values[inc] =
          (_input_values[i] - getClassificationMmean()[inc]) / getClassificationMscale()[inc];
      inc++;
    }
  }

  // compute the distance of the new point to ALL training points and sum the distance over all
  // input variables first compute difference between points.
  std::vector<std::vector<GenericReal<is_ad>>> difference(
      getClassificationXu()[0].size(),
      std::vector<GenericReal<is_ad>>(getClassificationXu().size()));
  // for-loop to compute difference
  for (unsigned int i = 0; i < getClassificationXu().size(); i++)
    for (unsigned int j = 0; j < getClassificationXu()[0].size(); j++)
      difference[j][i] = Utility::pow<2>((getClassificationXu()[i][j] - scaled_input_values[i]) /
                                         getClassificationEll());

  // sum difference over input dimension and take square root
  std::vector<GenericReal<is_ad>> distance(difference.size());
  for (unsigned int i = 0; i < difference.size(); i++)
  {
    for (unsigned int j = 0; j < difference[0].size(); j++)
      distance[i] += difference[i][j];
    distance[i] = std::sqrt(distance[i]);
  }

  // compute the covariance wrt ALL training points: the larger the distance, the smaller the
  // covariance
  std::vector<GenericReal<is_ad>> covariance(distance.size());
  for (unsigned int i = 0; i < covariance.size(); i++)
    covariance[i] = Utility::pow<2>(getClassificationEta()) * std::exp(-0.5 * distance[i]);

  // solve the system of equations:
  // convert covariance to libMesh::DenseVector
  DenseVector<GenericReal<is_ad>> b(covariance.size());
  std::vector<GenericReal<is_ad>> covariancet(b.size());
  for (unsigned int i = 0; i < covariance.size(); i++)
    b(i) = covariance[i];

  // convert Luu to libMesh::DenseMatrix
  auto Luu = getClassificationLuu();
  DenseMatrix<GenericReal<is_ad>> A(Luu[0].size(), Luu.size());
  // fill values of A with Luu-values
  for (unsigned int i = 0; i < Luu[0].size(); i++)
    for (unsigned int j = 0; j < Luu.size(); j++)
      A(i, j) = Luu[j][i];

  // solve the linear system of equations (lu_solve)
  DenseVector<GenericReal<is_ad>> ma(b.size());
  A.lu_solve(b, ma);

  // convert inducing points Vinduced to libMesh::DenseVector
  DenseVector<GenericReal<is_ad>> Vind(getClassificationVind().size());
  for (unsigned int i = 0; i < Vind.size(); i++)
    Vind(i) = getClassificationVind()[i];

  // induce full array through sparse grid and summarize into mu
  GenericReal<is_ad> mu = 0.0;
  for (unsigned int i = 0; i < ma.size(); i++)
    mu += ma(i) * Vind(i);

  // scale mu between 0 and 1: the weight of partition 2
  GenericReal<is_ad> partition_weight = -mu + 1.0;
  partition_weight = std::min(partition_weight, 1.0);
  partition_weight = std::max(partition_weight, 0.0);
  return partition_weight;
}

template <bool is_ad>
void
LAROMANCEPartitionStressUpdateBaseTempl<is_ad>::computeDSecondPartitionWeightDStress(
    GenericReal<is_ad> & dsecond_partition_weight_dstress)
{
  const Real fd_tol = 1.0e-6; // Finite difference width
  _input_values[_stress_input_index] += fd_tol;
  dsecond_partition_weight_dstress =
      (computeSecondPartitionWeight() - _second_partition_weight[_qp]) / fd_tol;
  _input_values[_stress_input_index] -= fd_tol;
}

template class LAROMANCEPartitionStressUpdateBaseTempl<false>;
template class LAROMANCEPartitionStressUpdateBaseTempl<true>;
