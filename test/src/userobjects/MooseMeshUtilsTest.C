//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MooseMeshUtilsTest.h"
#include "MooseMeshUtils.h"

registerMooseObject("MooseTestApp", MooseMeshUtilsTest);

InputParameters
MooseMeshUtilsTest::validParams()
{
  auto params = GeneralUserObject::validParams();

  params.addParam<bool>(
      "get_subdomain_id_any", false, "Tests the error for getSubdomainID with ANY_BLOCK_ID");

  return params;
}

MooseMeshUtilsTest::MooseMeshUtilsTest(const InputParameters & parameters)
  : GeneralUserObject(parameters)
{
  if (getParam<bool>("get_subdomain_id_any"))
    MooseMeshUtils::getSubdomainID("ANY_BLOCK_ID", _fe_problem.mesh().getMesh());
}
