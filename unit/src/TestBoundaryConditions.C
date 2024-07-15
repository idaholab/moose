#include "boundary_conditions.h"
#include "mfem.hpp"
#include "gtest/gtest.h"

TEST(CheckData, boundaryConditions)
{
  platypus::BCMap bc_map;
  mfem::Array<int> bdr_attrs({1, 2, 3});

  bc_map.Register(
      "tangential_dEdt",
      std::make_shared<platypus::BoundaryCondition>(std::string("boundary_1"), bdr_attrs));

  mfem::Array<int> ess_bdr = bc_map.Get("tangential_dEdt")->_bdr_attributes;

  for (int i = 0; i < bdr_attrs.Size(); ++i)
  {
    EXPECT_EQ(bdr_attrs[i], ess_bdr[i]);
  }
}
