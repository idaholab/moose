//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TestNumericalFlux3EqnCentered.h"
#include "ADNumericalFlux3EqnCentered.h"

TEST_F(TestNumericalFlux3EqnCentered, testSymmetry) { testSymmetry(); }
TEST_F(TestNumericalFlux3EqnCentered, testConsistency) { testConsistency(); }

const ADNumericalFlux3EqnBase *
TestNumericalFlux3EqnCentered::createFluxObject()
{
  const std::string class_name = "ADNumericalFlux3EqnCentered";
  InputParameters params = _factory.getValidParams(class_name);
  params.set<UserObjectName>("fluid_properties") = _fp_name;
  _fe_problem->addUserObject(class_name, class_name, params);
  return static_cast<const ADNumericalFlux3EqnBase *>(
      &_fe_problem->getUserObject<ADNumericalFlux3EqnCentered>(class_name));
}

std::vector<std::pair<std::vector<ADReal>, std::vector<ADReal>>>
TestNumericalFlux3EqnCentered::getPrimitiveSolutionsSymmetryTest() const
{
  const std::vector<ADReal> W1{1e5, 300, 1.5};
  const std::vector<ADReal> W2{2e5, 310, 1.2};

  std::vector<std::pair<std::vector<ADReal>, std::vector<ADReal>>> W_pairs;
  W_pairs.push_back(std::pair<std::vector<ADReal>, std::vector<ADReal>>(W1, W2));

  return W_pairs;
}

std::vector<std::vector<ADReal>>
TestNumericalFlux3EqnCentered::getPrimitiveSolutionsConsistencyTest() const
{
  const std::vector<ADReal> W1{1e5, 300, 1.5};

  std::vector<std::vector<ADReal>> W_list;
  W_list.push_back(W1);

  return W_list;
}
