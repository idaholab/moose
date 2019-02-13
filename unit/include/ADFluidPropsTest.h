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
#include "IdealGasFluidProperties.h"

class ADFluidPropsTest : public MooseObjectUnitTest
{
public:
  ADFluidPropsTest() : MooseObjectUnitTest("MooseUnitApp") { buildObj(); }

protected:
  const SinglePhaseFluidProperties & buildObj(const std::string name = "fp",
                                              bool allow_imperfect_jac = false)
  {
    InputParameters uo_pars = _factory.getValidParams("IdealGasFluidProperties");
    uo_pars.set<Real>("R") = 287.04;
    uo_pars.set<Real>("gamma") = 1.41;
    uo_pars.set<bool>("allow_imperfect_jacobians") = allow_imperfect_jac;
    _fe_problem->addUserObject("IdealGasFluidProperties", name, uo_pars);
    _fp = &_fe_problem->getUserObject<IdealGasFluidProperties>(name);
    return *_fp;
  }

  const SinglePhaseFluidProperties * _fp;
};
