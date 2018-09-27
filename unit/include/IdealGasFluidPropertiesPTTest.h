//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef IDEALGASFLUIDPROPERTIESPTTEST_H
#define IDEALGASFLUIDPROPERTIESPTTEST_H

#include "MooseObjectUnitTest.h"
#include "IdealGasFluidPropertiesPT.h"

class IdealGasFluidPropertiesPTTest : public MooseObjectUnitTest
{
public:
  IdealGasFluidPropertiesPTTest() : MooseObjectUnitTest("MooseUnitApp") { buildObjects(); }

protected:
  void buildObjects()
  {
    InputParameters uo_pars = _factory.getValidParams("IdealGasFluidPropertiesPT");
    _fe_problem->addUserObject("IdealGasFluidPropertiesPT", "fp", uo_pars);
    _fp = &_fe_problem->getUserObject<IdealGasFluidPropertiesPT>("fp");
  }

  const IdealGasFluidPropertiesPT * _fp;
};

#endif // IDEALGASFLUIDPROPERTIESPTTEST_H
