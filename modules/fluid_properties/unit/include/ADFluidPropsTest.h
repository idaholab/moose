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
#include "StiffenedGasTwoPhaseFluidProperties.h"

class ADFluidPropsTest : public MooseObjectUnitTest
{
public:
  ADFluidPropsTest() : MooseObjectUnitTest("FluidPropertiesApp") { buildObj(); }

protected:
  const SinglePhaseFluidProperties & buildObj(const std::string name = "fp",
                                              bool allow_imperfect_jac = false)
  {
    InputParameters uo_pars = _factory.getValidParams("IdealGasFluidProperties");
    uo_pars.set<Real>("molar_mass") = 0.028966206103678928;
    uo_pars.set<Real>("gamma") = 1.41;
    uo_pars.set<bool>("allow_imperfect_jacobians") = allow_imperfect_jac;
    _fe_problem->addUserObject("IdealGasFluidProperties", name, uo_pars);
    _fp = &_fe_problem->getUserObject<IdealGasFluidProperties>(name);
    return *_fp;
  }

  const TwoPhaseFluidProperties & buildTwoPhaseFluidProperties()
  {
    {
      const std::string class_name = "StiffenedGasTwoPhaseFluidProperties";
      const UserObjectName fp_2phase_name = "fp_2phase";
      InputParameters params = _factory.getValidParams(class_name);
      _fe_problem->addUserObject(class_name, fp_2phase_name, params);
      return _fe_problem->getUserObject<TwoPhaseFluidProperties>(fp_2phase_name);
    }
  }

  const SinglePhaseFluidProperties * _fp;
};
