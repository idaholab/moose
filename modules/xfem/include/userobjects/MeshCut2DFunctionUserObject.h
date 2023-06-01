//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MeshCut2DUserObjectBase.h"

class Function;

/**
 * MeshCut2DFunctionUserObject:
 * (1) reads in a mesh describing the crack surface,
 * (2) uses the mesh to do initial cutting of 2D elements, and
 * (3) grows the mesh based on prescribed growth functions.
 */

class MeshCut2DFunctionUserObject : public MeshCut2DUserObjectBase
{
public:
  static InputParameters validParams();

  MeshCut2DFunctionUserObject(const InputParameters & parameters);

  virtual void initialize() override;

protected:
  virtual void findActiveBoundaryGrowth() override;

private:
  /**
    Parsed functions of front growth
   */
  const Function * _func_x;
  const Function * _func_y;
  const Function * _growth_function;

  /// Gets the time of the previous call to this user object.
  /// This is used to determine if certain things should be executed or not
  Real _time_of_previous_call_to_UO;
};
