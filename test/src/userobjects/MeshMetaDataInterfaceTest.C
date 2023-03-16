//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MeshMetaDataInterfaceTest.h"

registerMooseObject("MooseTestApp", MeshMetaDataInterfaceTest);

InputParameters
MeshMetaDataInterfaceTest::validParams()
{
  InputParameters params = GeneralUserObject::validParams();

  params.addParam<std::vector<std::string>>(
      "get_wrong_type",
      "Set to test the error for getting a property with the wrong type; expects two values: "
      "property name and generator name");
  params.addParam<std::vector<std::string>>(
      "has_property",
      "Set to test seeing if a property exists; expects two values: property name and generator "
      "name (expects type Real)");

  return params;
}

MeshMetaDataInterfaceTest::MeshMetaDataInterfaceTest(const InputParameters & params)
  : GeneralUserObject(params)
{
  if (isParamValid("get_wrong_type"))
  {
    const auto & property_prefix = getParam<std::vector<std::string>>("get_wrong_type");
    mooseAssert(property_prefix.size() == 2, "Bad size");
    static_cast<void>(getMeshProperty<bool>(property_prefix[0], property_prefix[1]));
  }
  if (isParamValid("has_property"))
  {
    const auto & property_prefix = getParam<std::vector<std::string>>("has_property");
    mooseAssert(property_prefix.size() == 2, "Bad size");
    if (!hasMeshProperty(property_prefix[0], property_prefix[1]))
      mooseError("Has property fail");
    if (!hasMeshProperty<Real>(property_prefix[0], property_prefix[1]))
      mooseError("Has property typed fail");
    if (hasMeshProperty<Real>("foo", "bar"))
      mooseError("Missing property fail");
    if (hasMeshProperty<bool>(property_prefix[0], property_prefix[1]))
      mooseError("Missing property typed fail");
  }
}
