//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SpatialUserObjectFunctorMaterial.h"
#include "UserObject.h"

registerMooseObject("MooseApp", SpatialUserObjectFunctorMaterial);

InputParameters
SpatialUserObjectFunctorMaterial::validParams()
{
  InputParameters params = FunctorMaterial::validParams();
  params.addRequiredParam<MooseFunctorName>("functor_name", "Name of the functor to be created");
  params.addRequiredParam<UserObjectName>(
      "user_object", "The name of the user object providing the spatial values");

  return params;
}

SpatialUserObjectFunctorMaterial::SpatialUserObjectFunctorMaterial(
    const InputParameters & parameters)
  : FunctorMaterial(parameters), _user_object(getUserObjectBase("user_object"))
{
  const std::set<ExecFlagType> clearance_schedule(_execute_enum.begin(), _execute_enum.end());

  // This is block-restricted, when it probably does not need to be
  addFunctorProperty<Real>(
      getParam<MooseFunctorName>("functor_name"),
      [this](const auto & r, const auto & /*t*/) -> Real
      { return _user_object.spatialValue(r.getPoint()); },
      clearance_schedule);
}
