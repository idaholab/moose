//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseMesh.h"

class TiledMesh : public MooseMesh
{
public:
  static InputParameters validParams();

  TiledMesh(const InputParameters & parameters);
  TiledMesh(const TiledMesh & other_mesh);

  virtual std::unique_ptr<MooseMesh> safeClone() const override;

  virtual void buildMesh() override;

  virtual std::string getFileName() const override;

protected:
  const Real _x_width;
  const Real _y_width;
  const Real _z_width;
};
