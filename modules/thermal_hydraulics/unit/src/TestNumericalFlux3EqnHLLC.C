//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TestNumericalFlux3EqnHLLC.h"
#include "ADNumericalFlux3EqnHLLC.h"

TEST_F(TestNumericalFlux3EqnHLLC, testSymmetry) { testSymmetry(); }
TEST_F(TestNumericalFlux3EqnHLLC, testConsistency) { testConsistency(); }

const ADNumericalFlux3EqnBase *
TestNumericalFlux3EqnHLLC::createFluxObject()
{
  const std::string class_name = "ADNumericalFlux3EqnHLLC";
  InputParameters params = _factory.getValidParams(class_name);
  params.set<UserObjectName>("fluid_properties") = _fp_name;
  _fe_problem->addUserObject(class_name, class_name, params);
  return static_cast<const ADNumericalFlux3EqnBase *>(
      &_fe_problem->getUserObject<ADNumericalFlux3EqnHLLC>(class_name));
}

std::vector<std::pair<std::vector<ADReal>, std::vector<ADReal>>>
TestNumericalFlux3EqnHLLC::getPrimitiveSolutionsSymmetryTest() const
{
  // sL < 0 < sM
  const std::vector<ADReal> W1{1e5, 300, 20.0};
  const std::vector<ADReal> W2{2e5, 310, 1.2};

  // sL > 0
  const std::vector<ADReal> W3{1e5, 300, 20};
  const std::vector<ADReal> W4{2e5, 310, 25};

  std::vector<std::pair<std::vector<ADReal>, std::vector<ADReal>>> W_pairs;
  W_pairs.push_back(std::pair<std::vector<ADReal>, std::vector<ADReal>>(W1, W2));
  W_pairs.push_back(std::pair<std::vector<ADReal>, std::vector<ADReal>>(W3, W4));

  return W_pairs;
}

std::vector<std::vector<ADReal>>
TestNumericalFlux3EqnHLLC::getPrimitiveSolutionsConsistencyTest() const
{
  const std::vector<ADReal> W1{1e5, 300, 1.5};

  std::vector<std::vector<ADReal>> W_list;
  W_list.push_back(W1);

  return W_list;
}
