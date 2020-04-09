//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "NodalPostprocessor.h"
#include "PeridynamicsMesh.h"

// Forward Declarations

/**
 * Base postprocessor class for peridynamic calculation
 */
class NodalPostprocessorBasePD : public NodalPostprocessor
{
public:
  static InputParameters validParams();

  NodalPostprocessorBasePD(const InputParameters & parameters);

protected:
  /// Reference to peridynamic mesh object
  PeridynamicsMesh & _pdmesh;

  /// Mesh dimension
  const unsigned int _dim;
};
