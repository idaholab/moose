//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html
#include "gtest/gtest.h"

#include "QDEIM.h"
#include "libmesh/dense_vector.h"
#include "libmesh/id_types.h"

TEST(StochasticTools, QDEIM)
{
  // Here is the gold U file
  DenseVector<Real> U1 = {-0.064096, -0.40913, -0.16302, -0.02448, -0.89517};
  DenseVector<Real> U2 = {-0.39344, -0.53986, -0.63356, 0.023065, 0.38965};
  DenseVector<Real> U3 = {-0.80489, 0.35679, 0.1099, -0.44717, -0.11322};
  DenseVector<Real> U4 = {0.43825, 0.15973, -0.43841, -0.76825, -0.0035379};
  std::vector<DenseVector<Real>> gold_U = {U1, U2, U3, U4};

  // Compute QDEIM
  std::vector<DenseVector<Real>> new_basis;
  std::vector<dof_id_type> sample_indices;
  QDEIM qd(gold_U);
  qd.computeSelectionAndBasis(&new_basis, sample_indices);

  // Gold results file
  std::vector<dof_id_type> gold_s = {0, 4, 3, 2};
  DenseVector<Real> M1 = {1, 0, 0, 0};
  DenseVector<Real> M2 = {0.0552399996, 0.2959201626, -0.7330802260, 0.9731093669};
  DenseVector<Real> M3 = {0, 0, 0, 1};
  DenseVector<Real> M4 = {0, 0, 1, 0};
  DenseVector<Real> M5 = {0, 1, 0, 0};
  std::vector<DenseVector<Real>> gold_M = {M1, M2, M3, M4, M5};

  // Test Results
  ASSERT_EQ(new_basis.size(), gold_M.size());

  for (size_t i = 0; i < new_basis.size(); ++i)
  {
    ASSERT_EQ(new_basis[i].size(), gold_M[i].size());
    for (size_t j = 0; j < new_basis[i].size(); ++j)
    {
      EXPECT_NEAR(new_basis[i](j), gold_M[i](j), 1e-6);
    }
  }

  // Check if sample_indices and gold_s have the same size
  ASSERT_EQ(sample_indices.size(), gold_s.size());

  for (size_t i = 0; i < sample_indices.size(); ++i)
  {
    EXPECT_EQ(sample_indices[i], gold_s[i]);
  }
}

TEST(StochasticTools, QDEIM_Selection_Only)
{
  // Here is the gold U file
  DenseVector<Real> U1 = {-0.064096, -0.40913, -0.16302, -0.02448, -0.89517};
  DenseVector<Real> U2 = {-0.39344, -0.53986, -0.63356, 0.023065, 0.38965};
  DenseVector<Real> U3 = {-0.80489, 0.35679, 0.1099, -0.44717, -0.11322};
  DenseVector<Real> U4 = {0.43825, 0.15973, -0.43841, -0.76825, -0.0035379};
  std::vector<DenseVector<Real>> gold_U = {U1, U2, U3, U4};

  // Compute QDEIM
  std::vector<dof_id_type> sample_indices;
  QDEIM qd(gold_U);
  qd.computeSelection(sample_indices);

  // Gold results file
  std::vector<dof_id_type> gold_s = {0, 4, 3, 2};

  // Check if sample_indices and gold_s have the same size
  ASSERT_EQ(sample_indices.size(), gold_s.size());

  for (size_t i = 0; i < sample_indices.size(); ++i)
  {
    EXPECT_EQ(sample_indices[i], gold_s[i]);
  }
}
