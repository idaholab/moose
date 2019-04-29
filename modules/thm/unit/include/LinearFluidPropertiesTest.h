#pragma once

#include "MooseObjectUnitTest.h"
#include "LinearFluidProperties.h"

class LinearFluidPropertiesTest : public MooseObjectUnitTest
{
public:
  LinearFluidPropertiesTest() : MooseObjectUnitTest("THMTestApp") { buildObjects(); }

protected:
  void buildObjects()
  {
    InputParameters eos_pars = _factory.getValidParams("LinearFluidProperties");
    eos_pars.set<Real>("p_0") = 1e5;
    eos_pars.set<Real>("rho_0") = 1000;
    eos_pars.set<Real>("a2") = 10;
    eos_pars.set<Real>("beta") = 123;
    eos_pars.set<Real>("cv") = 1000;
    eos_pars.set<Real>("e_0") = 1e6;
    eos_pars.set<Real>("T_0") = 300;
    _fe_problem->addUserObject("LinearFluidProperties", "fp", eos_pars);
    _fp = &_fe_problem->getUserObject<LinearFluidProperties>("fp");
  }

  const LinearFluidProperties * _fp;
};
