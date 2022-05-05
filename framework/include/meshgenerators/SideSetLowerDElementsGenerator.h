//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MeshGenerator.h"

/**
 * Add lower dimensional elements along the faces contained in a side set
 */
class SideSetLowerDElementsGenerator : public MeshGenerator
{
public:
  static InputParameters validParams();

  SideSetLowerDElementsGenerator(const InputParameters & parameters);

  std::unique_ptr<MeshBase> generate() override;

protected:
  /// Mesh to add the sidesets to
  std::unique_ptr<MeshBase> & _input;

  /// Block ID to assign to the region
  const subdomain_id_type _block_id;
};
