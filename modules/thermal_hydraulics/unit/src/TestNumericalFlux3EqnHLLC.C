//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TestNumericalFlux3EqnHLLC.h"
#include "ADNumericalFlux3EqnHLLC.h"
#include "THMIndicesVACE.h"

TEST_F(TestNumericalFlux3EqnHLLC, testSymmetry) { testSymmetry(); }
TEST_F(TestNumericalFlux3EqnHLLC, testConsistency) { testConsistency(); }

TestNumericalFlux3EqnHLLC::TestNumericalFlux3EqnHLLC() : TestNumericalFlux3EqnBase()
{
}

const NumericalFlux1D &
TestNumericalFlux3EqnHLLC::createFluxObject()
{
  const std::string class_name = "ADNumericalFlux3EqnHLLC";
  InputParameters params = _factory.getValidParams(class_name);
  params.set<UserObjectName>("fluid_properties") = _fp_name;
  _fe_problem->addUserObject(class_name, class_name, params);

  return _fe_problem->getUserObject<NumericalFlux1D>(class_name);
}

std::vector<std::pair<std::vector<ADReal>, std::vector<ADReal>>>
TestNumericalFlux3EqnHLLC::getPrimitiveSolutionsSymmetryTest() const
{
  // sL < 0 < sM
  std::vector<ADReal> W1(THMVACE1D::N_PRIM_VARS);
  W1[THMVACE1D::PRESSURE] = 1e5;
  W1[THMVACE1D::TEMPERATURE] = 300;
  W1[THMVACE1D::VELOCITY] = 20;

  std::vector<ADReal> W2(THMVACE1D::N_PRIM_VARS);
  W2[THMVACE1D::PRESSURE] = 2e5;
  W2[THMVACE1D::TEMPERATURE] = 310;
  W2[THMVACE1D::VELOCITY] = 1.2;

  // sL > 0
  std::vector<ADReal> W3(THMVACE1D::N_PRIM_VARS);
  W3[THMVACE1D::PRESSURE] = 1e5;
  W3[THMVACE1D::TEMPERATURE] = 300;
  W3[THMVACE1D::VELOCITY] = 20;

  std::vector<ADReal> W4(THMVACE1D::N_PRIM_VARS);
  W4[THMVACE1D::PRESSURE] = 2e5;
  W4[THMVACE1D::TEMPERATURE] = 310;
  W4[THMVACE1D::VELOCITY] = 25;

  std::vector<std::pair<std::vector<ADReal>, std::vector<ADReal>>> W_pairs;
  W_pairs.push_back(std::pair<std::vector<ADReal>, std::vector<ADReal>>(W1, W2));
  W_pairs.push_back(std::pair<std::vector<ADReal>, std::vector<ADReal>>(W3, W4));

  return W_pairs;
}

std::vector<std::vector<ADReal>>
TestNumericalFlux3EqnHLLC::getPrimitiveSolutionsConsistencyTest() const
{
  std::vector<ADReal> W1(THMVACE1D::N_PRIM_VARS);
  W1[THMVACE1D::PRESSURE] = 1e5;
  W1[THMVACE1D::TEMPERATURE] = 300;
  W1[THMVACE1D::VELOCITY] = 1.5;

  std::vector<std::vector<ADReal>> W_list;
  W_list.push_back(W1);

  return W_list;
}
