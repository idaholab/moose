//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseUtils.h"

class MooseMesh;

namespace SCM
{
/// function to cast const mesh
template <typename T>
const T &
getConstMesh(const MooseMesh & mesh)
{
  const auto T_mesh = dynamic_cast<const T *>(&mesh);
  // Check that the mesh of the right type
  if (!T_mesh)
    mooseError("The mesh is not of type: ",
               MooseUtils::prettyCppType<T>(),
               ". You must use the relevant Subchannel mesh/mesh generator with this object.");
  return *T_mesh;
}

/// function to cast mesh
template <typename T>
T &
getMesh(MooseMesh & mesh)
{
  auto T_mesh = dynamic_cast<T *>(&mesh);
  // Check that the mesh of the right type
  if (!T_mesh)
    mooseError("The mesh is not of type: ",
               MooseUtils::prettyCppType<T>(),
               ". You must use the relevant Subchannel mesh/mesh generator with this object.");
  return *T_mesh;
}
}
