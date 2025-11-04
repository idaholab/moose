//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TestNumericalFluxGasMixHLLC.h"
#include "NumericalFluxGasMixHLLC.h"
#include "THMIndicesGasMix.h"

TEST_F(TestNumericalFluxGasMixHLLC, testSymmetry)
{
  testSymmetry();
}

TEST_F(TestNumericalFluxGasMixHLLC, testConsistency)
{
  testConsistency();
}

TestNumericalFluxGasMixHLLC::TestNumericalFluxGasMixHLLC() : TestNumericalFluxGasMixBase()
{
}

const NumericalFlux1D &
TestNumericalFluxGasMixHLLC::createFluxObject()
{
  const std::string class_name = "NumericalFluxGasMixHLLC";
  const std::string flux_mix_name = "flux_mix";
  InputParameters params = _factory.getValidParams(class_name);
  params.set<UserObjectName>("fluid_properties") = _fp_mix_name;
  _fe_problem->addUserObject(class_name, flux_mix_name, params);

  return _fe_problem->getUserObject<NumericalFluxGasMixHLLC>(flux_mix_name);
}

std::vector<std::pair<std::vector<ADReal>, std::vector<ADReal>>>
TestNumericalFluxGasMixHLLC::getPrimitiveSolutionsSymmetryTest() const
{
  // sL < 0 < sM
  std::vector<ADReal> W1(THMGasMix1D::N_PRIM_VARS);
  W1[THMGasMix1D::MASS_FRACTION] = 0.4;
  W1[THMGasMix1D::PRESSURE] = 1e5;
  W1[THMGasMix1D::TEMPERATURE] = 300;
  W1[THMGasMix1D::VELOCITY] = 20;

  std::vector<ADReal> W2(THMGasMix1D::N_PRIM_VARS);
  W2[THMGasMix1D::MASS_FRACTION] = 0.7;
  W2[THMGasMix1D::PRESSURE] = 2e5;
  W2[THMGasMix1D::TEMPERATURE] = 310;
  W2[THMGasMix1D::VELOCITY] = 1.2;

  // sL > 0
  std::vector<ADReal> W3(THMGasMix1D::N_PRIM_VARS);
  W3[THMGasMix1D::MASS_FRACTION] = 0.4;
  W3[THMGasMix1D::PRESSURE] = 1e5;
  W3[THMGasMix1D::TEMPERATURE] = 300;
  W3[THMGasMix1D::VELOCITY] = 500;

  std::vector<ADReal> W4(THMGasMix1D::N_PRIM_VARS);
  W4[THMGasMix1D::MASS_FRACTION] = 0.7;
  W4[THMGasMix1D::PRESSURE] = 2e5;
  W4[THMGasMix1D::TEMPERATURE] = 310;
  W4[THMGasMix1D::VELOCITY] = 550;

  std::vector<std::pair<std::vector<ADReal>, std::vector<ADReal>>> W_pairs;
  W_pairs.push_back(std::pair<std::vector<ADReal>, std::vector<ADReal>>(W1, W2));
  W_pairs.push_back(std::pair<std::vector<ADReal>, std::vector<ADReal>>(W3, W4));

  return W_pairs;
}

std::vector<std::vector<ADReal>>
TestNumericalFluxGasMixHLLC::getPrimitiveSolutionsConsistencyTest() const
{
  std::vector<ADReal> W1(THMGasMix1D::N_PRIM_VARS);
  W1[THMGasMix1D::MASS_FRACTION] = 0.4;
  W1[THMGasMix1D::PRESSURE] = 1e5;
  W1[THMGasMix1D::TEMPERATURE] = 300;
  W1[THMGasMix1D::VELOCITY] = 1.5;

  std::vector<std::vector<ADReal>> W_list;
  W_list.push_back(W1);

  return W_list;
}
