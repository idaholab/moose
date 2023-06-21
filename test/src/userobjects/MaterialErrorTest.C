//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MaterialErrorTest.h"

registerMooseObject("MooseTestApp", MaterialErrorTest);

InputParameters
MaterialErrorTest::validParams()
{
  InputParameters params = ElementUserObject::validParams();
  params.addParam<bool>(
      "get_different_types", false, "Test getting materals with the same name and different types");
  params.addParam<bool>("use_ad", false, "True to test with AD");
  params.addParam<bool>("get_different_ad_types",
                        false,
                        "Test getting materials with different AD types (one AD, one non-AD)");
  return params;
}

MaterialErrorTest::MaterialErrorTest(const InputParameters & params) : ElementUserObject(params)
{
  if (getParam<bool>("get_different_types"))
  {
    using type1 = Real;
    using type2 = std::vector<Real>;

    if (getParam<bool>("use_ad"))
    {
      getADMaterialPropertyByName<type1>("foo");
      getADMaterialPropertyByName<type2>("foo");
    }
    else
    {
      getMaterialPropertyByName<type1>("foo");
      getMaterialPropertyByName<type2>("foo");
    }
  }

  if (getParam<bool>("get_different_ad_types"))
  {
    using type = Real;
    if (getParam<bool>("use_ad"))
    {
      getADMaterialPropertyByName<type>("foo");
      getMaterialPropertyByName<type>("foo");
    }
    else
    {
      getMaterialPropertyByName<type>("foo");
      getADMaterialPropertyByName<type>("foo");
    }
  }
}
