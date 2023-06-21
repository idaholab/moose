//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "BoundaryBase.h"
#include "MeshAlignment2D3D.h"

/**
 * Couples a 2D heat structure boundary to a 3D heat structure boundary using gap heat transfer.
 */
class HSCoupler2D3D : public BoundaryBase
{
public:
  static InputParameters validParams();

  HSCoupler2D3D(const InputParameters & parameters);

  virtual void addMooseObjects() override;

protected:
  virtual void setupMesh() override;
  virtual void check() const override;

  /// 2D heat structure name
  const std::string & _hs_name_2d;
  /// 3D heat structure name
  const std::string & _hs_name_3d;
  /// 2D heat structure boundary
  const BoundaryName & _boundary_2d;
  /// 3D heat structure boundary
  const BoundaryName & _boundary_3d;

  /// Mesh alignment
  MeshAlignment2D3D _mesh_alignment;
};
