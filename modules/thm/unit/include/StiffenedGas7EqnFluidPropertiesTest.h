#pragma once

#include "MooseObjectUnitTest.h"
#include "StiffenedGas7EqnFluidProperties.h"
#include "StiffenedGasFluidProperties.h"

class StiffenedGas7EqnFluidPropertiesTest : public MooseObjectUnitTest
{
public:
  StiffenedGas7EqnFluidPropertiesTest() : MooseObjectUnitTest("THMTestApp") { buildObjects(); }

protected:
  void buildObjects()
  {
    InputParameters uo_pars = _factory.getValidParams("StiffenedGas7EqnFluidProperties");
    _fe_problem->addUserObject("StiffenedGas7EqnFluidProperties", "fp", uo_pars);
    _fp = &_fe_problem->getUserObject<StiffenedGas7EqnFluidProperties>("fp");
    _fp_liquid = &_fe_problem->getUserObject<StiffenedGasFluidProperties>(_fp->getLiquidName());
    _fp_vapor = &_fe_problem->getUserObject<StiffenedGasFluidProperties>(_fp->getVaporName());
  }

  const StiffenedGas7EqnFluidProperties * _fp;
  const StiffenedGasFluidProperties * _fp_liquid;
  const StiffenedGasFluidProperties * _fp_vapor;
};
