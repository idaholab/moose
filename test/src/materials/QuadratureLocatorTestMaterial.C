//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "QuadratureLocatorTestMaterial.h"

#include "MooseMesh.h"
#include "SystemBase.h"
#include "MooseEnum.h"
#include "PenetrationLocator.h"

#include "libmesh/string_to_enum.h"

registerMooseObject("MooseTestApp", QuadratureLocatorTestMaterial);

InputParameters
QuadratureLocatorTestMaterial::validParams()
{
  MooseEnum orders("FIRST SECOND THIRD FOURTH", "FIRST");

  InputParameters params = Material::validParams();
  params.set<bool>("_dual_restrictable") = true;
  params.addRequiredParam<BoundaryName>("paired_boundary",
                                        "The boundary on the other side of a gap.");
  params.set<bool>("use_displaced_mesh") = true;
  params.addParam<MooseEnum>("order", orders, "The finite element order");
  return params;
}

QuadratureLocatorTestMaterial::QuadratureLocatorTestMaterial(const InputParameters & parameters)
  : Material(parameters),
    _penetration_locator(getQuadraturePenetrationLocator(
        parameters.get<BoundaryName>("paired_boundary"),
        boundaryNames()[0],
        Utility::string_to_enum<Order>(parameters.get<MooseEnum>("order"))))
{
}
