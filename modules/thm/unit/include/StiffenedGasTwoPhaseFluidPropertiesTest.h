#pragma once

#include "MooseObjectUnitTest.h"
#include "StiffenedGasTwoPhaseFluidProperties.h"
#include "StiffenedGasFluidProperties.h"

class StiffenedGasTwoPhaseFluidPropertiesTest : public MooseObjectUnitTest
{
public:
  StiffenedGasTwoPhaseFluidPropertiesTest() : MooseObjectUnitTest("THMTestApp") { buildObjects(); }

protected:
  void buildObjects()
  {
    InputParameters uo_pars = _factory.getValidParams("StiffenedGasTwoPhaseFluidProperties");
    _fe_problem->addUserObject("StiffenedGasTwoPhaseFluidProperties", "fp", uo_pars);
    _fp = &_fe_problem->getUserObjectTempl<StiffenedGasTwoPhaseFluidProperties>("fp");
    _fp_liquid =
        &_fe_problem->getUserObjectTempl<StiffenedGasFluidProperties>(_fp->getLiquidName());
    _fp_vapor = &_fe_problem->getUserObjectTempl<StiffenedGasFluidProperties>(_fp->getVaporName());
  }

  const StiffenedGasTwoPhaseFluidProperties * _fp;
  const StiffenedGasFluidProperties * _fp_liquid;
  const StiffenedGasFluidProperties * _fp_vapor;
};
