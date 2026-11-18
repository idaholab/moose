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
#include "NASA7IdealGasFluidProperties.h"

class NASA7IdealGasFluidPropertiesTest : public MooseObjectUnitTest
{
public:
  NASA7IdealGasFluidPropertiesTest();

protected:
  /// Builds a fluid properties object for a species
  const NASA7IdealGasFluidProperties & buildFluidProperties(const std::string & species,
                                                            const std::string & name);

  const NASA7IdealGasFluidProperties * _fp_h2o;
  const NASA7IdealGasFluidProperties * _fp_n2;
};

inline NASA7IdealGasFluidPropertiesTest::NASA7IdealGasFluidPropertiesTest()
  : MooseObjectUnitTest("FluidPropertiesApp"),
    _fp_h2o(&buildFluidProperties("H2O", "h2o")),
    _fp_n2(&buildFluidProperties("N2", "n2"))
{
}

inline const NASA7IdealGasFluidProperties &
NASA7IdealGasFluidPropertiesTest::buildFluidProperties(const std::string & species,
                                                       const std::string & name)
{
  InputParameters params = _factory.getValidParams("NASA7IdealGasFluidProperties");
  params.set<MooseEnum>("species") = species;
  _fe_problem->addUserObject("NASA7IdealGasFluidProperties", name, params);
  return _fe_problem->getUserObject<NASA7IdealGasFluidProperties>(name);
}
