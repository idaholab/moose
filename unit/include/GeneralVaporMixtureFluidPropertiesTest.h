//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef GENERALVAPORMIXTUREFLUIDPROPERTIESTEST_H
#define GENERALVAPORMIXTUREFLUIDPROPERTIESTEST_H

#include "MooseObjectUnitTest.h"
#include "StiffenedGasFluidProperties.h"
#include "IdealGasFluidProperties.h"
#include "GeneralVaporMixtureFluidProperties.h"

class GeneralVaporMixtureFluidPropertiesTest : public MooseObjectUnitTest
{
public:
  GeneralVaporMixtureFluidPropertiesTest() : MooseObjectUnitTest("MooseUnitApp") { buildObjects(); }

protected:
  void buildObjects()
  {
    const std::string fp_steam_name = "fp_steam";
    const std::string fp_air_name = "fp_air";
    const std::string fp_mix_name = "fp_mix";

    // steam; parameters correspond to T in range 298 K to 473 K
    {
      const std::string class_name = "StiffenedGasFluidProperties";
      InputParameters params = _factory.getValidParams(class_name);
      params.set<Real>("gamma") = 1.43;
      params.set<Real>("q") = 2030e3;
      params.set<Real>("q_prime") = -23e3;
      // This parameter is supposed to be 0 for steam, but then de/dp = 0. For
      // the used mixture model, the derivative dp/de is computed as (de/dp)^-1,
      // so this is artificially changed to a non-zero number here.
      params.set<Real>("p_inf") = 1e4;
      params.set<Real>("cv") = 1040;
      params.set<Real>("M") = 0.01801488;
      _fe_problem->addUserObject(class_name, fp_steam_name, params);
      _fp_steam = &_fe_problem->getUserObject<StiffenedGasFluidProperties>(fp_steam_name);
    }

    // air
    {
      const std::string class_name = "IdealGasFluidProperties";
      InputParameters params = _factory.getValidParams(class_name);
      params.set<Real>("R") = 287.058;
      params.set<Real>("gamma") = 1.4;
      _fe_problem->addUserObject(class_name, fp_air_name, params);
      _fp_air = &_fe_problem->getUserObject<IdealGasFluidProperties>(fp_air_name);
    }

    // mixture
    {
      const std::string class_name = "GeneralVaporMixtureFluidProperties";
      InputParameters params = _factory.getValidParams(class_name);
      params.set<UserObjectName>("fp_primary") = fp_steam_name;
      params.set<std::vector<UserObjectName>>("fp_secondary") = {fp_air_name};
      params.set<Real>("newton_damping") = 0.5;
      params.set<Real>("newton_rel_tol") = 1e-12;
      params.set<unsigned int>("newton_max_its") = 50;
      _fe_problem->addUserObject(class_name, fp_mix_name, params);
      _fp_mix = &_fe_problem->getUserObject<GeneralVaporMixtureFluidProperties>(fp_mix_name);
    }
  }

  const StiffenedGasFluidProperties * _fp_steam;
  const IdealGasFluidProperties * _fp_air;
  const GeneralVaporMixtureFluidProperties * _fp_mix;
};

#endif /* GENERALVAPORMIXTUREFLUIDPROPERTIESTEST_H */
