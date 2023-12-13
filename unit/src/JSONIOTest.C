//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest/gtest.h"

#include "JsonIO.h"
#include "nlohmann/json.h"

#include "libmesh/int_range.h"
#include "libmesh/dense_matrix.h"

TEST(JSONIOTest, libMeshDenseMatrix)
{
  const std::size_t m = 2;
  const std::size_t n = 3;
  std::vector<std::vector<Real>> values(m, std::vector<Real>(n));
  libMesh::DenseMatrix<Real> matrix(m, n);

  Real current_value = 1;
  for (const auto i : make_range(m))
    for (const auto j : make_range(n))
    {
      values[i][j] = current_value;
      matrix(i, j) = current_value;
      current_value += 2;
    }

  nlohmann::json json;
  nlohmann::to_json(json, matrix);
  const std::vector<std::vector<Real>> json_values = json;
  EXPECT_EQ(json_values, values);
}
