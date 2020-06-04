#pragma once

#include "MooseObjectUnitTest.h"
#include "StiffenedGasTwoPhaseFluidProperties.h"
#include "StiffenedGasFluidProperties.h"

class StiffenedGasTwoPhaseFluidPropertiesTest : public MooseObjectUnitTest
{
public:
  StiffenedGasTwoPhaseFluidPropertiesTest()
    : MooseObjectUnitTest("FluidPropertiesApp"), _T_triple(300.0)
  {
    buildObjects();
  }

protected:
  void buildObjects()
  {
    InputParameters uo_pars = _factory.getValidParams("StiffenedGasTwoPhaseFluidProperties");
    uo_pars.set<Real>("T_triple") = _T_triple;
    _fe_problem->addUserObject("StiffenedGasTwoPhaseFluidProperties", "fp", uo_pars);
    _fp = &_fe_problem->getUserObject<StiffenedGasTwoPhaseFluidProperties>("fp");
    _fp_liquid = &_fe_problem->getUserObject<StiffenedGasFluidProperties>(_fp->getLiquidName());
    _fp_vapor = &_fe_problem->getUserObject<StiffenedGasFluidProperties>(_fp->getVaporName());
  }

  const StiffenedGasTwoPhaseFluidProperties * _fp;
  const StiffenedGasFluidProperties * _fp_liquid;
  const StiffenedGasFluidProperties * _fp_vapor;

  const Real _T_triple;
};
