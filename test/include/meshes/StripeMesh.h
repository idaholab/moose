//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneratedMesh.h"

/**
 * Mesh with subdomains as stripes
 *
 * NOTE: Tailored for rectangular meshes with quad elements
 */
class StripeMesh : public GeneratedMesh
{
public:
  static InputParameters validParams();

  StripeMesh(const InputParameters & parameters);
  StripeMesh(const StripeMesh & other_mesh);
  virtual ~StripeMesh();

  virtual MooseMesh & clone() const;

  virtual void buildMesh();

protected:
  unsigned int _n_stripes; ///< number of stripes
};
