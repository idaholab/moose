#pragma once

#include "MooseObjectUnitTest.h"
#include "PBSodiumFluidProperties.h"

class PBSodiumFluidPropertiesTest : public MooseObjectUnitTest
{
public:
  PBSodiumFluidPropertiesTest() : MooseObjectUnitTest("SubChannelApp") { buildObjects(); }

protected:
  void buildObjects()
  {
    InputParameters uo_pars = _factory.getValidParams("PBSodiumFluidProperties");
    _fe_problem->addUserObject("PBSodiumFluidProperties", "fp", uo_pars);
    _fp = &_fe_problem->getUserObject<PBSodiumFluidProperties>("fp");
  }

  const PBSodiumFluidProperties * _fp;
};
