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
 * Takes a mesh with a mix of element types, and subdivides elements
 * as needed to produce a mesh of the same domain with all simplex
 * (triangles in 2D, tetrahedra in 3D) elements.
 */
class ElementsToSimplicesConverter : public MeshGenerator
{
public:
  static InputParameters validParams();

  ElementsToSimplicesConverter(const InputParameters & parameters);

  std::unique_ptr<MeshBase> generate() override;

protected:
  /// Input mesh defining the original mixed mesh
  std::unique_ptr<MeshBase> & _input_ptr;
};
