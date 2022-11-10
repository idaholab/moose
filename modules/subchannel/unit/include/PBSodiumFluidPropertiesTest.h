/********************************************************************/
/*                   DO NOT MODIFY THIS HEADER                      */
/*          Subchannel: Thermal Hydraulics Reactor Analysis         */
/*                                                                  */
/*              (c) 2022 Battelle Energy Alliance, LLC              */
/*                      ALL RIGHTS RESERVED                         */
/*                                                                  */
/*             Prepared by Battelle Energy Alliance, LLC            */
/*               Under Contract No. DE-AC07-05ID14517               */
/*               With the U. S. Department of Energy                */
/*                                                                  */
/*               See COPYRIGHT for full restrictions                */
/********************************************************************/

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
