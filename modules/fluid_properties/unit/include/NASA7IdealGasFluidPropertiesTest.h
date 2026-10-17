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
  /// Builds the fluid properties object of a species, named after the species
  const NASA7IdealGasFluidProperties & buildFluidProperties(const std::string & species);

  const NASA7IdealGasFluidProperties * _fp_h2o;
  const NASA7IdealGasFluidProperties * _fp_n2;
};

inline NASA7IdealGasFluidPropertiesTest::NASA7IdealGasFluidPropertiesTest()
  : MooseObjectUnitTest("FluidPropertiesApp"),
    _fp_h2o(&buildFluidProperties("H2O")),
    _fp_n2(&buildFluidProperties("N2"))
{
}

inline const NASA7IdealGasFluidProperties &
NASA7IdealGasFluidPropertiesTest::buildFluidProperties(const std::string & species)
{
  InputParameters params = _factory.getValidParams("NASA7IdealGasFluidProperties");
  params.set<MooseEnum>("species") = species;
  _fe_problem->addUserObject("NASA7IdealGasFluidProperties", species, params);
  return _fe_problem->getUserObject<NASA7IdealGasFluidProperties>(species);
}
