//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef IDEALGASFLUIDPROPERTIESTEST_H
#define IDEALGASFLUIDPROPERTIESTEST_H

#include "MooseObjectUnitTest.h"
#include "IdealGasFluidProperties.h"

class IdealGasFluidPropertiesTest : public MooseObjectUnitTest
{
public:
  IdealGasFluidPropertiesTest() : MooseObjectUnitTest("MooseUnitApp")
  {
    buildObjects();
  }

protected:
  void buildObjects()
  {
    InputParameters uo_pars = _factory.getValidParams("IdealGasFluidProperties");
    uo_pars.set<Real>("R") = 287.04;
    uo_pars.set<Real>("gamma") = 1.41;
    _fe_problem->addUserObject("IdealGasFluidProperties", "fp", uo_pars);
    _fp = &_fe_problem->getUserObject<IdealGasFluidProperties>("fp");
  }

  const IdealGasFluidProperties * _fp;
};

#endif /* IDEALGASFLUIDPROPERTIESTEST_H */
