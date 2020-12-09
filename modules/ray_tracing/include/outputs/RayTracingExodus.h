//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "RayTracingMeshOutput.h"

#include "libmesh/exodusII_io.h"

class RayTracingExodus : public RayTracingMeshOutput
{
public:
  RayTracingExodus(const InputParameters & parameters);

  static InputParameters validParams();

  virtual void outputMesh() override;

  /// The ExodusII_IO object that does the writing
  std::unique_ptr<ExodusII_IO> _exodus_io;

  /// The number of exodus outputs we've done (begins with 1)
  unsigned int _exodus_num;
};
