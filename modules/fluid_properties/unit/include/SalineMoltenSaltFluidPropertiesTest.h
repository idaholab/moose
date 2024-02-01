//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseObjectUnitTest.h"
#include "SalineMoltenSaltFluidProperties.h"

class SalineMoltenSaltFluidPropertiesTest : public MooseObjectUnitTest
{
public:
  SalineMoltenSaltFluidPropertiesTest() : MooseObjectUnitTest("FluidPropertiesApp")
  {
    buildObjects();
  }

protected:
  void buildObjects()
  {
    InputParameters uo_pars = _factory.getValidParams("SalineMoltenSaltFluidProperties");
    uo_pars.set<std::vector<std::string>>("comp_name") = {"LiF", "NaF", "KF"};
    uo_pars.set<std::vector<Real>>("comp_val") = {0.465, 0.115, 0.42};
    uo_pars.set<std::string>("prop_def_file") = "../test/tests/saline/saline_custom.prp";
    _fe_problem->addUserObject("SalineMoltenSaltFluidProperties", "fp", uo_pars);
    _fp = &_fe_problem->getUserObject<SalineMoltenSaltFluidProperties>("fp");
  }

  const SalineMoltenSaltFluidProperties * _fp;
};
