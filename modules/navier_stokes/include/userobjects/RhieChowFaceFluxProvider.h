//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralUserObject.h"
#include "BlockRestrictable.h"
#include "FaceArgInterface.h"
#include "MooseTypes.h"

class MooseMesh;
namespace libMesh
{
class Elem;
class MeshBase;
}

class RhieChowFaceFluxProvider : public GeneralUserObject,
                                 public BlockRestrictable,
                                 public FaceArgInterface
{
public:
  static InputParameters validParams();
  RhieChowFaceFluxProvider(const InputParameters & params);

  /**
   * Retrieve the volumetric face flux, will not include derivatives
   * @param m The velocity interpolation method. This is either Rhie-Chow or Average. Rhie-Chow is
   * recommended as it avoids checkerboards in the pressure field
   * @param fi The face that we wish to retrieve the velocity for
   * @param tid The thread ID
   * @return The face velocity
   */
  virtual Real getVolumetricFaceFlux(const Moose::FV::InterpMethod m,
                                     const FaceInfo & fi,
                                     const Moose::StateArg & time,
                                     const THREAD_ID tid,
                                     bool subtract_mesh_velocity) const = 0;

  bool hasFaceSide(const FaceInfo & fi, const bool fi_elem_side) const override;
};
