//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef NODALPOSTPROCESSORBASEPD_H
#define NODALPOSTPROCESSORBASEPD_H

#include "NodalPostprocessor.h"
#include "MeshBasePD.h"

// Forward Declarations
class NodalPostprocessorBasePD;

template <>
InputParameters validParams<NodalPostprocessorBasePD>();

/**
 * Base postprocessor class for peridynamic calculation
 */
class NodalPostprocessorBasePD : public NodalPostprocessor
{
public:
  NodalPostprocessorBasePD(const InputParameters & parameters);

protected:
  /// Reference to peridynamic mesh object
  MeshBasePD & _pdmesh;

  /// Mesh dimension
  const unsigned int _dim;
};

#endif // NODALPOSTPROCESSORBASEPD
