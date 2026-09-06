//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PointInUnionCheckUO.h"
#include "UserObjectBase.h"
#include "SurfaceSide.h"

registerMooseObject("MooseApp", PointInUnionCheckUO);

InputParameters
PointInUnionCheckUO::validParams()
{
  InputParameters params = ThreadedGeneralUserObject::validParams();
  params.addClassDescription("Classifies a point against the union of several geometries, each "
                             "supplied by a user object that implements the point-in-surface check "
                             "interface (a meshed surface, a signed function, or another union).");
  params.addRequiredParam<std::vector<UserObjectName>>(
      "providers",
      "User objects, each implementing the point-in-surface check interface, whose geometries are "
      "unioned. A point is inside the union if it is inside any one of them.");
  return params;
}

PointInUnionCheckUO::PointInUnionCheckUO(const InputParameters & parameters)
  : ThreadedGeneralUserObject(parameters)
{
  const auto & names = getParam<std::vector<UserObjectName>>("providers");
  if (names.empty())
    paramError("providers",
               "must list at least one provider; an empty list would classify every point as "
               "outside.");

  _providers.reserve(names.size());
  for (const auto & name : names)
  {
    // Fetch the base (registering the user-object dependency) and cast to the
    // interface ourselves so a mismatch produces a providers-scoped error naming
    // the offending object, rather than a generic type error.
    const UserObjectBase & base = getUserObjectBaseByName(name);
    const auto * provider = dynamic_cast<const PointInSurfaceCheckInterface *>(&base);
    if (!provider)
      paramError("providers",
                 "'",
                 name,
                 "' (type ",
                 base.type(),
                 ") does not implement the point-in-surface check interface.");
    _providers.push_back(provider);
  }
}

SurfaceGeometry::SurfaceSide
PointInUnionCheckUO::sideness(const Point & p) const
{
  // Fold the provider queries incrementally and stop at the first provider that
  // reports INSIDE (the maximal union result). This avoids the remaining, possibly
  // expensive (e.g. mesh ray-casting) provider queries once membership is decided.
  SurfaceGeometry::SurfaceSide result = SurfaceGeometry::SurfaceSide::OUTSIDE;
  for (const auto * provider : _providers)
  {
    result = SurfaceGeometry::unionSideness(result, provider->sideness(p));
    if (result == SurfaceGeometry::SurfaceSide::INSIDE)
      return result;
  }

  return result;
}
