//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "RhieChowFaceFluxProvider.h"

using namespace libMesh;

InputParameters
RhieChowFaceFluxProvider::validParams()
{
  auto params = GeneralUserObject::validParams();
  params += BlockRestrictable::validParams();

  return params;
}

RhieChowFaceFluxProvider::RhieChowFaceFluxProvider(const InputParameters & params)
  : GeneralUserObject(params), BlockRestrictable(this)
{
}

bool
RhieChowFaceFluxProvider::hasFaceSide(const FaceInfo & fi, const bool fi_elem_side) const
{
  if (fi_elem_side)
    return hasBlocks(fi.elem().subdomain_id());
  else
    return fi.neighborPtr() && hasBlocks(fi.neighbor().subdomain_id());
}
