//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Moose.h"

// libMesh
#include "libmesh/libmesh.h"

/**
 * Initialization object for any MOOSE-based application
 *
 * This object must be created in the main() of any MOOSE-based application so
 * everything is properly initialized and finalized.
 */
class MooseInit : public LibMeshInit
{
public:
  MooseInit(int argc, char * argv[], MPI_Comm COMM_WORLD_IN = MPI_COMM_WORLD);
  virtual ~MooseInit() = default;
};
