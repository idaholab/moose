//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FlowModelGasMixUtilsTest.h"
#include "IdealGasMixtureFluidProperties.h"
#include "IdealGasFluidProperties.h"
#include "THMIndicesGasMix.h"
#include "FlowModelGasMixUtils.h"
#include "THMTestUtils.h"

#include "gtest/gtest.h"

FlowModelGasMixUtilsTest::FlowModelGasMixUtilsTest() : MooseObjectUnitTest("ThermalHydraulicsApp")
{
  addFluidProperties();
}

void
FlowModelGasMixUtilsTest::addFluidProperties()
{
  const std::string fp_steam_name = "fp_steam";
  const std::string fp_nitrogen_name = "fp_nitrogen";
  const std::string fp_mix_name = "fp_mix";

  // steam fluid properties; parameters correspond to T in range 298 K to 473 K
  {
    const std::string class_name = "IdealGasFluidProperties";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<Real>("gamma") = 1.43;
    params.set<Real>("molar_mass") = 0.01801488;
    params.set<Real>("mu") = 0.000013277592; // at 400 K and 1.e5 Pa
    params.set<Real>("k") = 0.026824977826;  // at 400 K and 1.e5 Pa
    _fe_problem->addUserObject(class_name, fp_steam_name, params);
  }

  // nitrogen fluid properties
  {
    const std::string class_name = "IdealGasFluidProperties";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<Real>("gamma") = 1.4;
    params.set<Real>("molar_mass") = 0.028012734746133888;
    params.set<Real>("mu") = 0.0000222084; // at 400 K and 1.e5 Pa
    params.set<Real>("k") = 0.032806168;   // at 400 K and 1.e5 Pa
    _fe_problem->addUserObject(class_name, fp_nitrogen_name, params);
  }

  // mixture fluid properties
  {
    const std::string class_name = "IdealGasMixtureFluidProperties";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<std::vector<UserObjectName>>("component_fluid_properties") = {fp_steam_name,
                                                                             fp_nitrogen_name};
    _fe_problem->addUserObject(class_name, fp_mix_name, params);
    _fp_mix = &_fe_problem->getUserObject<IdealGasMixtureFluidProperties>(fp_mix_name);
  }
}

TEST_F(FlowModelGasMixUtilsTest, testConservativePrimitiveConversionConsistency)
{
  std::vector<ADReal> W(THMGasMix1D::N_PRIM_VARS);
  W[THMGasMix1D::MASS_FRACTION] = 0.4;
  W[THMGasMix1D::PRESSURE] = 1e5;
  W[THMGasMix1D::TEMPERATURE] = 300;
  W[THMGasMix1D::VELOCITY] = 1.5;

  const ADReal A = 2.0;

  const auto U = FlowModelGasMixUtils::computeConservativeSolution<true>(W, A, *_fp_mix);
  const auto W_new = FlowModelGasMixUtils::computePrimitiveSolution<true>(U, *_fp_mix);

  for (unsigned int i = 0; i < W_new.size(); ++i)
    REL_TEST(W_new[i], W[i], REL_TOL_CONSISTENCY);
}
