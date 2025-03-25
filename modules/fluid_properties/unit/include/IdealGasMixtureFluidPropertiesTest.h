//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseObjectUnitTest.h"
#include "IdealGasFluidProperties.h"
#include "IdealGasMixtureFluidProperties.h"

class IdealGasMixtureFluidPropertiesTest : public MooseObjectUnitTest
{
public:
  IdealGasMixtureFluidPropertiesTest() : MooseObjectUnitTest("FluidPropertiesApp")
  {
    buildObjects();
  }

protected:
  void buildObjects()
  {
    const std::string fp_steam_name = "fp_steam";
    const std::string fp_nitrogen_name = "fp_nitrogen";
    const std::string fp_mix_name = "fp_mix";

    // steam; parameters correspond to T in range 298 K to 473 K
    {
      const std::string class_name = "IdealGasFluidProperties";
      InputParameters params = _factory.getValidParams(class_name);
      params.set<Real>("gamma") = 1.43;
      params.set<Real>("molar_mass") = 0.01801488;
      params.set<Real>("e_ref") = 2030e3;
      params.set<Real>("mu") = 0.000013277592; // at 400 K and 1.e5 Pa
      params.set<Real>("k") = 0.026824977826;  // at 400 K and 1.e5 Pa
      _fe_problem->addUserObject(class_name, fp_steam_name, params);
      _fp_steam = &_fe_problem->getUserObject<IdealGasFluidProperties>(fp_steam_name);
    }

    // nitrogen
    {
      const std::string class_name = "IdealGasFluidProperties";
      InputParameters params = _factory.getValidParams(class_name);
      params.set<Real>("gamma") = 1.4;
      params.set<Real>("molar_mass") = 0.028012734746133888;
      params.set<Real>("e_ref") = 1500e3;
      params.set<Real>("mu") = 0.0000222084; // at 400 K and 1.e5 Pa
      params.set<Real>("k") = 0.032806168;   // at 400 K and 1.e5 Pa
      _fe_problem->addUserObject(class_name, fp_nitrogen_name, params);
      _fp_nitrogen = &_fe_problem->getUserObject<IdealGasFluidProperties>(fp_nitrogen_name);
    }

    // mixture
    {
      const std::string class_name = "IdealGasMixtureFluidProperties";
      InputParameters params = _factory.getValidParams(class_name);
      params.set<std::vector<UserObjectName>>("component_fluid_properties") = {fp_steam_name, fp_nitrogen_name};
      _fe_problem->addUserObject(class_name, fp_mix_name, params);
      _fp_mix = &_fe_problem->getUserObject<IdealGasMixtureFluidProperties>(fp_mix_name);
    }
  }

  const IdealGasFluidProperties * _fp_steam;
  const IdealGasFluidProperties * _fp_nitrogen;
  const IdealGasMixtureFluidProperties * _fp_mix;
};
