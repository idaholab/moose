//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SurfaceSidenessTestAux.h"
#include "PointInSurfaceCheckInterface.h"
#include "UserObjectBase.h"

registerMooseObject("MooseTestApp", SurfaceSidenessTestAux);

InputParameters
SurfaceSidenessTestAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addClassDescription("Reports the tri-state point classification of a point-in-surface "
                             "check user object (outside=0, on=1, inside=2); test-only.");
  params.addRequiredParam<UserObjectName>(
      "user_object", "The point-in-surface check user object whose sideness is reported.");
  return params;
}

SurfaceSidenessTestAux::SurfaceSidenessTestAux(const InputParameters & parameters)
  : AuxKernel(parameters), _check(getCheckedInterface())
{
}

const PointInSurfaceCheckInterface &
SurfaceSidenessTestAux::getCheckedInterface()
{
  const auto * check =
      dynamic_cast<const PointInSurfaceCheckInterface *>(&getUserObjectBase("user_object"));
  if (!check)
    paramError("user_object", "must implement the point-in-surface check interface.");
  return *check;
}

Real
SurfaceSidenessTestAux::computeValue()
{
  const Point & p =
      isNodal() ? static_cast<const Point &>(*_current_node) : _current_elem->vertex_average();
  switch (_check.sideness(p))
  {
    case SurfaceSide::INSIDE:
      return 2.0;
    case SurfaceSide::ON:
      return 1.0;
    case SurfaceSide::OUTSIDE:
      return 0.0;
  }
  return 0.0;
}
