//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FileMeshComponentJunction.h"
#include "BoundaryIntegrationFunctor.h"

class FileMeshComponent;

/**
 * Flow junction that simply stitches the mesh together. The actual physics junction will have to be
 * performed in the definition of the kernels
 */
class FileMeshStitchJunction : public FileMeshComponentJunction
{
public:
  FileMeshStitchJunction(const InputParameters & params);
  static InputParameters validParams();

protected:
  virtual void setupMesh() override;
  virtual void init() override;
  virtual void check() const override;
};
