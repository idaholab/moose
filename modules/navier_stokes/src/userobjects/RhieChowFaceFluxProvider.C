//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "RhieChowFaceFluxProvider.h"
// #include "INSFVAttributes.h"
// #include "SubProblem.h"
// #include "MooseMesh.h"
// #include "SystemBase.h"
// #include "NS.h"

// #include "libmesh/mesh_base.h"
// #include "libmesh/parallel_algebra.h"
// #include "libmesh/remote_elem.h"
// #include "metaphysicl/dualsemidynamicsparsenumberarray.h"
// #include "metaphysicl/parallel_dualnumber.h"
// #include "metaphysicl/parallel_dynamic_std_array_wrapper.h"
// #include "metaphysicl/parallel_semidynamicsparsenumberarray.h"
// #include "timpi/parallel_sync.h"

using namespace libMesh;

InputParameters
RhieChowFaceFluxProvider::validParams()
{
  auto params = GeneralUserObject::validParams();
  params += BlockRestrictable::validParams();

  return params;
}

RhieChowFaceFluxProvider::RhieChowFaceFluxProvider(const InputParameters & params)
  : GeneralUserObject(params),
    BlockRestrictable(this)
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
