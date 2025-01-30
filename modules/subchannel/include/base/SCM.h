#pragma once

#include "MooseMesh.h"

namespace SCM
{
template <typename T>
const T &
getMesh(const MooseMesh & mesh)
{
  // Check that the mesh of the right type
  if (!dynamic_cast<const T *>(&mesh))
    mooseError("The mesh is not of type: ",
               ". You must use the relevant Subchannel mesh/mesh generator with this object.");
  //  prettyCppType<T>(),

  // Cast it to desired type
  return static_cast<const T &>(mesh);
}
};
